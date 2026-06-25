#include <criterion/criterion.h>

#include "Game/Common.hpp"
#include "Game/Player.hpp"

Test(player, initial_state) {
  game::Player p(42, "team1");

  cr_assert_eq(p.getId(), 42);
  cr_assert_eq(p.getLevel(), 1);
  cr_assert_eq(p.getOrientation(), N);
  cr_assert_eq(p.getRessource(FOOD_IDX), START_FOOD);
  cr_assert_eq(p.getLife(), SURVIVAL_TIME);
  cr_assert_not(p.isDead());
  cr_assert_not(p.isEvolving());
  cr_assert_str_eq(p.getTeamname().c_str(), "team1");
  for (int i = 1; i < RESOURCE_COUNT; i++)
    cr_assert_eq(p.getRessource(i), 0, "non-food resource %d should start at 0", i);
}

Test(player, forward_each_orientation) {
  game::Player p(1, "t");

  p.setPos(5, 5);
  p.setOrientation(N);
  p.forward();
  cr_assert_eq(p.getX(), 5);
  cr_assert_eq(p.getY(), 4);

  p.setPos(5, 5);
  p.setOrientation(S);
  p.forward();
  cr_assert_eq(p.getX(), 5);
  cr_assert_eq(p.getY(), 6);

  p.setPos(5, 5);
  p.setOrientation(E);
  p.forward();
  cr_assert_eq(p.getX(), 6);
  cr_assert_eq(p.getY(), 5);

  p.setPos(5, 5);
  p.setOrientation(W);
  p.forward();
  cr_assert_eq(p.getX(), 4);
  cr_assert_eq(p.getY(), 5);
}

Test(player, turn_right_cycle) {
  game::Player p(1, "t");
  p.setOrientation(N);

  p.turnRight();
  cr_assert_eq(p.getOrientation(), E);
  p.turnRight();
  cr_assert_eq(p.getOrientation(), S);
  p.turnRight();
  cr_assert_eq(p.getOrientation(), W);
  p.turnRight();
  cr_assert_eq(p.getOrientation(), N, "turning right 4 times must wrap back to N");
}

Test(player, turn_left_cycle) {
  game::Player p(1, "t");
  p.setOrientation(N);

  p.turnLeft();
  cr_assert_eq(p.getOrientation(), W);
  p.turnLeft();
  cr_assert_eq(p.getOrientation(), S);
  p.turnLeft();
  cr_assert_eq(p.getOrientation(), E);
  p.turnLeft();
  cr_assert_eq(p.getOrientation(), N, "turning left 4 times must wrap back to N");
}

Test(player, resource_add_remove) {
  game::Player p(1, "t");

  p.addRessource(LINEMATE_IDX, 3);
  cr_assert_eq(p.getRessource(LINEMATE_IDX), 3);
  p.removeRessource(LINEMATE_IDX, 1);
  cr_assert_eq(p.getRessource(LINEMATE_IDX), 2);
}

Test(player, levelup_increments) {
  game::Player p(1, "t");
  cr_assert_eq(p.getLevel(), 1);
  p.levelup();
  cr_assert_eq(p.getLevel(), 2);
  p.levelup();
  cr_assert_eq(p.getLevel(), 3);
}

Test(player, evolving_flag) {
  game::Player p(1, "t");
  cr_assert_not(p.isEvolving());
  p.setEvolving(true);
  cr_assert(p.isEvolving());
  p.setEvolving(false);
  cr_assert_not(p.isEvolving());
}

// One unit of food sustains the player for SURVIVAL_TIME (126) time units.
// A fresh player starts with START_FOOD (10) food, so total survival is
// START_FOOD * SURVIVAL_TIME time units before starving to death.
Test(player, starves_after_exactly_total_survival_time) {
  game::Player p(1, "t");
  int total = START_FOOD * SURVIVAL_TIME;

  for (int i = 0; i < total - 1; i++) {
    p.removeLife(1);
    cr_assert_not(p.isDead(), "player died early at tick %d/%d", i + 1, total);
  }
  cr_assert_eq(p.getRessource(FOOD_IDX), 1,
              "exactly 1 food unit should remain just before the last tick");
  p.removeLife(1);
  cr_assert(p.isDead(), "player should be dead exactly at tick %d", total);
  cr_assert_eq(p.getRessource(FOOD_IDX), 0);
}

Test(player, food_decreases_every_survival_time_ticks) {
  game::Player p(1, "t");

  for (int eaten = 1; eaten <= START_FOOD; eaten++) {
    p.removeLife(SURVIVAL_TIME);
    cr_assert_eq(p.getRessource(FOOD_IDX), START_FOOD - eaten,
                "after %d ticks of %d, expected %d food left", eaten * SURVIVAL_TIME,
                SURVIVAL_TIME, START_FOOD - eaten);
  }
  cr_assert(p.isDead());
}

Test(player, taking_food_extends_survival) {
  game::Player p(1, "t");
  p.addRessource(FOOD_IDX, 5);
  cr_assert_eq(p.getRessource(FOOD_IDX), START_FOOD + 5);

  int total = (START_FOOD + 5) * SURVIVAL_TIME;
  p.removeLife(total - 1);
  cr_assert_not(p.isDead());
  p.removeLife(1);
  cr_assert(p.isDead());
}

Test(player, removeLife_handles_large_batches_in_one_call) {
  // The life tick loop must catch up correctly even if many ticks elapse
  // between two calls (e.g. when the server loop is delayed).
  game::Player p(1, "t");
  int total = START_FOOD * SURVIVAL_TIME;

  p.removeLife(total - 1);
  cr_assert_not(p.isDead());
  p.removeLife(1);
  cr_assert(p.isDead());
}

Test(player, forward_moves_correctly_from_arbitrary_position) {
    game::Player p(1, "t");

    // N : y diminue
    p.setPos(10, 10); p.setOrientation(N); p.forward();
    cr_assert_eq(p.getX(), 10); cr_assert_eq(p.getY(), 9);

    // S : y augmente
    p.setPos(10, 10); p.setOrientation(S); p.forward();
    cr_assert_eq(p.getX(), 10); cr_assert_eq(p.getY(), 11);

    // E : x augmente
    p.setPos(10, 10); p.setOrientation(E); p.forward();
    cr_assert_eq(p.getX(), 11); cr_assert_eq(p.getY(), 10);

    // W : x diminue
    p.setPos(10, 10); p.setOrientation(W); p.forward();
    cr_assert_eq(p.getX(), 9); cr_assert_eq(p.getY(), 10);
}

Test(player, remove_more_than_stock_goes_negative_or_zero) {
    game::Player p(1, "t");
    p.addRessource(LINEMATE_IDX, 2);
    p.removeRessource(LINEMATE_IDX, 5);
    cr_assert(p.getRessource(LINEMATE_IDX) <= 0,
        "removing more than stock must not leave a positive value");
}

Test(player, add_ressource_zero_is_noop) {
    game::Player p(1, "t");
    p.addRessource(DERAUMERE_IDX, 0);
    cr_assert_eq(p.getRessource(DERAUMERE_IDX), 0);
}

Test(player, all_resource_slots_independent) {
    game::Player p(1, "t");
    for (int i = 1; i < RESOURCE_COUNT; i++)
        p.addRessource(i, i * 10);
    for (int i = 1; i < RESOURCE_COUNT; i++)
        cr_assert_eq(p.getRessource(i), i * 10,
            "slot %d should be %d", i, i * 10);
}


Test(player, addLife_increases_life_units) {
    game::Player p(1, "t");
    int before = p.getLife();
    p.addLife(50);
    cr_assert_eq(p.getLife(), before + 50);
}

Test(player, removeLife_zero_is_noop) {
    game::Player p(1, "t");
    int before = p.getLife();
    p.removeLife(0);
    cr_assert_eq(p.getLife(), before);
    cr_assert_not(p.isDead());
}

Test(player, removeLife_on_dead_player_is_idempotent) {
    game::Player p(1, "t");
    p.removeLife(START_FOOD * SURVIVAL_TIME);
    cr_assert(p.isDead());
    cr_assert_eq(p.getRessource(FOOD_IDX), 0);

    p.removeLife(999);
    cr_assert(p.isDead(), "isDead must stay true after extra removeLife");
    cr_assert_eq(p.getRessource(FOOD_IDX), 0,
        "food must not go below 0 after player is already dead");
}

Test(player, levelup_reaches_max_level) {
    game::Player p(1, "t");
    for (int i = 1; i < MAX_LVL; i++)
        p.levelup();
    cr_assert_eq(p.getLevel(), MAX_LVL);
}


Test(player, set_orientation_all_values) {
    game::Player p(1, "t");
    int dirs[] = {N, E, S, W};
    for (int d : dirs) {
        p.setOrientation(d);
        cr_assert_eq(p.getOrientation(), d);
    }
}

Test(player, turn_right_then_left_returns_to_original) {
    game::Player p(1, "t");
    p.setOrientation(E);
    p.turnRight();
    p.turnLeft();
    cr_assert_eq(p.getOrientation(), E,
        "right then left must cancel out");
}


Test(player, getClient_returns_nullptr_when_not_set) {
    game::Player p(1, "t");
    cr_assert_null(p.getClient(),
        "getClient() must return nullptr before any setClient call");
}


Test(player, evolving_does_not_affect_death) {
    game::Player p(1, "t");
    p.setEvolving(true);
    p.removeLife(START_FOOD * SURVIVAL_TIME);
    cr_assert(p.isDead(), "evolving flag must not prevent death from starvation");
}
