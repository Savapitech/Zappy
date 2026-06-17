import time

from zappy_client import AIClient


def _time_command(ai, cmd, expected_response="ok"):
    start = time.monotonic()
    ai.send(cmd)
    line = ai.read_line(timeout=5.0)
    elapsed = time.monotonic() - start
    assert line == expected_response, f"{cmd!r} -> {line!r}"
    return elapsed


def test_connect_nbr_is_instant(server):
    # subject: Connect_nbr has no time limit ("-").
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    start = time.monotonic()
    ai.send("Connect_nbr")
    ai.read_line()
    elapsed = time.monotonic() - start
    assert elapsed < 0.25  # GAME_TICK_MS poll granularity, not a real cost
    ai.close()


def test_command_costs_match_action_over_f(make_server):
    freq = 20
    server = make_server(freq=freq)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()

    cases = [
        ("Forward", 7, "ok"),
        ("Right", 7, "ok"),
        ("Left", 7, "ok"),
        ("Look", 7, None),
        ("Inventory", 1, None),
        ("Broadcast hi", 7, None),  # the "message 0, hi" line, not ok
    ]
    for cmd, cost, expected in cases:
        start = time.monotonic()
        ai.send(cmd)
        line = ai.read_line(timeout=5.0)
        elapsed = time.monotonic() - start
        wanted = cost / freq
        assert line is not None
        if expected is not None:
            assert line == expected
        assert abs(elapsed - wanted) < 0.15, (
            f"{cmd!r}: expected ~{wanted:.2f}s ({cost}/{freq}), got {elapsed:.2f}s"
        )
    ai.close()


def test_fork_cost_is_42_over_f(make_server):
    freq = 20
    server = make_server(freq=freq)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    elapsed = _time_command(ai, "Fork")
    assert abs(elapsed - 42 / freq) < 0.2
    ai.close()
