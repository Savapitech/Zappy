"""Connect_nbr reports the number of free slots for the player's team."""

from zappy_client import AIClient


def test_connect_nbr_returns_a_number(server):
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    ai.send("Connect_nbr")
    line = ai.read_line()
    assert line is not None and line.lstrip("-").isdigit()
    ai.close()


def test_connect_nbr_reflects_remaining_slots(make_server):
    server = make_server(clients=3)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    ai.send("Connect_nbr")
    free = int(ai.read_line())
    # One client connected out of three slots leaves two free.
    assert free == 2
    ai.close()


def test_connect_nbr_drops_as_clients_join(make_server):
    server = make_server(clients=4)
    a = AIClient(server.host, server.port, "team1")
    a.handshake()
    a.send("Connect_nbr")
    first = int(a.read_line())

    b = AIClient(server.host, server.port, "team1")
    b.handshake()

    a.send("Connect_nbr")
    second = int(a.read_line())
    assert second == first - 1
    a.close()
    b.close()


def test_fork_grows_the_slot_pool(make_server):
    server = make_server(clients=1)
    ai = AIClient(server.host, server.port, "team1")
    ai.handshake()
    ai.send("Connect_nbr")
    before = int(ai.read_line())

    ai.send("Fork")
    assert ai.read_line() == "ok"

    ai.send("Connect_nbr")
    after = int(ai.read_line())
    # Forking lays an egg, which authorizes one more connection.
    assert after >= before + 1
    ai.close()
