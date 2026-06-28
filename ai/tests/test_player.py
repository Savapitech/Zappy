"""The Player command layer: protocol wrappers and observable state, driven by
a fake network (no real socket)."""

import const
from fakes import make_player, run


def test_cmd_returns_plain_response():
    p = make_player()
    p.net.feed("ok")
    assert run(p.cmd("Forward")) == "ok"


def test_cmd_returns_ko():
    p = make_player()
    p.net.feed("ko")
    assert run(p.cmd("Take food")) == "ko"


def test_cmd_dead_short_circuits():
    p = make_player()
    p.net.feed("dead")
    assert run(p.cmd("Look")) == "dead"


def test_cmd_on_closed_writer_returns_dead():
    p = make_player()
    p.net.writer.close()
    assert run(p.cmd("Forward")) == "dead"


def test_cmd_consumes_level_up_then_returns_next_line():
    p = make_player()
    p.net.feed("Current level: 2", "ok")
    assert run(p.cmd("Forward")) == "ok"
    assert p.level == 2


def test_cmd_level_up_resets_stone_inventory():
    p = make_player()
    for item in const.RESOURCES[1:]:
        p.inventory_items[item] = 5
    p.net.feed("Current level: 2", "ok")
    run(p.cmd("Inventory"))
    for item in const.RESOURCES[1:]:
        assert p.inventory_items[item] == 0


def test_level_up_callback_is_invoked():
    p = make_player()
    seen = []
    p.on_level_up = lambda old, new: seen.append((old, new))
    p.net.feed("Current level: 3", "ok")
    run(p.cmd("Forward"))
    assert seen == [(1, 3)]


def test_take_sends_take_command():
    p = make_player()
    p.net.feed("ok")
    assert run(p.take("food")) == "ok"
    assert "Take food" in p.net.sent


def test_set_sends_set_command():
    p = make_player()
    p.net.feed("ok")
    run(p.set("linemate"))
    assert "Set linemate" in p.net.sent


def test_broadcast_sends_broadcast_command():
    p = make_player()
    p.net.feed("ok")
    run(p.broadcast("42_R_1_7"))
    assert "Broadcast 42_R_1_7" in p.net.sent


def test_movement_commands_send_the_right_strings():
    p = make_player()
    p.net.feed("ok", "ok", "ok")
    run(p.forward())
    run(p.left())
    run(p.right())
    assert p.net.sent == ["Forward", "Left", "Right"]


def test_fork_and_eject_send_their_commands():
    p = make_player()
    p.net.feed("ok", "ok")
    run(p.fork())
    run(p.eject())
    assert "Fork" in p.net.sent
    assert "Eject" in p.net.sent


def test_connect_nbr_returns_an_int():
    p = make_player()
    p.net.feed("3")
    assert run(p.connect_nbr()) == 3


def test_connect_nbr_returns_none_on_non_numeric():
    p = make_player()
    p.net.feed("ko")
    assert run(p.connect_nbr()) is None


def test_look_splits_tiles_and_objects():
    p = make_player()
    p.net.feed("[player, food linemate, , thystame]")
    tiles = run(p.look())
    assert tiles[0] == ["player"]
    assert tiles[1] == ["food", "linemate"]
    assert tiles[2] == []
    assert tiles[3] == ["thystame"]


def test_look_single_tile():
    p = make_player()
    p.net.feed("[player]")
    assert run(p.look()) == [["player"]]


def test_look_empty_tiles_are_empty_lists():
    p = make_player()
    p.net.feed("[,,,]")
    assert run(p.look()) == [[], [], [], []]


def test_look_returns_none_on_dead():
    p = make_player()
    p.net.feed("dead")
    assert run(p.look()) is None


def test_look_sends_the_look_command():
    p = make_player()
    p.net.feed("[player]")
    run(p.look())
    assert "Look" in p.net.sent


def test_inventory_updates_all_known_resources():
    p = make_player()
    p.net.feed("[food 10, linemate 2, deraumere 0, sibur 5, "
               "mendiane 1, phiras 3, thystame 7]")
    run(p.inventory())
    assert p.inventory_items["food"] == 10
    assert p.inventory_items["linemate"] == 2
    assert p.inventory_items["sibur"] == 5
    assert p.inventory_items["thystame"] == 7


def test_inventory_handles_partial_listing():
    p = make_player()
    p.net.feed("[food 42, sibur 3]")
    run(p.inventory())
    assert p.inventory_items["food"] == 42
    assert p.inventory_items["sibur"] == 3


def test_inventory_ignores_unknown_tokens():
    p = make_player()
    p.net.feed("[food 5, garbage 9, linemate 1]")
    run(p.inventory())
    assert p.inventory_items["food"] == 5
    assert p.inventory_items["linemate"] == 1


def test_inventory_noop_on_dead():
    p = make_player()
    p.inventory_items["food"] = 7
    p.net.feed("dead")
    run(p.inventory())
    assert p.inventory_items["food"] == 7


def test_food_property_reflects_inventory():
    p = make_player()
    p.inventory_items["food"] = 12
    assert p.food == 12


def test_incantation_success_applies_new_level():
    p = make_player()
    p.net.feed("Elevation underway", "Current level: 2")
    result = run(p.incantation())
    assert result == "Current level: 2"
    assert p.level == 2


def test_incantation_failure_returns_ko():
    p = make_player()
    p.net.feed("ko")
    assert run(p.incantation()) == "ko"
    assert p.level == 1


def _fill_for_level(player, level, factor=1):
    req = const.ELEVATION_REQS[level - 1]
    for idx, item in enumerate(const.RESOURCES[1:]):
        player.inventory_items[item] = req[idx + 1] * factor


def test_has_stones_false_when_inventory_empty():
    for level in range(1, 8):
        p = make_player()
        p.level = level
        assert p.has_stones() is False


def test_has_stones_true_with_exact_requirements_every_level():
    for level in range(1, 8):
        p = make_player()
        p.level = level
        _fill_for_level(p, level)
        assert p.has_stones() is True, f"level {level} should be ready"


def test_has_stones_true_with_surplus():
    for level in range(1, 8):
        p = make_player()
        p.level = level
        _fill_for_level(p, level, factor=3)
        assert p.has_stones() is True


def test_has_stones_false_when_one_stone_missing():
    for level in range(1, 8):
        req = const.ELEVATION_REQS[level - 1]
        for idx, item in enumerate(const.RESOURCES[1:]):
            if req[idx + 1] == 0:
                continue
            p = make_player()
            p.level = level
            _fill_for_level(p, level)
            p.inventory_items[item] -= 1
            assert p.has_stones() is False, (
                f"level {level} missing one {item} must not be ready")


def test_has_stones_false_at_max_level():
    p = make_player()
    p.level = 8
    for item in const.RESOURCES[1:]:
        p.inventory_items[item] = 99
    assert p.has_stones() is False


def test_has_stones_ignores_food():
    p = make_player()
    p.level = 2
    p.inventory_items["food"] = 9999
    assert p.has_stones() is False
