import time

from zappy_client import AIClient, GUIClient
from helpers import find_and_take


def _get_linemate_on_tile(ai, gui, tries=80):
    got = find_and_take(ai, "linemate", tries=tries)
    if got:
        ai.send("Set linemate")
        assert ai.read_line() == "ok"
    gui.read_lines_until_idle()
    return got


def test_incantation_without_resources_fails_immediately(server):
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()

    # the spawn tile may already carry a linemate by chance (30% density):
    # clear it so the pre-check is guaranteed to fail.
    for _ in range(20):
        ai.send("Take linemate")
        if ai.read_line() == "ko":
            break

    start = time.monotonic()
    ai.send("Incantation")
    line = ai.read_line(timeout=1.0)
    elapsed = time.monotonic() - start
    assert line == "ko"
    assert elapsed < 0.5, "a failed pre-check must not wait the 300/f incantation delay"
    ai.close()


def test_incantation_level1_to_2_succeeds_and_times_pic_then_pie(make_server):
    server = make_server(freq=100)
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    gui.read_lines_until_idle()

    assert _get_linemate_on_tile(ai, gui)

    t0 = time.monotonic()
    ai.send("Incantation")
    ai.expect("Elevation underway")

    pic = gui.read_line(timeout=0.5)
    t_pic = time.monotonic() - t0
    assert pic is not None and pic.startswith("pic ")
    assert t_pic < 0.3, "pic must be broadcast immediately, not after the 300/f delay"
    parts = pic.split()
    assert parts[3] == "1", "pic level must be the participants' current level"

    level_msg = ai.read_line(timeout=4.0)
    t_end = time.monotonic() - t0
    assert level_msg == "Current level: 2"
    assert t_end >= 2.8, "the incantation must take ~300/f seconds before resolving"

    lines_after = gui.read_lines_until_idle()
    assert any(l.startswith("plv #") and l.endswith(" 2") for l in lines_after)
    assert any(l == f"pie {parts[1]} {parts[2]} 1" for l in lines_after)
    ai.close()
    gui.close()


def test_frozen_player_cannot_act_during_incantation(make_server):
    server = make_server(freq=100)
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    gui.read_lines_until_idle()
    assert _get_linemate_on_tile(ai, gui)

    ai.send("Incantation")
    ai.expect("Elevation underway")
    gui.read_lines_until_idle(idle=0.1, max_total=0.5)  # drain pic

    ai.send("Forward")
    assert ai.read_line(timeout=1.0) == "ko", "a frozen player must not be able to act"
    ai.close()
    gui.close()


def test_same_level_player_on_tile_joins_the_ritual_and_is_frozen(make_server):
    server = make_server(freq=100, clients=1)
    ai1 = AIClient(server.host, server.port, "team1")
    ai1.handshake()
    gui = GUIClient(server.host, server.port)
    gui.read_lines_until_idle()
    assert _get_linemate_on_tile(ai1, gui)

    ai1.send("Fork")
    ai1.expect("ok")
    ai2 = AIClient(server.host, server.port, "team1")
    ai2.handshake()  # same tile, same level 1
    gui.read_lines_until_idle()

    ai1.send("Incantation")
    ai1.expect("Elevation underway")
    gui.read_lines_until_idle(idle=0.1, max_total=0.5)  # drain pic

    # a same-level bystander present when the ritual starts is swept in and
    # frozen too, per "every player in a group doing an incantation".
    ai2.send("Take linemate")
    assert ai2.read_line(timeout=1.0) == "ko"

    assert ai1.read_line(timeout=4.0) == "Current level: 2"
    assert ai2.read_line(timeout=1.0) == "Current level: 2"
    ai1.close()
    ai2.close()
    gui.close()


def test_incantation_fails_at_end_check_if_sole_participant_disconnects(make_server):
    server = make_server(freq=100)
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    gui.read_lines_until_idle()
    assert _get_linemate_on_tile(ai, gui)

    ai.send("Incantation")
    ai.expect("Elevation underway")
    pic = gui.read_line(timeout=0.5)
    assert pic is not None and pic.startswith("pic ")

    ai.close()  # rage-quit mid-ritual

    lines = gui.read_lines_until_idle(idle=0.5, max_total=5.0)
    pie = next((l for l in lines if l.startswith("pie ")), None)
    assert pie is not None, "the GUI must still be told the ritual ended"
    assert pie.endswith(" 0"), "with no participant left, the ritual must fail"
    gui.close()
