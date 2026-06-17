import time

from zappy_client import AIClient, GUIClient
from helpers import find_and_take


def test_resources_respawn_to_target_density(make_server):
    # subject: "The server spawns resources upon starting and every 20
    # time units" up to map_width * map_height * density.
    freq = 50
    server = make_server(x=10, y=10, freq=freq)
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    gui.read_lines_until_idle()

    assert find_and_take(ai, "linemate", tries=80)

    def total_linemate():
        gui.send("mct")
        lines = gui.read_lines_until_idle()
        return sum(int(l.split()[4]) for l in lines if l.startswith("bct "))

    after_take = total_linemate()
    assert after_take == 29, f"expected 30 - 1 = 29 linemate right after taking one, got {after_take}"

    time.sleep(20 / freq + 2.0)

    restored = total_linemate()
    assert restored == 30, f"expected respawn back up to 30, got {restored}"
    ai.close()
    gui.close()
