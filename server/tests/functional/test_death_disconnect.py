import time

from zappy_client import AIClient, GUIClient


def test_starving_player_gets_dead_message_exactly_once(make_server):
    freq = 50
    server = make_server(freq=freq)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()

    total_ticks = 10 * 126  # START_FOOD * SURVIVAL_TIME
    deadline = time.monotonic() + total_ticks / freq + 3.0

    seen = []
    while time.monotonic() < deadline:
        line = ai.read_line(timeout=0.5)
        if line is None:
            continue
        seen.append(line)
        if line == "dead":
            break
    assert seen.count("dead") == 1, f"expected exactly one 'dead', got {seen.count('dead')}"
    ai.close()


def test_dead_player_disconnect_is_broadcast_to_gui_as_pdi(make_server):
    freq = 80
    server = make_server(freq=freq)
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    gui.read_lines_until_idle()

    total_ticks = 10 * 126
    ai.expect("dead", timeout=total_ticks / freq + 3.0)

    lines = gui.read_lines_until_idle(idle=0.5, max_total=3.0)
    assert any(l.startswith("pdi #") for l in lines)
    gui.close()


def test_clean_disconnect_without_dying_is_also_broadcast_as_pdi(server):
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    gui.read_lines_until_idle()

    ai.close()  # plain TCP close, no starvation involved

    lines = gui.read_lines_until_idle(idle=0.5, max_total=2.0)
    assert any(l.startswith("pdi #") for l in lines), "any disconnect must clean up the GUI's view"
    gui.close()


def test_disconnect_without_a_spare_egg_cannot_reconnect(make_server):
    # Slots are backed by eggs (subject: "a team has n slot(s) available,
    # represented by an egg waiting for a client to connect"). Forking is
    # the only way to lay a new egg, so disconnecting does not let a new
    # client take the now-hatched egg's place.
    server = make_server(clients=1)
    ai1 = AIClient(server.host, server.port, "team1")
    ai1.handshake()
    ai1.close()
    time.sleep(0.3)

    ai2 = AIClient(server.host, server.port, "team1")
    try:
        ai2.handshake()
        assert False, "no egg left, the connection should have been refused"
    except AssertionError as e:
        assert "ko" in str(e)
    ai2.close()
