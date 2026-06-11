#pragma once

// Resource densities
#define FOOD_DENSITY 0.5
#define LINEMATE_DENSITY 0.3
#define DERAUMERE_DENSITY 0.15
#define SIBUR_DENSITY 0.1
#define MENDIANE_DENSITY 0.1
#define PHIRAS_DENSITY 0.08
#define THYSTAME_DENSITY 0.05

// Game rules
#define SURVIVAL_TIME 126
#define START_FOOD 10
#define START_LIFE 1260
#define MAX_LVL 8
#define WIN_COND 6
#define RESOURCE_RESPAWN 20
#define MAX_CMD_QUEUE 10

// Command time costs (/ freq = seconds)
#define TIME_FORWARD 7
#define TIME_RIGHT 7
#define TIME_LEFT 7
#define TIME_LOOK 7
#define TIME_INVENTORY 1
#define TIME_BROADCAST 7
#define TIME_EJECT 7
#define TIME_TAKE 7
#define TIME_SET 7
#define TIME_FORK 42
#define TIME_INCANTATION 300

// Resource count (i)
#define FOOD_IDX 0
#define LINEMATE_IDX 1
#define DERAUMERE_IDX 2
#define SIBUR_IDX 3
#define MENDIANE_IDX 4
#define PHIRAS_IDX 5
#define THYSTAME_IDX 6
#define RESOURCE_COUNT 7

// Orientation
#define N 1
#define E 2
#define S 3
#define W 4

#define GUI_TEAM "GRAPHIC"
