"""Conformity of the AI's static tables with the Zappy subject (G-YEP-400)."""

import const


def test_resources_count_is_seven():
    assert len(const.RESOURCES) == 7


def test_food_is_first_resource():
    assert const.RESOURCES[0] == "food"


def test_resource_names_and_order():
    assert const.RESOURCES == [
        "food", "linemate", "deraumere", "sibur",
        "mendiane", "phiras", "thystame",
    ]


def test_resource_names_are_unique():
    assert len(set(const.RESOURCES)) == len(const.RESOURCES)


EXPECTED_ELEVATION = [
    [1, 1, 0, 0, 0, 0, 0],   # 1 -> 2
    [2, 1, 1, 1, 0, 0, 0],   # 2 -> 3
    [2, 2, 0, 1, 0, 2, 0],   # 3 -> 4
    [4, 1, 1, 2, 0, 1, 0],   # 4 -> 5
    [4, 1, 2, 1, 3, 0, 0],   # 5 -> 6
    [6, 1, 2, 3, 0, 1, 0],   # 6 -> 7
    [6, 2, 2, 2, 2, 2, 1],   # 7 -> 8
]


def test_elevation_table_has_seven_levels():
    assert len(const.ELEVATION_REQS) == 7


def test_elevation_table_matches_subject_exactly():
    assert const.ELEVATION_REQS == EXPECTED_ELEVATION


def test_each_elevation_row_has_seven_entries():
    for row in const.ELEVATION_REQS:
        assert len(row) == 7


def test_player_counts_follow_subject_progression():
    counts = [row[0] for row in const.ELEVATION_REQS]
    assert counts == [1, 2, 2, 4, 4, 6, 6]


def test_only_last_elevation_needs_thystame():
    for idx, row in enumerate(const.ELEVATION_REQS):
        thystame = row[6]
        if idx == 6:
            assert thystame == 1
        else:
            assert thystame == 0


def test_first_elevation_needs_only_one_linemate():
    assert const.ELEVATION_REQS[0] == [1, 1, 0, 0, 0, 0, 0]


def test_moves_cover_all_eight_directions():
    assert sorted(const.MOVES.keys()) == [1, 2, 3, 4, 5, 6, 7, 8]


def test_moves_only_use_valid_protocol_commands():
    allowed = {"Forward", "Left", "Right"}
    for seq in const.MOVES.values():
        assert seq, "movement sequence must not be empty"
        assert set(seq) <= allowed


def test_move_straight_ahead_is_a_single_forward():
    assert const.MOVES[1] == ["Forward"]


def test_every_move_sequence_ends_by_stepping_forward():
    for direction, seq in const.MOVES.items():
        assert seq[-1] == "Forward", f"dir {direction} does not advance"
