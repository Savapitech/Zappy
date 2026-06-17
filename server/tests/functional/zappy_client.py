"""Small line-buffered TCP client used to drive the Zappy server like a real
AI or GUI client would, without guessing at timing via sleeps.
"""
import socket
import time


class ZappyClient:
    def __init__(self, host, port, connect_timeout=2.0):
        self.sock = socket.create_connection((host, port), timeout=connect_timeout)
        self.sock.settimeout(0.05)
        self._buf = b""

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass

    def send(self, line):
        if not line.endswith("\n"):
            line += "\n"
        self.sock.sendall(line.encode())

    def _pump(self):
        try:
            data = self.sock.recv(65536)
        except socket.timeout:
            return False
        except OSError:
            return False
        if data == b"":
            raise ConnectionError("peer closed the connection")
        self._buf += data
        return True

    def read_line(self, timeout=2.0):
        """Read exactly one '\\n'-terminated line, waiting up to `timeout`
        seconds. Returns None on timeout."""
        deadline = time.monotonic() + timeout
        while True:
            nl = self._buf.find(b"\n")
            if nl != -1:
                line = self._buf[:nl].decode(errors="replace")
                self._buf = self._buf[nl + 1 :]
                return line
            if time.monotonic() >= deadline:
                return None
            self._pump()

    def read_lines_until_idle(self, idle=0.3, max_total=5.0):
        """Drain every line currently arriving, stopping once nothing new
        has shown up for `idle` seconds (or `max_total` elapses)."""
        lines = []
        start = time.monotonic()
        last_activity = start
        while True:
            line = self._try_read_line(0.05)
            now = time.monotonic()
            if line is not None:
                lines.append(line)
                last_activity = now
            if now - last_activity >= idle or now - start >= max_total:
                return lines

    def _try_read_line(self, timeout):
        nl = self._buf.find(b"\n")
        if nl != -1:
            line = self._buf[:nl].decode(errors="replace")
            self._buf = self._buf[nl + 1 :]
            return line
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            if self._pump():
                nl = self._buf.find(b"\n")
                if nl != -1:
                    line = self._buf[:nl].decode(errors="replace")
                    self._buf = self._buf[nl + 1 :]
                    return line
        return None

    def expect(self, expected, timeout=2.0):
        line = self.read_line(timeout)
        assert line == expected, f"expected {expected!r}, got {line!r}"
        return line

    def expect_prefix(self, prefix, timeout=2.0):
        line = self.read_line(timeout)
        assert line is not None, f"expected a line starting with {prefix!r}, got nothing"
        assert line.startswith(prefix), f"expected prefix {prefix!r}, got {line!r}"
        return line

    def assert_silent(self, timeout=0.3):
        line = self.read_line(timeout)
        assert line is None, f"expected no message, got {line!r}"


class AIClient(ZappyClient):
    def __init__(self, host, port, team, connect_timeout=2.0):
        super().__init__(host, port, connect_timeout)
        self.expect("WELCOME")
        self.send(team)

    def handshake(self, timeout=2.0):
        """Read the [slots, mapX mapY] pair sent after a successful team
        join. Returns (slots:int, (x:int,y:int)) or raises on 'ko'."""
        first = self.read_line(timeout)
        assert first is not None, "no handshake response"
        assert first != "ko", "team join refused (ko)"
        slots = int(first)
        size_line = self.read_line(timeout)
        x, y = (int(v) for v in size_line.split())
        return slots, (x, y)


class GUIClient(ZappyClient):
    def __init__(self, host, port, connect_timeout=2.0):
        super().__init__(host, port, connect_timeout)
        self.expect("WELCOME")
        self.send("GRAPHIC")
