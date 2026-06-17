from zappy_client import AIClient, GUIClient


def test_fork_adds_a_slot_and_lays_an_egg(server):
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    slots, _ = ai.handshake()
    gui.read_lines_until_idle()

    ai.send("Connect_nbr")
    before = int(ai.read_line())

    ai.send("Fork")
    ai.expect("ok")
    after_lines = gui.read_lines_until_idle()
    assert any(l.startswith("pfk #") for l in after_lines)
    assert any(l.startswith("enw #") for l in after_lines)

    ai.send("Connect_nbr")
    after = int(ai.read_line())
    assert after == before + 1, "Fork must add exactly one slot to the team"
    ai.close()
    gui.close()


def test_egg_hatches_at_forking_players_tile(make_server):
    server = make_server(clients=1)
    ai1 = AIClient(server.host, server.port, "team1")
    ai1.handshake()
    ai1.send("Fork")
    ai1.expect("ok")

    ai2 = AIClient(server.host, server.port, "team1")
    ai2.handshake()

    ai1.send("Look")
    line = ai1.read_line()
    tiles = line[1:-1].split(",")
    assert tiles[0].count("player") == 2
    ai1.close()
    ai2.close()


def test_eject_with_nobody_on_tile_returns_ko_and_keeps_egg(make_server):
    server = make_server(clients=1)
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    gui.read_lines_until_idle()

    ai.send("Fork")  # lays an egg on the player's own tile
    ai.expect("ok")
    gui.read_lines_until_idle()

    ai.send("Eject")
    ai.expect("ko")
    gui.assert_silent(0.3)  # no pex, and the un-hatched egg must not be destroyed
    ai.close()
    gui.close()


def test_eject_pushes_player_and_destroys_eggs_on_tile(make_server):
    server = make_server(clients=1)
    gui = GUIClient(server.host, server.port)
    ai1 = AIClient(server.host, server.port, "team1")
    ai1.handshake()
    ai1.send("Fork")  # extra slot
    ai1.expect("ok")
    ai1.send("Fork")  # second egg, also laid on ai1's tile
    ai1.expect("ok")

    ai2 = AIClient(server.host, server.port, "team1")
    ai2.handshake()  # hatches one of the two eggs, same tile as ai1
    gui.read_lines_until_idle()

    ai1.send("Eject")
    ai1.expect("ok")
    eject_line = ai2.read_line()
    assert eject_line is not None and eject_line.startswith("eject: ")

    lines = gui.read_lines_until_idle()
    assert any(l.startswith("pex #") for l in lines)
    assert any(l.startswith("edi #") for l in lines), "the leftover egg must be destroyed"
    ai1.close()
    ai2.close()
    gui.close()
