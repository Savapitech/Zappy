from zappy_client import AIClient, GUIClient


def test_two_teams_are_independent(make_server):
    server = make_server(teams=("red", "blue"), clients=2)
    ai_red = AIClient(server.host, server.port, "red")
    slots_red, _ = ai_red.handshake()
    ai_blue = AIClient(server.host, server.port, "blue")
    slots_blue, _ = ai_blue.handshake()

    assert slots_red == 1
    assert slots_blue == 1

    ai_red.send("Connect_nbr")
    assert ai_red.read_line() == "1"
    ai_blue.send("Connect_nbr")
    assert ai_blue.read_line() == "1"
    ai_red.close()
    ai_blue.close()


def test_broadcast_crosses_team_boundaries(make_server):
    # subject: "All the players can hear the broadcasts" - not scoped to a
    # team.
    server = make_server(teams=("red", "blue"))
    ai_red = AIClient(server.host, server.port, "red")
    ai_red.handshake()
    ai_blue = AIClient(server.host, server.port, "blue")
    ai_blue.handshake()

    ai_red.send("Broadcast cross-team")
    ai_red.read_line()
    line = ai_blue.read_line(timeout=1.0)
    assert line is not None and line.endswith("cross-team")
    ai_red.close()
    ai_blue.close()


def test_gui_sees_pnw_with_correct_team_name(make_server):
    server = make_server(teams=("red", "blue"))
    gui = GUIClient(server.host, server.port)
    ai_blue = AIClient(server.host, server.port, "blue")
    ai_blue.handshake()
    lines = gui.read_lines_until_idle()
    pnw = next(l for l in lines if l.startswith("pnw #"))
    assert pnw.endswith(" blue")
    ai_blue.close()
    gui.close()
