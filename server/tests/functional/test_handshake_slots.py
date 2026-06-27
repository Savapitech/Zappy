"""Connection handshake and team-slot accounting (subject section 4 & 2)."""

import pytest

from zappy_client import AIClient, ZappyClient


def test_welcome_is_sent_first(server):
    raw = ZappyClient(server.host, server.port)
    assert raw.read_line() == "WELCOME"
    raw.close()


@pytest.mark.parametrize("clients", [1, 2, 5, 10])
def test_initial_slots_are_remaining_after_join(make_server, clients):
    # CLIENT-NUM is the number of slots still free once this client has taken
    # one, i.e. clients - 1 for the very first connection.
    server = make_server(clients=clients)
    ai = AIClient(server.host, server.port, "team1")
    slots, _ = ai.handshake()
    assert slots == clients - 1
    ai.close()


@pytest.mark.parametrize("size", [(10, 10), (20, 30), (42, 7), (5, 5)])
def test_handshake_echoes_map_size(make_server, size):
    x, y = size
    server = make_server(x=x, y=y)
    ai = AIClient(server.host, server.port, "team1")
    _, (rx, ry) = ai.handshake()
    assert (rx, ry) == (x, y)
    ai.close()


def test_each_join_consumes_one_slot(make_server):
    server = make_server(clients=3)
    a = AIClient(server.host, server.port, "team1")
    slots_a, _ = a.handshake()
    b = AIClient(server.host, server.port, "team1")
    slots_b, _ = b.handshake()
    assert slots_a == 2  # 3 slots, 1 taken
    assert slots_b == 1  # 3 slots, 2 taken
    a.close()
    b.close()


def test_joining_an_unknown_team_is_rejected(server):
    ai = ZappyClient(server.host, server.port)
    ai.expect("WELCOME")
    ai.send("no_such_team")
    # The server must not grant a normal handshake to an unknown team.
    line = ai.read_line()
    assert line == "ko" or line is None or not line.isdigit()
    ai.close()


def test_full_team_refuses_extra_clients(make_server):
    server = make_server(clients=1)
    a = AIClient(server.host, server.port, "team1")
    a.handshake()
    # Second client on a 1-slot team: must be refused with ko.
    b = ZappyClient(server.host, server.port)
    b.expect("WELCOME")
    b.send("team1")
    assert b.read_line() == "ko"
    a.close()
    b.close()
