#include <criterion/criterion.h>

#include "Game/Common.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"

Test(tile, starts_empty) {
  game::Tile t;
  for (int i = 0; i < RESOURCE_COUNT; i++)
    cr_assert_eq(t.getRessource(i), 0);
}

Test(tile, add_and_remove_resource) {
  game::Tile t;
  t.addRessource(FOOD_IDX, 3);
  cr_assert_eq(t.getRessource(FOOD_IDX), 3);
  t.addRessource(FOOD_IDX, 2);
  cr_assert_eq(t.getRessource(FOOD_IDX), 5);
  t.removeRessource(FOOD_IDX, 4);
  cr_assert_eq(t.getRessource(FOOD_IDX), 1);
}

Test(tile, resources_are_independent_per_index) {
  game::Tile t;
  t.addRessource(FOOD_IDX, 1);
  t.addRessource(LINEMATE_IDX, 1);
  cr_assert_eq(t.getRessource(DERAUMERE_IDX), 0);
  cr_assert_eq(t.getRessource(FOOD_IDX), 1);
  cr_assert_eq(t.getRessource(LINEMATE_IDX), 1);
}

Test(map, in_bounds_tiles_are_independent) {
  game::Map m(5, 5);
  m.getTile(0, 0).addRessource(FOOD_IDX, 1);
  m.getTile(1, 0).addRessource(FOOD_IDX, 2);
  cr_assert_eq(m.getTile(0, 0).getRessource(FOOD_IDX), 1);
  cr_assert_eq(m.getTile(1, 0).getRessource(FOOD_IDX), 2);
}

// The world wraps around: exiting right comes back through the left, and
// likewise for top/bottom, per the subject's "Geography" section.
Test(map, wraps_around_horizontally) {
  game::Map m(5, 5);
  m.getTile(0, 0).addRessource(FOOD_IDX, 7);
  cr_assert_eq(m.getTile(5, 0).getRessource(FOOD_IDX), 7,
              "x == width must wrap back to x == 0");
  cr_assert_eq(m.getTile(-1, 0).getRessource(FOOD_IDX), 0,
              "x == -1 must wrap to x == width-1, an untouched tile");
}

Test(map, wraps_around_vertically) {
  game::Map m(5, 5);
  m.getTile(0, 0).addRessource(LINEMATE_IDX, 4);
  cr_assert_eq(m.getTile(0, 5).getRessource(LINEMATE_IDX), 4,
              "y == height must wrap back to y == 0");
}

Test(map, negative_coordinates_wrap_to_last_tile) {
  game::Map m(5, 5);
  m.getTile(4, 4).addRessource(SIBUR_IDX, 9);
  cr_assert_eq(m.getTile(-1, -1).getRessource(SIBUR_IDX), 9);
}

Test(map, large_multiple_wraps_resolve_to_same_tile) {
  game::Map m(5, 5);
  m.getTile(2, 3).addRessource(PHIRAS_IDX, 1);
  cr_assert_eq(m.getTile(2 + 5 * 10, 3 - 5 * 7).getRessource(PHIRAS_IDX), 1);
}

Test(map, dimensions_are_stored_correctly) {
    game::Map m(10, 20);
    cr_assert_eq(m.getWidth(), 10);
    cr_assert_eq(m.getHeigth(), 20);
}

Test(map, square_map) {
    game::Map m(5, 5);
    cr_assert_eq(m.getWidth(), 5);
    cr_assert_eq(m.getHeigth(), 5);
}

Test(map, minimal_1x1_map) {
    game::Map m(1, 1);
    cr_assert_eq(m.getWidth(), 1);
    cr_assert_eq(m.getHeigth(), 1);
    // toute coordonnée wrap sur la seule tuile
    game::Tile &t = m.getTile(0, 0);
    cr_assert_eq(&m.getTile(99, 99), &t);
}


Test(map, getTile_returns_consistent_reference) {
    game::Map m(10, 10);
    game::Tile &a = m.getTile(3, 4);
    game::Tile &b = m.getTile(3, 4);
    cr_assert_eq(&a, &b, "same coords must return the same Tile object");
}

Test(map, getTile_different_coords_are_different_tiles) {
    game::Map m(10, 10);
    cr_assert_neq(&m.getTile(0, 0), &m.getTile(1, 0));
    cr_assert_neq(&m.getTile(0, 0), &m.getTile(0, 1));
    cr_assert_neq(&m.getTile(2, 3), &m.getTile(3, 2));
}

Test(map, getTile_covers_all_corners) {
    game::Map m(8, 6);
    game::Tile *tl = &m.getTile(0, 0);
    game::Tile *tr = &m.getTile(7, 0);
    game::Tile *bl = &m.getTile(0, 5);
    game::Tile *br = &m.getTile(7, 5);
    cr_assert_neq(tl, tr);
    cr_assert_neq(tl, bl);
    cr_assert_neq(tl, br);
    cr_assert_neq(tr, bl);
    cr_assert_neq(tr, br);
    cr_assert_neq(bl, br);
}


Test(map, getTile_wraps_x_at_width) {
    game::Map m(10, 10);
    cr_assert_eq(&m.getTile(10, 0), &m.getTile(0, 0));
    cr_assert_eq(&m.getTile(11, 0), &m.getTile(1, 0));
    cr_assert_eq(&m.getTile(20, 0), &m.getTile(0, 0));
}

Test(map, getTile_wraps_y_at_height) {
    game::Map m(10, 10);
    cr_assert_eq(&m.getTile(0, 10), &m.getTile(0, 0));
    cr_assert_eq(&m.getTile(0, 11), &m.getTile(0, 1));
    cr_assert_eq(&m.getTile(0, 20), &m.getTile(0, 0));
}

Test(map, getTile_wraps_both_axes) {
    game::Map m(10, 10);
    cr_assert_eq(&m.getTile(10, 10), &m.getTile(0, 0));
    cr_assert_eq(&m.getTile(13, 17), &m.getTile(3, 7));
}

Test(map, getTile_large_positive_offset_wraps_correctly) {
    game::Map m(7, 5);
    // 7*k + r doit donner la tuile r
    cr_assert_eq(&m.getTile(7 * 100,     0), &m.getTile(0, 0));
    cr_assert_eq(&m.getTile(7 * 100 + 3, 0), &m.getTile(3, 0));
    cr_assert_eq(&m.getTile(0, 5 * 100 + 2), &m.getTile(0, 2));
}


Test(map, getTile_wraps_negative_x) {
    game::Map m(10, 10);
    cr_assert_eq(&m.getTile(-1, 0), &m.getTile(9, 0));
    cr_assert_eq(&m.getTile(-2, 0), &m.getTile(8, 0));
    cr_assert_eq(&m.getTile(-10, 0), &m.getTile(0, 0));
}

Test(map, getTile_wraps_negative_y) {
    game::Map m(10, 10);
    cr_assert_eq(&m.getTile(0, -1), &m.getTile(0, 9));
    cr_assert_eq(&m.getTile(0, -2), &m.getTile(0, 8));
    cr_assert_eq(&m.getTile(0, -10), &m.getTile(0, 0));
}

Test(map, getTile_wraps_both_negative) {
    game::Map m(10, 10);
    cr_assert_eq(&m.getTile(-1, -1), &m.getTile(9, 9));
}

Test(map, getTile_negative_large_offset) {
    game::Map m(7, 5);
    cr_assert_eq(&m.getTile(-7, 0),  &m.getTile(0, 0));
    cr_assert_eq(&m.getTile(-8, 0),  &m.getTile(6, 0));
    cr_assert_eq(&m.getTile(0, -15), &m.getTile(0, 0));
}


Test(map, getTile_reference_is_mutable) {
    game::Map m(10, 10);
    game::Tile &t = m.getTile(2, 3);
    t.addRessource(FOOD_IDX, 5);
    cr_assert_eq(m.getTile(2, 3).getRessource(FOOD_IDX), 5);
    // le wrap doit pointer sur la même tuile modifiée
    cr_assert_eq(m.getTile(2 + 10, 3).getRessource(FOOD_IDX), 5);
    cr_assert_eq(m.getTile(2 - 10, 3).getRessource(FOOD_IDX), 5);
}

Test(map, tiles_are_independent) {
    game::Map m(5, 5);
    m.getTile(0, 0).addRessource(FOOD_IDX, 10);
    for (int x = 0; x < 5; x++)
        for (int y = 0; y < 5; y++)
            if (x != 0 || y != 0)
                cr_assert_eq(m.getTile(x, y).getRessource(FOOD_IDX), 0,
                    "tile (%d,%d) must not be affected by write to (0,0)", x, y);
}
