#!/usr/bin/env python3
"""Private CA integration tests: run after make with python3 test/test_tls_cacert.py.

Requires GnuTLS/XML-enabled YAZ, Python SSL, openssl, and local TCP sockets.
Certificates and keys are generated in a temporary directory.
"""
# This file is part of the YAZ toolkit.
# Copyright (C) Index Data. See the file LICENSE for details.
import contextlib
import http.server
import pathlib
import ssl
import subprocess
import tempfile
import threading
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
RESPONSE = b'''<searchRetrieveResponse xmlns="http://www.loc.gov/zing/srw/">
<version>1.2</version><numberOfRecords>0</numberOfRecords>
</searchRetrieveResponse>'''


def openssl(*args):
    subprocess.run(["openssl", *map(str, args)], check=True,
                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)


@contextlib.contextmanager
def server(cert, key):
    class Handler(http.server.BaseHTTPRequestHandler):
        protocol_version = "HTTP/1.1"

        def do_GET(self):
            self.server.requests.append(self.path)
            redirect = self.path.split("?")[0] == "/start"
            self.send_response(302 if redirect else 200)
            if redirect:
                self.send_header("Location", "/done")
            self.send_header("Content-Type", "text/xml")
            self.send_header("Content-Length", str(0 if redirect else len(RESPONSE)))
            self.end_headers()
            if not redirect:
                self.wfile.write(RESPONSE)

        def log_message(self, *args):
            pass

    httpd = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ctx.load_cert_chain(cert, key)
    httpd.socket = ctx.wrap_socket(httpd.socket, server_side=True)
    httpd.requests = []
    thread = threading.Thread(target=httpd.serve_forever, daemon=True)
    thread.start()
    try:
        yield httpd
    finally:
        httpd.shutdown()
        httpd.server_close()
        thread.join()


class CACertTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix="yaz-cacert-")
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.path = pathlib.Path(cls.tmp.name)
        for name in ("ca", "other"):
            openssl("req", "-x509", "-newkey", "rsa:2048", "-nodes", "-sha256",
                    "-days", "2", "-keyout", cls.path / (name + ".key"),
                    "-out", cls.path / (name + ".pem"), "-subj", "/CN=" + name,
                    "-addext", "basicConstraints=critical,CA:TRUE",
                    "-addext", "keyUsage=critical,keyCertSign,cRLSign")
        openssl("req", "-new", "-newkey", "rsa:2048", "-nodes",
                "-keyout", cls.path / "server.key", "-out", cls.path / "server.csr",
                "-subj", "/CN=localhost")
        (cls.path / "server.ext").write_text(
            "basicConstraints=critical,CA:FALSE\n"
            "keyUsage=critical,digitalSignature,keyEncipherment\n"
            "extendedKeyUsage=serverAuth\nsubjectAltName=DNS:localhost\n")
        openssl("x509", "-req", "-in", cls.path / "server.csr",
                "-CA", cls.path / "ca.pem", "-CAkey", cls.path / "ca.key",
                "-set_serial", "2", "-out", cls.path / "server.pem",
                "-days", "2", "-sha256", "-extfile", cls.path / "server.ext")
        (cls.path / "index").write_text("")
        (cls.path / "serial").write_text("03\n")
        (cls.path / "ca.cnf").write_text(
            "[ca]\ndefault_ca=test\n[test]\n"
            "database={0}/index\nserial={0}/serial\nnew_certs_dir={0}\n"
            "certificate={0}/ca.pem\nprivate_key={0}/ca.key\n"
            "default_md=sha256\npolicy=policy\n[policy]\ncommonName=supplied\n".format(cls.path))
        openssl("ca", "-batch", "-config", cls.path / "ca.cnf",
                "-in", cls.path / "server.csr", "-out", cls.path / "expired.pem",
                "-startdate", "20000101000000Z", "-enddate", "20000102000000Z",
                "-extfile", cls.path / "server.ext", "-notext")
        openssl("x509", "-req", "-in", cls.path / "server.csr",
                "-signkey", cls.path / "server.key", "-out", cls.path / "self.pem",
                "-days", "2", "-sha256", "-extfile", cls.path / "server.ext")
        (cls.path / "bundle.pem").write_bytes(
            (cls.path / "other.pem").read_bytes() + (cls.path / "ca.pem").read_bytes())
        (cls.path / "empty.pem").write_text("")
        (cls.path / "invalid.pem").write_text("not a certificate\n")

    def fetch(self, client, endpoint, cafile=None, verify=False, check_false=False,
              short=False):
        args = []
        settings = ""
        if cafile is not None:
            args += ["-A" if short else "--cacert", str(cafile)]
            settings += 'set cafile "%s"\n' % cafile
        if verify:
            args += ["--check-cert"]
            settings += "set check_cert 1\n"
        if check_false:
            settings += "set check_cert 0\n"
        if client == "url":
            command = [str(ROOT / "util/yaz-url"), *args, endpoint]
            commands = ""
        elif client == "client":
            command = [str(ROOT / "client/yaz-client"), *args, "-f", "/dev/null"]
            commands = "sru get\nopen %s\nfind computer\nquit\n" % endpoint
        else:
            command = [str(ROOT / "zoom/zoomsh")]
            commands = settings + "set sru get\nconnect %s\nsearch computer\nquit\n" % endpoint
        return subprocess.run(command, input=commands, text=True, capture_output=True,
                              timeout=10, cwd=self.path)

    def test_private_trust(self):
        cases = [
            # name, bundle, verify, host, cert, allowed, check_false
            ("legacy default", None, False, "localhost", "server", True, False),
            ("system rejects private CA", None, True, "localhost", "server", False, False),
            ("private CA", "ca.pem", False, "localhost", "server", True, False),
            ("explicit check", "ca.pem", True, "localhost", "server", True, False),
            ("CA implies check", "ca.pem", False, "localhost", "server", True, True),
            ("self-signed", "self.pem", False, "localhost", "self", True, False),
            ("combined bundle", "bundle.pem", False, "localhost", "server", True, False),
            ("wrong CA", "other.pem", False, "localhost", "server", False, False),
            ("wrong host", "ca.pem", False, "127.0.0.1", "server", False, False),
            ("expired", "ca.pem", False, "localhost", "expired", False, False),
            ("missing", "missing.pem", False, "localhost", "server", False, False),
            ("empty", "empty.pem", False, "localhost", "server", False, False),
            ("invalid", "invalid.pem", False, "localhost", "server", False, False),
        ]
        for client in ("url", "client", "zoom"):
            for name, bundle, verify, host, cert, allowed, check_false in cases:
                with self.subTest(client=client, case=name):
                    with server(self.path / (cert + ".pem"), self.path / "server.key") as httpd:
                        endpoint = "https://%s:%d/" % (host, httpd.server_port)
                        result = self.fetch(client, endpoint,
                                            self.path / bundle if bundle else None,
                                            verify, check_false)
                        output = result.stdout + result.stderr
                        self.assertEqual(bool(httpd.requests), allowed, output)
                        if client == "url":
                            self.assertEqual(result.returncode == 0, allowed, output)
                        if client != "zoom" and bundle in ("missing.pem", "empty.pem", "invalid.pem"):
                            self.assertIn("TLS trust loading failed", output)

    def test_redirect_and_short_option(self):
        for client in ("url", "client", "zoom"):
            with self.subTest(client=client):
                with server(self.path / "server.pem", self.path / "server.key") as httpd:
                    result = self.fetch(client, "https://localhost:%d/start" % httpd.server_port,
                                        self.path / "ca.pem", short=True)
                    self.assertEqual(len(httpd.requests), 2, result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
