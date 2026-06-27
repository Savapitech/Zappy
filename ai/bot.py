import asyncio
import random
import stats
from network import Network
from const import R, S, D

_pending = 0


def _spawn(t, h, p, n=1):
    global _pending
    _pending += n
    for _ in range(n):
        asyncio.create_task(_run_bot(t, h, p))


async def _run_bot(t, h, p):
    global _pending
    bot = Bot(t, h, p)
    connected = False
    try:
        net = Network(h, p)
        bot.n = net
        await net.c()

        await asyncio.wait_for(net.q.get(), timeout=10)
        await net.s(t)
        resp = await asyncio.wait_for(net.q.get(), timeout=10)

        if resp == "ko" or resp == "dead":
            return

        await asyncio.wait_for(net.q.get(), timeout=10)

        _pending -= 1
        connected = True
        stats.add_bot()
        bot.log("Connected")

        await bot.life()

    except asyncio.TimeoutError:
        pass
    except Exception:
        pass
    finally:
        if not connected and _pending > 0:
            _pending -= 1
        if connected:
            stats.rm_bot(bot.l)
        try:
            if bot.n and bot.n.w and not bot.n.w.is_closing():
                bot.n.w.close()
        except Exception:
            pass


class Bot:
    def __init__(self, t, h, p):
        self.t = t
        self.h = h
        self.p = p
        self.n = None
        self.l = 1
        self.i = {k: 0 for k in R}
        self.s = 0
        self.id = str(random.randint(0, 999999))
        self.k = sum(ord(c) for c in t) % 999
        self.ld = -1
        self.lid = None
        self.forked = False
        self.wt = 0

    def log(self, msg, is_lvl=False):
        m = f"[Bot {self.id} | Lvl {self.l}] {msg}"
        if stats.D:
            print(m, flush=True)
        if is_lvl:
            stats.log_evt(m)

    async def cmd(self, c):
        if not self.n.w or self.n.w.is_closing():
            return "dead"
        await self.n.s(c)
        while True:
            r = await self.n.q.get()
            if r == "dead":
                return "dead"
            if r == "Elevation underway":
                if c == "Incantation":
                    return r
                continue
            if r.startswith("Current level: "):
                nl = int(r[15:])
                if nl > self.l:
                    old = self.l
                    stats.lvl_up(old, nl)
                    self.l = nl
                    self.log(f"Level up! Now level {self.l}", True)
                self.s = 0
                for k in R[1:]:
                    self.i[k] = 0
                continue
            return r

    async def look(self):
        r = await self.cmd("Look")
        if r == "dead":
            return None
        r = r.strip("[]").split(",")
        return [x.strip().split() for x in r]

    async def inventory(self):
        r = await self.cmd("Inventory")
        if r == "dead":
            return
        r = r.strip("[]").split(",")
        for x in r:
            p = x.strip().split()
            if len(p) == 2 and p[0] in self.i:
                try:
                    self.i[p[0]] = int(p[1])
                except ValueError:
                    pass

    def has_stones(self):
        if self.l >= 8:
            return False
        req = S[self.l - 1]
        for idx, item in enumerate(R[1:]):
            if self.i.get(item, 0) < req[idx + 1]:
                return False
        return True

    async def handle_events(self):
        while not self.n.e.empty():
            ev, data = await self.n.e.get()
            if ev == "j":
                if self.s in (1, 2, 3):
                    self.s = 0
                    self.ld = -1
                    self.lid = None
                    self.log("Ejected")
            elif ev == "m":
                try:
                    ds, txt = data.split(",", 1)
                    di = int(ds.strip())
                    txt = txt.strip()
                    if txt.startswith(f"{self.k}_R_"):
                        parts = txt.split("_")
                        if len(parts) == 4 and parts[1] == "R":
                            msg_level = int(parts[2])
                            lid = parts[3]
                            if msg_level == self.l:
                                if self.s == 0 and self.i.get("food", 0) > 10:
                                    self.ld = di
                                    self.lid = lid
                                    self.s = 2
                                    self.wt = 0
                                    self.log(f"Following leader {lid} lvl {self.l} dir {di}")
                                elif self.s == 2 and self.lid == lid:
                                    self.ld = di
                                elif self.s == 1 and lid < self.id:
                                    self.s = 2
                                    self.lid = lid
                                    self.ld = di
                                    self.wt = 0
                                    self.log(f"Yielding to better leader {lid}")
                except Exception:
                    pass

    async def life(self):
        while True:
            if not self.n.w or self.n.w.is_closing():
                break
            await self.handle_events()
            await self.inventory()
            if not self.n.w or self.n.w.is_closing():
                break
            if self.i.get("food", 0) <= 0:
                break
            if self.s == 0:
                await self.collect()
            elif self.s == 1:
                await self.lead()
            elif self.s == 2:
                await self.follow()
            elif self.s == 3:
                await self.wait_incantation()

    async def collect(self):
        global _pending

        if not self.forked and self.i.get("food", 0) > 20:
            total = stats.S["t"] + _pending
            if total < 35:
                cn_str = await self.cmd("Connect_nbr")
                if cn_str != "dead" and cn_str.isdigit():
                    cn = int(cn_str)
                    available = cn - _pending
                    if available > 0:
                        needed = min(available, 35 - stats.S["t"] - _pending)
                        if needed > 0:
                            self.log(f"Forking to spawn {needed} bot(s)")
                            r = await self.cmd("Fork")
                            if r == "ok":
                                self.forked = True
                                _spawn(self.t, self.h, self.p, 1)

        if self.has_stones() and self.i.get("food", 0) > 25:
            self.log(f"Have stones for level {self.l + 1}, becoming leader")
            self.s = 1
            return

        v = await self.look()
        if v is None:
            return
        cell = v[0]

        if "food" in cell:
            await self.cmd("Take food")
            return

        if self.l < 8:
            req = S[self.l - 1]
            for idx, item in enumerate(R[1:]):
                if self.i.get(item, 0) < req[idx + 1] and item in cell:
                    await self.cmd(f"Take {item}")
                    return

        r = random.random()
        if r < 0.15:
            await self.cmd("Left")
        elif r < 0.30:
            await self.cmd("Right")
        await self.cmd("Forward")

    async def lead(self):
        food = self.i.get("food", 0)
        if food < 6:
            self.log("Low food, back to collect")
            self.s = 0
            return

        req = S[self.l - 1]
        needed_players = req[0]

        await self.cmd(f"Broadcast {self.k}_R_{self.l}_{self.id}")

        v = await self.look()
        if v is None:
            return
        cell = v[0]

        if "food" in cell:
            await self.cmd("Take food")
            return

        player_count = cell.count("player")

        if player_count >= needed_players:
            self.log(f"Starting incantation ({player_count}/{needed_players} players)")

            for item in R[1:]:
                for _ in range(cell.count(item)):
                    r = await self.cmd(f"Take {item}")
                    if r == "dead":
                        return

            for idx, item in enumerate(R[1:]):
                for _ in range(req[idx + 1]):
                    r = await self.cmd(f"Set {item}")
                    if r == "dead":
                        return

            r = await self.cmd("Incantation")
            if r == "Elevation underway":
                res = await self.n.q.get()
                if res != "dead" and res.startswith("Current level: "):
                    nl = int(res[15:])
                    if nl > self.l:
                        old = self.l
                        stats.lvl_up(old, nl)
                        self.l = nl
                        self.log(f"Level up! Now level {self.l}", True)
                    for k in R[1:]:
                        self.i[k] = 0
            self.s = 0
            self.forked = False

    async def follow(self):
        food = self.i.get("food", 0)
        if food < 5:
            self.log("Low food, back to collect")
            self.s = 0
            self.ld = -1
            self.lid = None
            return

        if self.ld == 0:
            self.log("On leader tile, waiting for incantation")
            self.s = 3
            self.wt = 0
            return

        if self.ld == -1:
            self.wt += 1
            if self.wt > 8:
                self.s = 0
                self.lid = None
            return

        direction = self.ld
        self.ld = -1
        self.wt = 0

        if direction in D:
            for mv in D[direction]:
                r = await self.cmd(mv)
                if r == "dead":
                    return

    async def wait_incantation(self):
        self.wt += 1
        food = self.i.get("food", 0)

        v = await self.look()
        if v is not None and "food" in v[0]:
            await self.cmd("Take food")

        if self.wt > 150 or food < 3:
            self.log("Incantation wait timeout, back to collect")
            self.s = 0
            self.lid = None
            self.ld = -1
