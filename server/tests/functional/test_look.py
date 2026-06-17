from zappy_client import AIClient


def _connect(server, team="team1"):
    ai = AIClient(server.host, server.port, team)
    ai.handshake()
    return ai


def test_look_level1_has_4_tiles(server):
    # subject: vision unit is 1 at level 1 -> tiles 0..3, i.e. (level+1)^2 = 4
    ai = _connect(server)
    ai.send("Look")
    line = ai.read_line()
    assert line.startswith("[") and line.endswith("]")
    tiles = line[1:-1].split(",")
    assert len(tiles) == 4
    ai.close()


def test_look_own_tile_lists_self_as_player(server):
    ai = _connect(server)
    ai.send("Look")
    line = ai.read_line()
    tiles = line[1:-1].split(",")
    assert "player" in tiles[0]
    ai.close()


def test_look_sees_other_player_on_same_tile(make_server):
    server = make_server(clients=1)
    ai1 = _connect(server)
    ai1.send("Fork")
    ai1.expect("ok")
    ai2 = AIClient(server.host, server.port, "team1")
    ai2.handshake()  # hatches from the only egg: same tile as ai1

    ai1.send("Look")
    line = ai1.read_line()
    tiles = line[1:-1].split(",")
    assert tiles[0].count("player") == 2, f"expected 2 players on own tile, got {tiles[0]!r}"
    ai1.close()
    ai2.close()
