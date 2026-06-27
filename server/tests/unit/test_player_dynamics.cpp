#include <criterion/criterion.h>

#include "Game/Common.hpp"
#include "Game/Player.hpp"

using game::Player;

// Movement, orientation and survival mechanics, exercised across a wide range
// of inputs. These complement test_player.cpp without overlapping its names.

// --- Forward over many tiles --------------------------------------------

Test(player_move, north_walks_straight_in_negative_y) {
  Player p(1, "t");
  p.setPos(0, 100);
  p.setOrientation(N);
  for (int i = 1; i <= 50; i++) {
    p.forward();
    cr_assert_eq(p.getY(), 100 - i);
    cr_assert_eq(p.getX(), 0);
  }
}

Test(player_move, south_walks_straight_in_positive_y) {
  Player p(1, "t");
  p.setPos(0, 0);
  p.setOrientation(S);
  for (int i = 1; i <= 50; i++) {
    p.forward();
    cr_assert_eq(p.getY(), i);
  }
}

Test(player_move, east_walks_straight_in_positive_x) {
  Player p(1, "t");
  p.setPos(0, 0);
  p.setOrientation(E);
  for (int i = 1; i <= 50; i++) {
    p.forward();
    cr_assert_eq(p.getX(), i);
  }
}

Test(player_move, west_walks_straight_in_negative_x) {
  Player p(1, "t");
  p.setPos(100, 0);
  p.setOrientation(W);
  for (int i = 1; i <= 50; i++) {
    p.forward();
    cr_assert_eq(p.getX(), 100 - i);
  }
}

Test(player_move, forward_never_touches_the_perpendicular_axis) {
  int dirs[] = {N, E, S, W};
  for (int d : dirs) {
    Player p(1, "t");
    p.setPos(7, 7);
    p.setOrientation(d);
    p.forward();
    if (d == N || d == S)
      cr_assert_eq(p.getX(), 7);
    else
      cr_assert_eq(p.getY(), 7);
  }
}

// --- Rotations -----------------------------------------------------------

Test(player_rotate, full_right_cycle_is_identity_modulo_four) {
  Player p(1, "t");
  for (int start : {N, E, S, W}) {
    p.setOrientation(start);
    for (int i = 0; i < 4; i++)
      p.turnRight();
    cr_assert_eq(p.getOrientation(), start);
  }
}

Test(player_rotate, full_left_cycle_is_identity_modulo_four) {
  Player p(1, "t");
  for (int start : {N, E, S, W}) {
    p.setOrientation(start);
    for (int i = 0; i < 4; i++)
      p.turnLeft();
    cr_assert_eq(p.getOrientation(), start);
  }
}

Test(player_rotate, right_follows_n_e_s_w_order) {
  Player p(1, "t");
  p.setOrientation(N);
  p.turnRight();
  cr_assert_eq(p.getOrientation(), E);
  p.turnRight();
  cr_assert_eq(p.getOrientation(), S);
  p.turnRight();
  cr_assert_eq(p.getOrientation(), W);
  p.turnRight();
  cr_assert_eq(p.getOrientation(), N);
}

Test(player_rotate, left_is_the_inverse_of_right_everywhere) {
  Player p(1, "t");
  for (int start : {N, E, S, W}) {
    p.setOrientation(start);
    p.turnRight();
    p.turnLeft();
    cr_assert_eq(p.getOrientation(), start);
  }
}

Test(player_rotate, twelve_rights_equals_zero_net_rotation) {
  Player p(1, "t");
  p.setOrientation(E);
  for (int i = 0; i < 12; i++)
    p.turnRight();
  cr_assert_eq(p.getOrientation(), E);
}

// --- Life / food ---------------------------------------------------------

Test(player_life, fresh_player_has_full_survival_time) {
  Player p(1, "t");
  cr_assert_eq(p.getLife(), SURVIVAL_TIME);
  cr_assert_eq(p.getRessource(FOOD_IDX), START_FOOD);
  cr_assert_not(p.isDead());
}

Test(player_life, removing_one_food_worth_keeps_player_alive) {
  Player p(1, "t");
  p.removeLife(SURVIVAL_TIME);
  cr_assert_not(p.isDead());
  cr_assert_eq(p.getRessource(FOOD_IDX), START_FOOD - 1);
}

Test(player_life, each_food_unit_buys_one_survival_window) {
  Player p(1, "t");
  for (int eaten = 1; eaten <= START_FOOD - 1; eaten++) {
    p.removeLife(SURVIVAL_TIME);
    cr_assert_not(p.isDead(), "died too early after %d windows", eaten);
    cr_assert_eq(p.getRessource(FOOD_IDX), START_FOOD - eaten);
  }
}

Test(player_life, dies_once_all_food_is_consumed) {
  Player p(1, "t");
  p.removeLife(SURVIVAL_TIME * START_FOOD);
  cr_assert(p.isDead());
  cr_assert_eq(p.getLife(), 0);
}

Test(player_life, adding_food_then_starving_takes_longer) {
  Player p(1, "t");
  p.addRessource(FOOD_IDX, 5);
  int total = (START_FOOD + 5);
  p.removeLife(SURVIVAL_TIME * total);
  cr_assert(p.isDead());
}

Test(player_life, partial_damage_does_not_consume_food) {
  Player p(1, "t");
  p.removeLife(SURVIVAL_TIME / 2);
  cr_assert_eq(p.getRessource(FOOD_IDX), START_FOOD);
  cr_assert_not(p.isDead());
}

Test(player_life, addLife_can_extend_beyond_one_window) {
  Player p(1, "t");
  p.addLife(1000);
  cr_assert_eq(p.getLife(), SURVIVAL_TIME + 1000);
}

Test(player_life, dead_stays_dead_after_more_damage) {
  // Once a player is dead, removeLife no longer converts food (there is none
  // left) and the death flag is sticky.
  Player p(1, "t");
  p.removeLife(SURVIVAL_TIME * START_FOOD);
  cr_assert(p.isDead());
  cr_assert_eq(p.getRessource(FOOD_IDX), 0);
  p.removeLife(99999);
  cr_assert(p.isDead());
  cr_assert(p.getLife() <= 0);
}

// --- Levels --------------------------------------------------------------

Test(player_level, climbs_one_level_at_a_time) {
  Player p(1, "t");
  for (int expected = 2; expected <= MAX_LVL; expected++) {
    p.levelup();
    cr_assert_eq(p.getLevel(), expected);
  }
}

Test(player_level, new_player_starts_at_level_one) {
  Player p(42, "t");
  cr_assert_eq(p.getLevel(), 1);
}

// --- Inventory -----------------------------------------------------------

Test(player_inv, every_slot_is_independent) {
  Player p(1, "t");
  for (int i = 1; i < RESOURCE_COUNT; i++)
    p.addRessource(i, i * 3);
  for (int i = 1; i < RESOURCE_COUNT; i++)
    cr_assert_eq(p.getRessource(i), i * 3);
}

Test(player_inv, add_then_remove_balances_out) {
  Player p(1, "t");
  for (int i = 1; i < RESOURCE_COUNT; i++) {
    p.addRessource(i, 10);
    p.removeRessource(i, 4);
    cr_assert_eq(p.getRessource(i), 6);
  }
}

Test(player_inv, identity_and_team_are_preserved) {
  Player p(123, "blue");
  cr_assert_eq(p.getId(), 123);
  cr_assert_str_eq(p.getTeamname().c_str(), "blue");
}

Test(player_inv, position_setter_is_exact) {
  Player p(1, "t");
  for (int x = -5; x <= 5; x++)
    for (int y = -5; y <= 5; y++) {
      p.setPos(x, y);
      cr_assert_eq(p.getX(), x);
      cr_assert_eq(p.getY(), y);
    }
}
