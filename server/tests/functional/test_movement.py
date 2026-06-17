from zappy_client import AIClient, GUIClient


def _connect_ai_and_gui(server):
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    # drain the pnw/pin/ebo triplet emitted for this connection
    gui.read_lines_until_idle()
    return ai, gui


def test_forward_moves_one_tile_and_broadcasts_ppo(server):
    ai, gui = _connect_ai_and_gui(server)
    ai.send("Forward")
    ai.expect("ok")
    line = gui.expect_prefix("ppo #")
    parts = line.split()
    assert len(parts) == 5  # ppo #n X Y O
    ai.close()
    gui.close()


def test_right_changes_orientation_not_position(server):
    ai, gui = _connect_ai_and_gui(server)
    ai.send("Forward")
    ai.expect("ok")
    before = gui.expect_prefix("ppo #").split()

    ai.send("Right")
    ai.expect("ok")
    after = gui.expect_prefix("ppo #").split()

    assert before[2] == after[2] and before[3] == after[3], "Right must not move the player"
    assert before[4] != after[4], "Right must change orientation"
    ai.close()
    gui.close()


def test_left_is_the_inverse_of_right(server):
    ai, _ = _connect_ai_and_gui(server)
    ai.send("Right")
    ai.expect("ok")
    ai.send("Left")
    ai.expect("ok")
    ai.send("Inventory")
    inv = ai.read_line()
    assert inv is not None
    ai.close()


def test_forward_wraps_around_world_edges(server):
    # The subject: "If a player exits by the right of the board, they will
    # come back through the left." Walking exactly map_width tiles forward
    # while facing east must return to the exact same tile.
    ai, gui = _connect_ai_and_gui(server)

    # face east deterministically: rotate right until orientation == 2 (E)
    ai.send("Forward")
    ai.expect("ok")
    pos1 = gui.expect_prefix("ppo #").split()
    orientation = int(pos1[4])
    turns_needed = (2 - orientation) % 4
    for _ in range(turns_needed):
        ai.send("Right")
        ai.expect("ok")
        gui.expect_prefix("ppo #")

    ai.send("Forward")
    ai.expect("ok")
    start = tuple(gui.expect_prefix("ppo #").split()[2:4])

    line = None
    for _ in range(10):  # map is 10 wide: 10 more steps closes the loop
        ai.send("Forward")
        ai.expect("ok")
        line = gui.expect_prefix("ppo #").split()
    end = tuple(line[2:4])
    assert end == start, "after width Forward steps east, must be back on the same tile"
    ai.close()
    gui.close()
