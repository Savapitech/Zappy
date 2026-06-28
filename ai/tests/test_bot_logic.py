"""Bot strategy layer: identity, initial state, and event handling (leader
election / ejection), driven by a fake network."""

import const
from fakes import make_bot, run


def test_broadcast_key_is_deterministic_for_a_team():
    a = make_bot("alpha")
    b = make_bot("alpha")
    assert a.key == b.key


def test_broadcast_key_differs_between_teams_usually():
    a = make_bot("alpha")
    b = make_bot("beta")
    assert a.key != b.key


def test_broadcast_key_is_bounded():
    for name in ["a", "team1", "GRAPHIC", "longteamname", "zZz"]:
        bot = make_bot(name)
        assert 0 <= bot.key < 999


def test_new_bot_starts_at_level_one_collecting():
    bot = make_bot()
    assert bot.player.level == 1
    assert bot.state == 0
    assert bot.leader_id is None
    assert bot.leader_dir == -1
    assert not bot.forked


def test_inventory_starts_zeroed_for_every_resource():
    bot = make_bot()
    for resource in const.RESOURCES:
        assert bot.player.inventory_items[resource] == 0


def test_passive_level_up_resets_strategy_state_and_records_stats():
    import stats
    bot = make_bot()
    bot.state = 2
    before1 = stats.STATS["levels"][1]
    before2 = stats.STATS["levels"][2]
    bot.player.net.feed("Current level: 2", "ok")
    run(bot.player.cmd("Look"))
    assert bot.player.level == 2
    assert bot.state == 0
    assert stats.STATS["levels"][1] == before1 - 1
    assert stats.STATS["levels"][2] == before2 + 1
    stats.STATS["levels"][1] = before1
    stats.STATS["levels"][2] = before2


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
    bot.player.level = 1
    bot.player.inventory_items["food"] = 50
    bot.net.feed_event("m", f"4, {bot.key}_R_1_777")
    run(bot.handle_events())
    assert bot.state == 2
    assert bot.leader_id == "777"
    assert bot.leader_dir == 4


def test_broadcast_from_other_level_is_ignored():
    bot = make_bot()
    bot.state = 0
    bot.player.level = 3
    bot.player.inventory_items["food"] = 50
    bot.net.feed_event("m", f"2, {bot.key}_R_1_777")
    run(bot.handle_events())
    assert bot.state == 0


def test_broadcast_with_foreign_key_is_ignored():
    bot = make_bot()
    bot.state = 0
    bot.player.level = 1
    bot.player.inventory_items["food"] = 50
    foreign = (bot.key + 1) % 999
    bot.net.feed_event("m", f"4, {foreign}_R_1_777")
    run(bot.handle_events())
    assert bot.state == 0


def test_leader_yields_to_smaller_id():
    bot = make_bot()
    bot.id = "500"
    bot.state = 1
    bot.player.level = 1
    bot.player.inventory_items["food"] = 50
    bot.net.feed_event("m", f"4, {bot.key}_R_1_200")
    run(bot.handle_events())
    assert bot.state == 2
    assert bot.leader_id == "200"
