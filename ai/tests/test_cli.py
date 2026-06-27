"""Black-box checks of the built zappy_ai launcher's argument handling."""

import pathlib
import subprocess

import pytest

BIN = pathlib.Path(__file__).resolve().parents[2] / "zappy_ai"

requires_binary = pytest.mark.skipif(
    not BIN.exists(), reason="zappy_ai not built (run `make zappy_ai`)")


def _run(*args, timeout=5):
    return subprocess.run([str(BIN), *args], capture_output=True, text=True,
                          timeout=timeout)


@requires_binary
def test_help_prints_usage_and_exits_zero():
    res = _run("--help")
    assert res.returncode == 0
    assert "USAGE" in res.stdout
    assert "-p port" in res.stdout
    assert "-n name" in res.stdout


@requires_binary
def test_no_arguments_exits_84():
    assert _run().returncode == 84


@requires_binary
def test_missing_team_exits_84():
    assert _run("-p", "4242").returncode == 84


@requires_binary
def test_missing_port_exits_84():
    assert _run("-n", "team1").returncode == 84


@requires_binary
def test_dangling_port_flag_exits_84():
    assert _run("-p").returncode == 84
