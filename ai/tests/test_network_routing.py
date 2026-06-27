"""Network.listen routes server lines into the right queue."""

import asyncio

from network import Network
from fakes import FakeReader, FakeWriter


def _drain(queue):
    items = []
    while not queue.empty():
        items.append(queue.get_nowait())
    return items


def _run_listen(lines):
    net = Network("127.0.0.1", 4242)
    net.reader = FakeReader(lines)
    net.writer = FakeWriter()
    asyncio.run(net.listen())
    return net


def test_plain_responses_go_to_the_response_queue():
    net = _run_listen(["ok", "ko", "[player]"])
    responses = _drain(net.responses)
    assert "ok" in responses
    assert "ko" in responses
    assert "[player]" in responses


def test_broadcast_is_routed_as_message_event():
    net = _run_listen(["message 5, hello world"])
    events = _drain(net.events)
    assert ("m", "5, hello world") in events


def test_eject_is_routed_as_eject_event():
    net = _run_listen(["eject: 3"])
    events = _drain(net.events)
    assert ("j", "3") in events


def test_dead_stops_the_listener_and_signals_response_queue():
    net = _run_listen(["ok", "dead", "ignored after dead"])
    responses = _drain(net.responses)
    assert "ok" in responses
    assert "dead" in responses
    assert "ignored after dead" not in responses


def test_eof_closes_and_signals_dead():
    net = _run_listen([])
    responses = _drain(net.responses)
    assert responses == ["dead"]
    assert net.writer.is_closing()


def test_messages_and_responses_do_not_cross_queues():
    net = _run_listen(["ok", "message 1, hi", "eject: 2", "ko"])
    responses = _drain(net.responses)
    events = _drain(net.events)
    assert responses[:2] == ["ok", "ko"]  # "dead" appended at EOF after these
    assert ("m", "1, hi") in events
    assert ("j", "2") in events
    assert all(not r.startswith("message") for r in responses)
    assert all(not r.startswith("eject") for r in responses)
