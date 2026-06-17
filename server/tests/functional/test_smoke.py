from zappy_client import AIClient, GUIClient


def test_ai_handshake(server):
    ai = AIClient(server.host, server.port, "team1")
    slots, (x, y) = ai.handshake()
    assert slots == 1
    assert (x, y) == (10, 10)
    ai.close()


def test_gui_handshake(server):
    gui = GUIClient(server.host, server.port)
    gui.send("msz")
    gui.expect_prefix("msz ")
    gui.close()
