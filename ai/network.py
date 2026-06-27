import asyncio
import sys


class Network:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.reader = None
        self.writer = None
        self.responses = asyncio.Queue()
        self.events = asyncio.Queue()

    async def connect(self):
        self.reader, self.writer = await asyncio.open_connection(self.host, self.port)
        asyncio.create_task(self.listen())

    async def listen(self):
        try:
            while True:
                data = await self.reader.readline()
                if not data:
                    break
                line = data.decode().strip()
                if line == "dead":
                    break
                if line.startswith("message "):
                    await self.events.put(("m", line[8:]))
                elif line.startswith("eject: "):
                    await self.events.put(("j", line[7:]))
                else:
                    await self.responses.put(line)
        except Exception:
            pass
        if self.writer:
            self.writer.close()
        await self.responses.put("dead")

    async def send(self, command):
        self.writer.write((command + "\n").encode())
        await self.writer.drain()
