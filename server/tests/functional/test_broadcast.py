from zappy_client import AIClient, GUIClient


def _connect_ai(server, team="team1"):
    ai = AIClient(server.host, server.port, team)
    ai.handshake()
    return ai


def test_broadcast_to_self_is_direction_zero(server):
    # subject: "In the event that the broadcast is emitted from the same
    # player receptor tile, they will receive the message coming from the
    # 0 tile."
    ai = _connect_ai(server)
    ai.send("Broadcast hello")
    msg = ai.read_line()
    assert msg == "message 0, hello"
    ai.expect("ok")
    ai.close()


def test_broadcast_reaches_every_connected_player(server):
    ai1 = _connect_ai(server)
    ai2 = AIClient(server.host, server.port, "team1")
    ai2.handshake()

    ai1.send("Broadcast hi-there")
    line1 = ai1.read_line()
    assert line1 is not None and line1.endswith("hi-there")
    ai1.expect("ok")

    line2 = ai2.read_line()
    assert line2 is not None and line2.endswith("hi-there")
    ai1.close()
    ai2.close()


def test_broadcast_message_text_is_preserved_verbatim(server):
    ai = _connect_ai(server)
    ai.send("Broadcast hello world with spaces")
    msg = ai.read_line()
    assert msg == "message 0, hello world with spaces"
    ai.close()


def test_broadcast_is_relayed_to_gui_as_pbc(server):
    gui = GUIClient(server.host, server.port)
    ai = _connect_ai(server)
    gui.read_lines_until_idle()

    ai.send("Broadcast ping")
    ai.read_line()  # message 0, ping
    ai.expect("ok")
    line = gui.expect_prefix("pbc #")
    assert line.endswith("ping")
    ai.close()
    gui.close()
