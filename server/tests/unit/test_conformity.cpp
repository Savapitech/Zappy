#include <criterion/criterion.h>

#include "Game/Common.hpp"

Test(conformity_density, food_is_half) {
  cr_assert_float_eq(FOOD_DENSITY, 0.5, 1e-9);
}
Test(conformity_density, linemate_is_0_3) {
  cr_assert_float_eq(LINEMATE_DENSITY, 0.3, 1e-9);
}
Test(conformity_density, deraumere_is_0_15) {
  cr_assert_float_eq(DERAUMERE_DENSITY, 0.15, 1e-9);
}
Test(conformity_density, sibur_is_0_1) {
  cr_assert_float_eq(SIBUR_DENSITY, 0.1, 1e-9);
}
Test(conformity_density, mendiane_is_0_1) {
  cr_assert_float_eq(MENDIANE_DENSITY, 0.1, 1e-9);
}
Test(conformity_density, phiras_is_0_08) {
  cr_assert_float_eq(PHIRAS_DENSITY, 0.08, 1e-9);
}
Test(conformity_density, thystame_is_0_05) {
  cr_assert_float_eq(THYSTAME_DENSITY, 0.05, 1e-9);
}
Test(conformity_density, food_is_the_most_common_resource) {
  cr_assert(FOOD_DENSITY > LINEMATE_DENSITY);
  cr_assert(LINEMATE_DENSITY > DERAUMERE_DENSITY);
  cr_assert(DERAUMERE_DENSITY >= SIBUR_DENSITY);
}
Test(conformity_density, thystame_is_the_rarest_resource) {
  cr_assert(THYSTAME_DENSITY < PHIRAS_DENSITY);
  cr_assert(THYSTAME_DENSITY < MENDIANE_DENSITY);
  cr_assert(THYSTAME_DENSITY < SIBUR_DENSITY);
}


Test(conformity_rules, one_food_is_126_time_units) {
  cr_assert_eq(SURVIVAL_TIME, 126);
}
Test(conformity_rules, players_start_with_10_food) {
  cr_assert_eq(START_FOOD, 10);
}
Test(conformity_rules, max_level_is_8) {
  cr_assert_eq(MAX_LVL, 8);
}
Test(conformity_rules, win_needs_six_players_at_max_level) {
  cr_assert_eq(WIN_COND, 6);
}
Test(conformity_rules, resources_respawn_every_20_time_units) {
  cr_assert_eq(RESOURCE_RESPAWN, 20);
}
Test(conformity_rules, command_queue_caps_at_10) {
  cr_assert_eq(MAX_CMD_QUEUE, 10);
}
Test(conformity_rules, starting_life_lasts_1260_time_units) {
  // 10 food * 126 time units = 1260, as stated by the subject.
  cr_assert_eq(START_FOOD * SURVIVAL_TIME, 1260);
}

Test(conformity_cost, forward_is_7) { cr_assert_eq(TIME_FORWARD, 7); }
Test(conformity_cost, right_is_7) { cr_assert_eq(TIME_RIGHT, 7); }
Test(conformity_cost, left_is_7) { cr_assert_eq(TIME_LEFT, 7); }
Test(conformity_cost, look_is_7) { cr_assert_eq(TIME_LOOK, 7); }
Test(conformity_cost, inventory_is_1) { cr_assert_eq(TIME_INVENTORY, 1); }
Test(conformity_cost, broadcast_is_7) { cr_assert_eq(TIME_BROADCAST, 7); }
Test(conformity_cost, eject_is_7) { cr_assert_eq(TIME_EJECT, 7); }
Test(conformity_cost, take_is_7) { cr_assert_eq(TIME_TAKE, 7); }
Test(conformity_cost, set_is_7) { cr_assert_eq(TIME_SET, 7); }
Test(conformity_cost, fork_is_42) { cr_assert_eq(TIME_FORK, 42); }
Test(conformity_cost, incantation_is_300) { cr_assert_eq(TIME_INCANTATION, 300); }
Test(conformity_cost, inventory_is_the_cheapest_action) {
  cr_assert(TIME_INVENTORY < TIME_FORWARD);
  cr_assert(TIME_INVENTORY < TIME_FORK);
}
Test(conformity_cost, incantation_is_the_most_expensive_action) {
  cr_assert(TIME_INCANTATION > TIME_FORK);
  cr_assert(TIME_INCANTATION > TIME_FORWARD);
}

Test(conformity_index, resource_count_is_7) {
  cr_assert_eq(RESOURCE_COUNT, 7);
}
Test(conformity_index, food_is_index_zero) { cr_assert_eq(FOOD_IDX, 0); }
Test(conformity_index, indices_are_contiguous_and_ordered) {
  cr_assert_eq(LINEMATE_IDX, 1);
  cr_assert_eq(DERAUMERE_IDX, 2);
  cr_assert_eq(SIBUR_IDX, 3);
  cr_assert_eq(MENDIANE_IDX, 4);
  cr_assert_eq(PHIRAS_IDX, 5);
  cr_assert_eq(THYSTAME_IDX, 6);
}
Test(conformity_index, all_indices_are_distinct_and_in_range) {
  int idx[] = {FOOD_IDX,     LINEMATE_IDX, DERAUMERE_IDX, SIBUR_IDX,
               MENDIANE_IDX, PHIRAS_IDX,   THYSTAME_IDX};
  for (int i = 0; i < RESOURCE_COUNT; i++) {
    cr_assert(idx[i] >= 0 && idx[i] < RESOURCE_COUNT);
    for (int j = i + 1; j < RESOURCE_COUNT; j++)
      cr_assert_neq(idx[i], idx[j], "indices %d and %d collide", i, j);
  }
}

Test(conformity_orientation, cardinal_values_are_1_to_4) {
  cr_assert_eq(N, 1);
  cr_assert_eq(E, 2);
  cr_assert_eq(S, 3);
  cr_assert_eq(W, 4);
}
Test(conformity_orientation, all_four_are_distinct) {
  int o[] = {N, E, S, W};
  for (int i = 0; i < 4; i++)
    for (int j = i + 1; j < 4; j++)
      cr_assert_neq(o[i], o[j]);
}

Test(conformity_misc, gui_team_name_is_graphic) {
  cr_assert_str_eq(GUI_TEAM, "GRAPHIC");
}
Test(conformity_misc, game_tick_is_100ms) {
  cr_assert_eq(GAME_TICK_MS, 100);
}
