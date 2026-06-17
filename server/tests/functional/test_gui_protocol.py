from zappy_client import AIClient, GUIClient


def test_msz_tna_sgt_are_available_on_request(make_server):
    # The server only answers these on request; it never pushes them on
    # its own (the GUI client is responsible for asking after GRAPHIC).
    server = make_server(x=7, y=13, teams=("team1", "team2"))
    gui = GUIClient(server.host, server.port)
    gui.read_lines_until_idle()

    gui.send("msz")
    gui.expect("msz 7 13")

    gui.send("tna")
    lines = gui.read_lines_until_idle()
    assert "tna team1" in lines
    assert "tna team2" in lines

    gui.send("sgt")
    gui.expect_prefix("sgt ")
    gui.close()


def test_mct_returns_every_tile_exactly_once(server):
    gui = GUIClient(server.host, server.port)
    gui.read_lines_until_idle()
    gui.send("mct")
    lines = gui.read_lines_until_idle()
    bct_lines = [l for l in lines if l.startswith("bct ")]
    assert len(bct_lines) == 10 * 10
    seen = {(l.split()[1], l.split()[2]) for l in bct_lines}
    assert len(seen) == 100, "mct must cover every tile exactly once"
    gui.close()


def test_bct_on_one_tile(server):
    gui = GUIClient(server.host, server.port)
    gui.read_lines_until_idle()
    gui.send("bct 2 3")
    line = gui.read_line()
    parts = line.split()
    assert parts[0] == "bct"
    assert parts[1] == "2" and parts[2] == "3"
    assert len(parts) == 10  # bct X Y q0..q6
    gui.close()


def test_sst_changes_freq_and_echoes_it(server):
    gui = GUIClient(server.host, server.port)
    gui.read_lines_until_idle()
    gui.send("sst 42")
    gui.expect("sst 42")
    gui.send("sgt")
    gui.expect("sgt 42")
    gui.close()


def test_sst_with_invalid_freq_returns_sbp(server):
    gui = GUIClient(server.host, server.port)
    gui.read_lines_until_idle()
    gui.send("sst 0")
    gui.expect("sbp")
    gui.send("sst -1")
    gui.expect("sbp")
    gui.close()


def test_ppo_plv_pin_on_request_for_existing_player(server):
    gui = GUIClient(server.host, server.port)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    lines = gui.read_lines_until_idle()
    pnw = next(l for l in lines if l.startswith("pnw #"))
    pid = pnw.split()[1]

    gui.send(f"ppo {pid}")
    gui.expect_prefix(f"ppo {pid} ")

    gui.send(f"plv {pid}")
    gui.expect(f"plv {pid} 1")

    gui.send(f"pin {pid}")
    gui.expect_prefix(f"pin {pid} ")
    ai.close()
    gui.close()


def test_ppo_for_unknown_player_returns_sbp(server):
    gui = GUIClient(server.host, server.port)
    gui.read_lines_until_idle()
    gui.send("ppo #999")
    gui.expect("sbp")
    gui.close()


def test_unknown_gui_command_returns_suc(server):
    gui = GUIClient(server.host, server.port)
    gui.read_lines_until_idle()
    gui.send("frobnicate")
    gui.expect("suc")
    gui.close()


def test_bct_with_garbage_args_returns_sbp(server):
    gui = GUIClient(server.host, server.port)
    gui.read_lines_until_idle()
    gui.send("bct not numbers")
    gui.expect("sbp")
    gui.close()
