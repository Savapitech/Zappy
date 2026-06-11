import sys

max_int = sys.maxsize

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
