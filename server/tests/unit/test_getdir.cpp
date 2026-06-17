#include <criterion/criterion.h>

#include "Game/Common.hpp"
#include "Game/GameLogic.hpp"
#include "Game/Player.hpp"

// Per the subject's "Sound transmission" diagram: octants are numbered
// counter-clockwise starting from the tile directly in front of the
// listening player (1), then front-left (2), left (3), back-left (4),
// back (5), back-right (6), right (7), front-right (8).
// 0 is reserved for "same tile as the emitter".

static void check(game::GameLogic &g, game::Player &listener, int orientation,
                  int dx, int dy, int expected) {
  listener.setOrientation(orientation);
  game::Player emitter(99, "team1");
  emitter.setPos(listener.getX() + dx, listener.getY() + dy);

  int dir = g.getDir(listener, emitter, g.getMapX(), g.getMapY());
  cr_assert_eq(dir, expected,
              "orientation=%d dx=%d dy=%d: expected %d, got %d", orientation, dx,
              dy, expected, dir);
}

Test(getdir, same_tile_is_zero) {
  game::GameLogic g(20, 20, 1, 1, {"team1"});
  game::Player listener(1, "team1");
  listener.setPos(10, 10);
  game::Player emitter(2, "team1");
  emitter.setPos(10, 10);

  int dir = g.getDir(listener, emitter, g.getMapX(), g.getMapY());
  cr_assert_eq(dir, 0);
}

Test(getdir, facing_north_octants) {
  game::GameLogic g(20, 20, 1, 1, {"team1"});
  game::Player listener(1, "team1");
  listener.setPos(10, 10);

  // Facing N: front = -Y, left = -X, right = +X.
  check(g, listener, N, 0, -1, 1);  // front
  check(g, listener, N, -1, -1, 2); // front-left
  check(g, listener, N, -1, 0, 3);  // left
  check(g, listener, N, -1, 1, 4);  // back-left
  check(g, listener, N, 0, 1, 5);   // back
  check(g, listener, N, 1, 1, 6);   // back-right
  check(g, listener, N, 1, 0, 7);   // right
  check(g, listener, N, 1, -1, 8);  // front-right
}

Test(getdir, facing_east_octants) {
  game::GameLogic g(20, 20, 1, 1, {"team1"});
  game::Player listener(1, "team1");
  listener.setPos(10, 10);

  // Facing E: front = +X, left = -Y, right = +Y.
  check(g, listener, E, 1, 0, 1);
  check(g, listener, E, 1, -1, 2);
  check(g, listener, E, 0, -1, 3);
  check(g, listener, E, -1, -1, 4);
  check(g, listener, E, -1, 0, 5);
  check(g, listener, E, -1, 1, 6);
  check(g, listener, E, 0, 1, 7);
  check(g, listener, E, 1, 1, 8);
}

Test(getdir, facing_south_octants) {
  game::GameLogic g(20, 20, 1, 1, {"team1"});
  game::Player listener(1, "team1");
  listener.setPos(10, 10);

  // Facing S: front = +Y, left = +X, right = -X.
  check(g, listener, S, 0, 1, 1);
  check(g, listener, S, 1, 1, 2);
  check(g, listener, S, 1, 0, 3);
  check(g, listener, S, 1, -1, 4);
  check(g, listener, S, 0, -1, 5);
  check(g, listener, S, -1, -1, 6);
  check(g, listener, S, -1, 0, 7);
  check(g, listener, S, -1, 1, 8);
}

Test(getdir, facing_west_octants) {
  game::GameLogic g(20, 20, 1, 1, {"team1"});
  game::Player listener(1, "team1");
  listener.setPos(10, 10);

  // Facing W: front = -X, left = +Y, right = -Y.
  check(g, listener, W, -1, 0, 1);
  check(g, listener, W, -1, 1, 2);
  check(g, listener, W, 0, 1, 3);
  check(g, listener, W, 1, 1, 4);
  check(g, listener, W, 1, 0, 5);
  check(g, listener, W, 1, -1, 6);
  check(g, listener, W, 0, -1, 7);
  check(g, listener, W, -1, -1, 8);
}

// The map wraps around (toroidal world): the shortest path must be chosen,
// so a far-away emitter on a small map is seen as if it were adjacent on
// the other side.
Test(getdir, wraps_around_picks_shortest_path) {
  game::GameLogic g(10, 10, 1, 1, {"team1"});
  game::Player listener(1, "team1");
  listener.setPos(0, 0);
  listener.setOrientation(N);

  game::Player emitter(2, "team1");
  emitter.setPos(0, 9); // one tile north of (0,0) once wrapped
  int dir = g.getDir(listener, emitter, g.getMapX(), g.getMapY());
  cr_assert_eq(dir, 1, "emitter wrapped one tile north must be 'front' (1)");
}

Test(getdir, every_orientation_every_octant_is_in_range) {
  game::GameLogic g(20, 20, 1, 1, {"team1"});
  game::Player listener(1, "team1");
  listener.setPos(10, 10);

  int orientations[] = {N, E, S, W};
  for (int o : orientations) {
    listener.setOrientation(o);
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        if (dx == 0 && dy == 0)
          continue;
        game::Player emitter(2, "team1");
        emitter.setPos(10 + dx, 10 + dy);
        int dir = g.getDir(listener, emitter, g.getMapX(), g.getMapY());
        cr_assert(dir >= 1 && dir <= 8,
                  "orientation=%d dx=%d dy=%d produced out-of-range dir %d", o,
                  dx, dy, dir);
      }
    }
  }
}
