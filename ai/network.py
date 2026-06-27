import asyncio
import sys


class Network:
    def __init__(self, h, p):
        self.h = h
        self.p = p
        self.r = None
        self.w = None
        self.q = asyncio.Queue()
        self.e = asyncio.Queue()

    async def c(self):
        self.r, self.w = await asyncio.open_connection(self.h, self.p)
        asyncio.create_task(self.l())

    async def l(self):
        try:
            while True:
                d = await self.r.readline()
                if not d:
                    break
                x = d.decode().strip()
                if x == "dead":
                    break
                if x.startswith("message "):
                    await self.e.put(("m", x[8:]))
                elif x.startswith("eject: "):
                    await self.e.put(("j", x[7:]))
                else:
                    await self.q.put(x)
        except Exception:
            pass
        if self.w:
            self.w.close()
        await self.q.put("dead")

    async def s(self, c):
        self.w.write((c + "\n").encode())
        await self.w.drain()
