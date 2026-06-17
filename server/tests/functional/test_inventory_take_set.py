from zappy_client import AIClient, GUIClient


def _connect(server):
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    gui.read_lines_until_idle()
    return ai, gui


def test_inventory_format_matches_subject(server):
    ai, gui = _connect(server)
    ai.send("Inventory")
    line = ai.read_line()
    assert line.startswith("[") and line.endswith("]")
    for name in ("food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"):
        assert name in line
    # food must start at 10 (subject: "the player has 10 life units")
    assert "food 10" in line
    ai.close()
    gui.close()


def test_take_on_resourceless_tile_returns_ko(server):
    ai, gui = _connect(server)
    # Clear the tile via repeated Take to guarantee it's empty, then try once more.
    for _ in range(50):
        ai.send("Take linemate")
        r = ai.read_line()
        gui.read_lines_until_idle(idle=0.05, max_total=0.2)
        if r == "ko":
            break
    ai.send("Take linemate")
    ai.expect("ko")
    ai.close()
    gui.close()


def test_take_success_broadcasts_pgt_and_updates_inventory(server):
    ai, gui = _connect(server)
    got = False
    for _ in range(60):
        ai.send("Take linemate")
        r = ai.read_line()
        if r == "ok":
            got = True
            break
        ai.send("Forward")
        ai.read_line()
        gui.read_lines_until_idle(idle=0.05, max_total=0.2)
    assert got, "could not find a linemate on the map to pick up"

    line = gui.expect_prefix("pgt #")
    parts = line.split()
    assert parts[2] == "1", "pgt resource index for linemate must be 1"

    ai.send("Inventory")
    inv = ai.read_line()
    assert "linemate 1" in inv
    ai.close()
    gui.close()


def test_set_success_broadcasts_pdr(server):
    ai, gui = _connect(server)
    got = False
    for _ in range(60):
        ai.send("Take food")
        r = ai.read_line()
        if r == "ok":
            got = True
            break
        ai.send("Forward")
        ai.read_line()
        gui.read_lines_until_idle(idle=0.05, max_total=0.2)
    assert got
    gui.read_lines_until_idle()

    ai.send("Set food")
    ai.expect("ok")
    line = gui.expect_prefix("pdr #")
    parts = line.split()
    assert parts[2] == "0", "pdr resource index for food must be 0"
    ai.close()
    gui.close()


def test_set_without_resource_returns_ko(server):
    ai, _ = _connect(server)
    ai.send("Set thystame")
    ai.expect("ko")
    ai.close()


def test_take_unknown_resource_name_returns_ko(server):
    ai, _ = _connect(server)
    ai.send("Take diamond")
    ai.expect("ko")
    ai.close()
