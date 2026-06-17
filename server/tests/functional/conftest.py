import itertools
import pathlib
import socket
import subprocess
import time

import pytest

SERVER_BIN = pathlib.Path(__file__).resolve().parents[3] / "zappy_server"

_port_counter = itertools.count(30000)


def _free_port():
    for _ in range(50):
        port = next(_port_counter)
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.bind(("127.0.0.1", port))
            except OSError:
                continue
        return port
    raise RuntimeError("could not find a free port")


class ServerHandle:
    def __init__(self, proc, port):
        self.proc = proc
        self.port = port
        self.host = "127.0.0.1"

    def stop(self):
        if self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait(timeout=2)


def _wait_listening(port, timeout=3.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            with socket.create_connection(("127.0.0.1", port), timeout=0.2):
                return
        except OSError:
            time.sleep(0.02)
    raise RuntimeError(f"server did not start listening on port {port} in time")


@pytest.fixture
def make_server(tmp_path):
    handles = []

    def _make(x=10, y=10, teams=("team1",), clients=2, freq=100):
        assert SERVER_BIN.exists(), f"server binary not found at {SERVER_BIN}, build it first"
        port = _free_port()
        log_path = tmp_path / f"server_{port}.log"
        args = [
            str(SERVER_BIN),
            "-p", str(port),
            "-x", str(x),
            "-y", str(y),
            "-n", *teams,
            "-c", str(clients),
            "-f", str(freq),
        ]
        with open(log_path, "wb") as logf:
            proc = subprocess.Popen(args, stdout=logf, stderr=subprocess.STDOUT)
        _wait_listening(port)
        handle = ServerHandle(proc, port)
        handle.log_path = log_path
        handles.append(handle)
        return handle

    yield _make

    for h in handles:
        h.stop()


@pytest.fixture
def server(make_server):
    return make_server()
