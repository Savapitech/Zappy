#include <criterion/criterion.h>
#include <set>

#include "Game/GameLogic.hpp"
#include "Game/Player.hpp"
#include "Game/Common.hpp"

Test(initTeams, creates_all_requested_teams)
{
    game::GameLogic g(10, 10, 1, 1,
        {"team1", "team2", "team3"});

    cr_assert_eq(g.getTeams().size(), (size_t)3);

    cr_assert_str_eq(
        g.getTeams()[0]->getName().c_str(),
        "team1");

    cr_assert_str_eq(
        g.getTeams()[1]->getName().c_str(),
        "team2");

    cr_assert_str_eq(
        g.getTeams()[2]->getName().c_str(),
        "team3");
}

Test(constructor, initializes_map_dimensions)
{
    game::GameLogic g(17, 23, 1, 1, {"team1"});

    cr_assert_eq(g.getMapX(), 17);
    cr_assert_eq(g.getMapY(), 23);
}

Test(initEggs, generated_eggs_have_unique_ids)
{
    game::GameLogic g(
        10,
        10,
        1,
        5,
        {"team1", "team2"});

    std::set<int> ids;

    for (const auto &team : g.getTeams()) {
        for (const auto &egg : team->getEggs()) {

            cr_assert(
                ids.insert(egg->getId()).second,
                "duplicate egg id %d",
                egg->getId());
        }
    }
}

Test(getPlayerById, returns_nullptr_when_player_does_not_exist)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    cr_assert_eq(
        g.getPlayerById(424242),
        nullptr);
}

Test(getPlayerById, returns_correct_player)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    auto player =
        std::make_shared<game::Player>(
            42,
            "team1");

    g.getTeams()[0]->addPlayer(player);

    auto found = g.getPlayerById(42);

    cr_assert_not_null(found);
    cr_assert_eq(found->getId(), 42);
}

Test(getDir, same_position_returns_zero)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player a(1, "team1");
    game::Player b(2, "team1");

    a.setPos(5, 5);
    b.setPos(5, 5);

    a.setOrientation(N);

    cr_assert_eq(
        g.getDir(a, b, 10, 10),
        0);
}

Test(getDir, north_front_returns_one)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player sender(1, "team1");
    game::Player receiver(2, "team1");

    sender.setPos(5, 5);
    receiver.setPos(5, 4);

    sender.setOrientation(N);

    cr_assert_eq(
        g.getDir(sender, receiver, 10, 10),
        1);
}

Test(getDir, north_right_returns_seven)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player sender(1, "team1");
    game::Player receiver(2, "team1");

    sender.setPos(5, 5);
    receiver.setPos(6, 5);

    sender.setOrientation(N);

    cr_assert_eq(
        g.getDir(sender, receiver, 10, 10),
        7);
}

Test(getDir, north_left_returns_three)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player sender(1, "team1");
    game::Player receiver(2, "team1");

    sender.setPos(5, 5);
    receiver.setPos(4, 5);

    sender.setOrientation(N);

    cr_assert_eq(
        g.getDir(sender, receiver, 10, 10),
        3);
}

Test(getDir, north_back_returns_five)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player sender(1, "team1");
    game::Player receiver(2, "team1");

    sender.setPos(5, 5);
    receiver.setPos(5, 6);

    sender.setOrientation(N);

    cr_assert_eq(
        g.getDir(sender, receiver, 10, 10),
        5);
}

Test(getDir, east_front_returns_one)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player sender(1, "team1");
    game::Player receiver(2, "team1");

    sender.setPos(5, 5);
    receiver.setPos(6, 5);

    sender.setOrientation(E);

    cr_assert_eq(
        g.getDir(sender, receiver, 10, 10),
        1);
}

Test(getDir, south_front_returns_one)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player sender(1, "team1");
    game::Player receiver(2, "team1");

    sender.setPos(5, 5);
    receiver.setPos(5, 6);

    sender.setOrientation(S);

    cr_assert_eq(
        g.getDir(sender, receiver, 10, 10),
        1);
}

Test(getDir, west_front_returns_one)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player sender(1, "team1");
    game::Player receiver(2, "team1");

    sender.setPos(5, 5);
    receiver.setPos(4, 5);

    sender.setOrientation(W);

    cr_assert_eq(
        g.getDir(sender, receiver, 10, 10),
        1);
}

Test(getDir, map_wrap_horizontal_is_taken_into_account)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player sender(1, "team1");
    game::Player receiver(2, "team1");

    sender.setPos(0, 5);
    receiver.setPos(9, 5);

    sender.setOrientation(N);

    cr_assert_eq(
        g.getDir(sender, receiver, 10, 10),
        3);
}

Test(getDir, map_wrap_vertical_is_taken_into_account)
{
    game::GameLogic g(10, 10, 1, 1, {"team1"});

    game::Player sender(1, "team1");
    game::Player receiver(2, "team1");

    sender.setPos(5, 0);
    receiver.setPos(5, 9);

    sender.setOrientation(N);

    cr_assert_eq(
        g.getDir(sender, receiver, 10, 10),
        1);
}
