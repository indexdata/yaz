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
def server():
    class Handler(http.server.BaseHTTPRequestHandler):
        protocol_version = "HTTP/1.1"

        def do_GET(self):
            self.server.requests.append((self.path, self.headers.get("Cookie")))
            location, cookie = self.server.route(self.path)
            self.send_response(self.server.code if location else 200)
            if location:
                self.send_header("Location", location)
            if cookie is not None:
                self.send_header("Set-Cookie", cookie)
            self.send_header("Content-Type", "text/xml")
            self.send_header("Content-Length", str(0 if location else len(RESPONSE)))
            self.end_headers()
            if not location:
                self.wfile.write(RESPONSE)

        def log_message(self, *args):
            pass

    httpd = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    httpd.requests = []
    httpd.route = lambda path: (None, None)
    httpd.code = 302
    httpd.url = "http://127.0.0.1:%d" % httpd.server_port
    thread = threading.Thread(target=httpd.serve_forever, daemon=True)
    thread.start()
    try:
        yield httpd
    finally:
        httpd.shutdown()
        httpd.server_close()
        thread.join()


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
            url = source.url + "/start"
            if client == "zoom":
                program = ROOT / "zoom/zoomsh"
                commands = "set sru get\nconnect %s\nsearch computer\nquit\n" % url
            else:
                program = ROOT / "client/yaz-client"
                commands = "sru get\nopen %s\nfind computer\nquit\n" % url
            result = subprocess.run([str(program)], input=commands, text=True,
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                    timeout=10)
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


if __name__ == "__main__":
    unittest.main()
