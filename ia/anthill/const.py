import sys

maxInt = sys.maxsize

#
FOOD = 0
LIMEMATE = 1
DERAUMERE = 2
SIBUR = 3
MENDIANE = 4
PHIRAS = 5
THYSTAME = 6


# only Queen states
Spawn = 0
Hungry = 2
Call = 3

#only Workers/Guards states
Feed = 4
Join = 5

#all States
Collect = 1

steps = [
    [1, 1, 0, 0, 0, 0, 0],
    [2, 1, 1, 1, 0, 0, 0],
    [2, 2, 0, 1, 0, 2, 0],
    [4, 1, 1, 2, 0, 1, 0],
    [4, 1, 2, 1, 3, 0, 0],
    [6, 1, 2, 3, 0, 1, 0],
    [6, 2, 2, 2, 2, 2, 1],
]

nearest = [[[0, 1]],         #0
   [[2, 2], [1, 4], [3, 4]], #1
   [[6, 3], [5, 5], [7, 5]], #2
          [[12, 4]],         #3
          [[20, 5]],         #4
    ]
