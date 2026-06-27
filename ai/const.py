RESOURCES = ["food", "linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]
ELEVATION_REQS = [
    [1, 1, 0, 0, 0, 0, 0],
    [2, 1, 1, 1, 0, 0, 0],
    [2, 2, 0, 1, 0, 2, 0],
    [4, 1, 1, 2, 0, 1, 0],
    [4, 1, 2, 1, 3, 0, 0],
    [6, 1, 2, 3, 0, 1, 0],
    [6, 2, 2, 2, 2, 2, 1]
]
MOVES = {
    1: ["Forward"],
    2: ["Forward", "Left", "Forward"],
    3: ["Left", "Forward"],
    4: ["Left", "Left", "Forward"],
    5: ["Left", "Left", "Forward"],
    6: ["Right", "Right", "Forward"],
    7: ["Right", "Forward"],
    8: ["Forward", "Right", "Forward"]
}
