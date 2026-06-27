"""The bot's coroutines that parse server replies, driven by a fake network."""

import const
from fakes import make_bot, run


def test_look_splits_tiles_and_objects():
    bot = make_bot()
    bot.net.feed("[player, food linemate, , thystame]")
    tiles = run(bot.look())
    assert tiles[0] == ["player"]
    assert tiles[1] == ["food", "linemate"]
    assert tiles[2] == []
    assert tiles[3] == ["thystame"]


def test_look_single_tile():
    bot = make_bot()
    bot.net.feed("[player]")
    tiles = run(bot.look())
    assert tiles == [["player"]]


def test_look_empty_tiles_are_empty_lists():
    bot = make_bot()
    bot.net.feed("[,,,]")
    tiles = run(bot.look())
    assert tiles == [[], [], [], []]


def test_look_returns_none_on_dead():
    bot = make_bot()
    bot.net.feed("dead")
    assert run(bot.look()) is None


def test_look_sends_the_look_command():
    bot = make_bot()
    bot.net.feed("[player]")
    run(bot.look())
    assert "Look" in bot.net.sent


def test_inventory_updates_all_known_resources():
    bot = make_bot()
    bot.net.feed("[food 10, linemate 2, deraumere 0, sibur 5, "
                 "mendiane 1, phiras 3, thystame 7]")
    run(bot.inventory())
    assert bot.inventory_items["food"] == 10
    assert bot.inventory_items["linemate"] == 2
    assert bot.inventory_items["sibur"] == 5
    assert bot.inventory_items["thystame"] == 7


def test_inventory_handles_partial_listing():
    bot = make_bot()
    bot.net.feed("[food 42, sibur 3]")
    run(bot.inventory())
    assert bot.inventory_items["food"] == 42
    assert bot.inventory_items["sibur"] == 3


def test_inventory_ignores_unknown_tokens():
    bot = make_bot()
    bot.net.feed("[food 5, garbage 9, linemate 1]")
    run(bot.inventory())
    assert bot.inventory_items["food"] == 5
    assert bot.inventory_items["linemate"] == 1


def test_inventory_noop_on_dead():
    bot = make_bot()
    bot.inventory_items["food"] = 7
    bot.net.feed("dead")
    run(bot.inventory())
    assert bot.inventory_items["food"] == 7


def test_cmd_returns_plain_response():
    bot = make_bot()
    bot.net.feed("ok")
    assert run(bot.cmd("Forward")) == "ok"


def test_cmd_returns_ko():
    bot = make_bot()
    bot.net.feed("ko")
    assert run(bot.cmd("Take food")) == "ko"


def test_cmd_consumes_level_up_then_returns_next_line():
    bot = make_bot()
    bot.level = 1
    bot.net.feed("Current level: 2", "ok")
    result = run(bot.cmd("Forward"))
    assert result == "ok"
    assert bot.level == 2


def test_cmd_level_up_resets_stone_inventory():
    bot = make_bot()
    bot.level = 1
    for item in const.RESOURCES[1:]:
        bot.inventory_items[item] = 5
    bot.net.feed("Current level: 2", "ok")
    run(bot.cmd("Inventory"))
    for item in const.RESOURCES[1:]:
        assert bot.inventory_items[item] == 0


def test_cmd_dead_short_circuits():
    bot = make_bot()
    bot.net.feed("dead")
    assert run(bot.cmd("Look")) == "dead"


def test_cmd_on_closed_writer_returns_dead():
    bot = make_bot()
    bot.net.writer.close()
    assert run(bot.cmd("Forward")) == "dead"


def test_eject_resets_following_state():
    bot = make_bot()
    bot.state = 2
    bot.leader_id = "42"
    bot.leader_dir = 5
    bot.net.feed_event("j", "3")
    run(bot.handle_events())
    assert bot.state == 0
    assert bot.leader_id is None
    assert bot.leader_dir == -1


def test_eject_ignored_while_collecting():
    bot = make_bot()
    bot.state = 0
    bot.net.feed_event("j", "3")
    run(bot.handle_events())
    assert bot.state == 0


def test_broadcast_from_same_level_leader_triggers_follow():
    bot = make_bot()
    bot.state = 0
    bot.level = 1
    bot.inventory_items["food"] = 50
    msg = f"4, {bot.key}_R_1_777"
    bot.net.feed_event("m", msg)
    run(bot.handle_events())
    assert bot.state == 2
    assert bot.leader_id == "777"
    assert bot.leader_dir == 4


def test_broadcast_from_other_level_is_ignored():
    bot = make_bot()
    bot.state = 0
    bot.level = 3
    bot.inventory_items["food"] = 50
    msg = f"2, {bot.key}_R_1_777"
    bot.net.feed_event("m", msg)
    run(bot.handle_events())
    assert bot.state == 0


def test_broadcast_with_foreign_key_is_ignored():
    bot = make_bot()
    bot.state = 0
    bot.level = 1
    bot.inventory_items["food"] = 50
    foreign = (bot.key + 1) % 999
    msg = f"4, {foreign}_R_1_777"
    bot.net.feed_event("m", msg)
    run(bot.handle_events())
    assert bot.state == 0
