"""Lightweight async fakes so the bot's coroutines can be unit tested without
opening a real socket."""

import asyncio


class FakeWriter:
    def __init__(self):
        self.closed = False

    def is_closing(self):
        return self.closed

    def close(self):
        self.closed = True


class FakeNet:
    """Stands in for network.Network: same attributes the Player relies on."""

    def __init__(self):
        self.responses = asyncio.Queue()
        self.events = asyncio.Queue()
        self.writer = FakeWriter()
        self.sent = []

    async def send(self, command):
        self.sent.append(command)

    def feed(self, *lines):
        for line in lines:
            self.responses.put_nowait(line)

    def feed_event(self, kind, data):
        self.events.put_nowait((kind, data))


class FakeReader:
    """Stands in for an asyncio StreamReader feeding pre-baked lines."""

    def __init__(self, lines):
        self._chunks = [(line + "\n").encode() for line in lines]

    async def readline(self):
        if self._chunks:
            return self._chunks.pop(0)
        return b""


def make_bot(team="team1"):
    from bot import Bot

    bot = Bot(team, "127.0.0.1", 4242)
    bot.attach_network(FakeNet())
    return bot


def make_player():
    from player import Player

    return Player(FakeNet())


def run(coro):
    return asyncio.run(coro)
