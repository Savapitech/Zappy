"""Pure decision logic of the bot (no I/O)."""

import const
from fakes import make_bot


def test_broadcast_key_is_deterministic_for_a_team():
    a = make_bot("alpha")
    b = make_bot("alpha")
    assert a.key == b.key


def test_broadcast_key_differs_between_teams_usually():
    a = make_bot("alpha")
    b = make_bot("beta")
    # Not guaranteed in theory, but these two names hash differently.
    assert a.key != b.key


def test_broadcast_key_is_bounded():
    for name in ["a", "team1", "GRAPHIC", "longteamname", "zZz"]:
        bot = make_bot(name)
        assert 0 <= bot.key < 999


def test_new_bot_starts_at_level_one_collecting():
    bot = make_bot()
    assert bot.level == 1
    assert bot.state == 0
    assert bot.leader_id is None
    assert bot.leader_dir == -1
    assert not bot.forked


def test_inventory_starts_zeroed_for_every_resource():
    bot = make_bot()
    for resource in const.RESOURCES:
        assert bot.inventory_items[resource] == 0


def _fill_for_level(bot, level, factor=1):
    req = const.ELEVATION_REQS[level - 1]
    for idx, item in enumerate(const.RESOURCES[1:]):
        bot.inventory_items[item] = req[idx + 1] * factor


def test_has_stones_false_when_inventory_empty():
    for level in range(1, 8):
        bot = make_bot()
        bot.level = level
        assert bot.has_stones() is False


def test_has_stones_true_with_exact_requirements_every_level():
    for level in range(1, 8):
        bot = make_bot()
        bot.level = level
        _fill_for_level(bot, level)
        assert bot.has_stones() is True, f"level {level} should be ready"


def test_has_stones_true_with_surplus():
    for level in range(1, 8):
        bot = make_bot()
        bot.level = level
        _fill_for_level(bot, level, factor=3)
        assert bot.has_stones() is True


def test_has_stones_false_when_one_stone_missing():
    for level in range(1, 8):
        req = const.ELEVATION_REQS[level - 1]
        for idx, item in enumerate(const.RESOURCES[1:]):
            if req[idx + 1] == 0:
                continue
            bot = make_bot()
            bot.level = level
            _fill_for_level(bot, level)
            bot.inventory_items[item] -= 1
            assert bot.has_stones() is False, (
                f"level {level} missing one {item} must not be ready")


def test_has_stones_false_at_max_level():
    bot = make_bot()
    bot.level = 8
    for item in const.RESOURCES[1:]:
        bot.inventory_items[item] = 99
    assert bot.has_stones() is False


def test_has_stones_ignores_food():
    bot = make_bot()
    bot.level = 2
    bot.inventory_items["food"] = 9999
    assert bot.has_stones() is False
