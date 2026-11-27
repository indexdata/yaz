#!/usr/bin/env python3
"""SRU redirect integration regression: run after make with python3 <this file>.

Requires an XML-enabled build, zoomsh, yaz-client, and local TCP sockets.
"""
# This file is part of the YAZ toolkit.
# Copyright (C) Index Data. See the file LICENSE for details.
import contextlib
import http.server
import pathlib
import subprocess
import threading
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
RESPONSE = b'''<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
<version>1.2</version><numberOfRecords>0</numberOfRecords>
</searchRetrieveResponse>'''


@contextlib.contextmanager
def server(protocol="HTTP/1.1"):
    class Handler(http.server.BaseHTTPRequestHandler):
        protocol_version = protocol

        def do_GET(self):
            self.server.requests.append((self.path, self.headers.get("Cookie")))
            location, cookie = self.server.route(self.path)
            self.send_response(self.server.code if location else 200)
            if location:
                self.send_header("Location", location)
            if cookie is not None:
                self.send_header("Set-Cookie", cookie)
            self.send_header("Content-Type", "text/xml")
            self.send_header("Content-Length",
                             str(0 if location else len(self.server.response)))
            self.end_headers()
            if not location:
                self.wfile.write(self.server.response)

        def log_message(self, *args):
            pass

    httpd = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    httpd.requests = []
    httpd.route = lambda path: (None, None)
    httpd.code = 302
    httpd.response = RESPONSE
    httpd.url = "http://127.0.0.1:%d" % httpd.server_port
    thread = threading.Thread(target=httpd.serve_forever, daemon=True)
    thread.start()
    try:
        yield httpd
    finally:
        httpd.shutdown()
        httpd.server_close()
        thread.join()


def search(client, url, count=1):
    if client == "zoom":
        program = ROOT / "zoom/zoomsh"
        commands = "set sru get\nconnect %s\n%squit\n" % (url, "search computer\n" * count)
    else:
        program = ROOT / "client/yaz-client"
        commands = "sru get\nopen %s\n%squit\n" % (url, "find computer\n" * count)
    return subprocess.run([str(program)], input=commands, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          timeout=10)


class RedirectCookies(unittest.TestCase):
    def test_redirects(self):
        # Cookies on the first redirect, cookies retained from a previous hop,
        # and empty-valued cookies must all prevent forwarding across origins.
        for client in ("zoom", "client"):
            for code in (301, 302):
                for mode in ("same", "port", "host", "stored", "empty", "none"):
                    with self.subTest(client=client, code=code, mode=mode):
                        self.check_redirect(client, code, mode)

    def check_redirect(self, client, code, mode):
        with server() as source, server() as target:
            source.code = code
            destination = source.url + "/done" if mode == "same" else target.url + "/done"
            if mode == "host":
                destination = (source.url + "/done").replace("127.0.0.1", "localhost")
            cookie = None if mode == "none" else "session=secret; Secure; Path=/"
            if mode == "empty":
                cookie = "session="

            def route(path):
                if path.startswith("/start"):
                    return ("/stage" if mode == "stored" else destination, cookie)
                if path == "/stage":
                    return destination, None
                return None, None

            source.route = route
            result = search(client, source.url + "/start")
            self.assertEqual(result.returncode, 0, result.stdout)
            expected = 2 if mode == "stored" else 1
            if mode == "same":
                expected = 2
                self.assertEqual(source.requests[-1], ("/done", "session=secret"),
                                 result.stdout)
            self.assertEqual(len(source.requests), expected, result.stdout)
            if mode == "none":
                self.assertEqual(target.requests, [("/done", None)], result.stdout)
            else:
                self.assertEqual(target.requests, [], result.stdout)
            if mode not in ("same", "none"):
                self.assertIn("Cross-origin authenticated redirect refused", result.stdout)

    def test_requests_after_redirect(self):
        # A redirect target of another origin gets the redirect URL only. What
        # follows is for the configured host: the request for the records that
        # the target did not return (ZOOM), and later searches (both).
        for client in ("zoom", "client"):
            with self.subTest(client=client):
                self.check_after_redirect(client)

    def check_after_redirect(self, client):
        with server() as source, server() as target:
            source.route = lambda path: (target.url + "/done", None)
            target.response = RESPONSE.replace(b">0<", b">2<").replace(
                b"</numberOfRecords>", b"</numberOfRecords><records><record>"
                b"<recordData><title/></recordData></record></records>")
            url = source.url + "/start"
            if client == "zoom":
                program = ROOT / "zoom/zoomsh"
                commands = ("set sru get\nset count 2\nset extraArgs x-key=secret\n"
                            "connect %s\nsearch water\nsearch fire\nquit\n" % url)
            else:
                program = ROOT / "client/yaz-client"
                commands = "sru get\nopen %s\nfind water\nfind fire\nquit\n" % url
            result = subprocess.run([str(program)], input=commands, text=True,
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                    timeout=10)
            self.assertEqual(result.returncode, 0, result.stdout)
            # one request per record for ZOOM, one per search for yaz-client
            expected = 4 if client == "zoom" else 2
            self.assertEqual(len(source.requests), expected, result.stdout)
            self.assertEqual(target.requests, [("/done", None)] * expected,
                             result.stdout)
            if client == "zoom":
                self.assertEqual(result.stdout.count("<title"), 4, result.stdout)

    def test_redirect_target_cookies(self):
        # A cookie that the target of a redirect sets belongs to that target.
        # It must not reach the configured server: not in a later search, not
        # when the target redirects back, and not when a relative Location of
        # the target is followed, also after the target closed the connection.
        for client in ("zoom", "client"):
            for mode in ("later", "back", "relative", "closed"):
                with self.subTest(client=client, mode=mode):
                    self.check_redirect_target(client, mode)

    def check_redirect_target(self, client, mode):
        protocol = "HTTP/1.0" if mode == "closed" else "HTTP/1.1"
        with server() as source, server(protocol) as target:
            # First search is redirected to target, second to source itself
            locations = [target.url + "/db", "/done"]

            def source_route(path):
                if path.startswith("/start"):
                    return locations.pop(0), None
                return None, None

            def target_route(path):
                location = None
                if path != "/db":
                    return None, None
                if mode == "back":
                    location = source.url + "/done"
                elif mode in ("relative", "closed"):
                    location = "/done"
                return location, "session=secret; Path=/"

            source.route = source_route
            target.route = target_route
            result = search(client, source.url + "/start",
                            2 if mode == "later" else 1)
            self.assertEqual(result.returncode, 0, result.stdout)
            self.assertEqual([cookie for path, cookie in source.requests
                              if cookie], [], result.stdout)
            self.assertEqual(target.requests[0], ("/db", None), result.stdout)
            if mode == "later" and client == "zoom":
                # ZOOM returns to the configured server after a redirect
                self.assertEqual(source.requests[-1], ("/done", None),
                                 result.stdout)
            elif mode == "back":
                self.assertEqual(len(source.requests), 1, result.stdout)
                self.assertIn("Cross-origin authenticated redirect refused",
                              result.stdout)
            elif mode != "later":
                self.assertEqual(len(source.requests), 1, result.stdout)
                self.assertEqual(target.requests[1:],
                                 [("/done", "session=secret")], result.stdout)


if __name__ == "__main__":
    unittest.main()
