from zappy_client import AIClient


def test_unknown_ai_command_returns_ko(server):
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    ai.send("Frobnicate")
    ai.expect("ko")
    ai.close()


def test_team_full_is_refused(make_server):
    server = make_server(clients=1)
    ai1 = AIClient(server.host, server.port, "team1")
    ai1.handshake()
    ai2 = AIClient(server.host, server.port, "team1")
    line = ai2.read_line()
    assert line == "ko"
    ai1.close()
    ai2.close()


def test_unknown_team_name_is_refused(server):
    ai = AIClient(server.host, server.port, "no-such-team")
    line = ai.read_line()
    assert line == "ko"
    ai.close()


def test_commands_queue_up_and_execute_in_order(server):
    # subject: "The client can send up to 10 requests in a row without any
    # response from the server." Forward/Right/Left all cost 7/f; queue a
    # batch and check every "ok" eventually arrives, in order.
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()

    n = 8
    for _ in range(n):
        ai.send("Right")
    for _ in range(n):
        assert ai.read_line(timeout=3.0) == "ok"
    ai.close()


def test_more_than_ten_queued_commands_are_dropped(server):
    # subject: "Over 10, the server will no longer take them into account."
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()

    for _ in range(15):
        ai.send("Right")

    oks = 0
    while True:
        line = ai.read_line(timeout=1.5)
        if line is None:
            break
        assert line == "ok"
        oks += 1
    assert oks <= 10, f"expected at most 10 queued commands to run, got {oks}"
    ai.close()
