import asyncio
import random
import stats
from network import Network
from const import RESOURCES, ELEVATION_REQS, MOVES

_pending = 0


def _spawn(team, host, port, count=1, attack_mode=False):
    global _pending
    _pending += count
    for _ in range(count):
        asyncio.create_task(_run_bot(team, host, port, attack_mode))


async def _run_bot(team, host, port, attack_mode=False):
    global _pending
    bot = Bot(team, host, port, attack_mode)
    connected = False
    try:
        net = Network(host, port)
        bot.net = net
        await net.connect()

        await asyncio.wait_for(net.responses.get(), timeout=10)
        await net.send(team)
        resp = await asyncio.wait_for(net.responses.get(), timeout=10)

        if resp == "ko" or resp == "dead":
            return

        await asyncio.wait_for(net.responses.get(), timeout=10)

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
            stats.rm_bot(bot.level)
        try:
            if bot.net and bot.net.writer and not bot.net.writer.is_closing():
                bot.net.writer.close()
        except Exception:
            pass


class Bot:
    def __init__(self, team, host, port, attack_mode=False):
        self.team = team
        self.host = host
        self.port = port
        self.attack_mode = attack_mode
        self.net = None
        self.level = 1
        self.inventory_items = {item: 0 for item in RESOURCES}
        self.state = 0
        self.id = str(random.randint(0, 999999))
        self.key = sum(ord(c) for c in team) % 999
        self.leader_dir = -1
        self.leader_id = None
        self.forked = False
        self.wait_ticks = 0

    def log(self, msg, is_lvl=False):
        message = f"[Bot {self.id} | Lvl {self.level}] {msg}"
        if stats.DEBUG:
            print(message, flush=True)
        if is_lvl:
            stats.log_evt(message)

    async def cmd(self, command):
        if not self.net.writer or self.net.writer.is_closing():
            return "dead"
        await self.net.send(command)
        while True:
            resp = await self.net.responses.get()
            if resp == "dead":
                return "dead"
            if resp == "Elevation underway":
                if command == "Incantation":
                    return resp
                continue
            if resp.startswith("Current level: "):
                new_level = int(resp[15:])
                if new_level > self.level:
                    old_level = self.level
                    stats.lvl_up(old_level, new_level)
                    self.level = new_level
                    self.log(f"Level up! Now level {self.level}", True)
                self.state = 0
                for item in RESOURCES[1:]:
                    self.inventory_items[item] = 0
                continue
            return resp

    async def look(self):
        resp = await self.cmd("Look")
        if resp == "dead":
            return None
        resp = resp.strip("[]").split(",")
        return [tile.strip().split() for tile in resp]

    async def inventory(self):
        resp = await self.cmd("Inventory")
        if resp == "dead":
            return
        resp = resp.strip("[]").split(",")
        for entry in resp:
            parts = entry.strip().split()
            if len(parts) == 2 and parts[0] in self.inventory_items:
                try:
                    self.inventory_items[parts[0]] = int(parts[1])
                except ValueError:
                    pass

    def has_stones(self):
        if self.level >= 8:
            return False
        req = ELEVATION_REQS[self.level - 1]
        for idx, item in enumerate(RESOURCES[1:]):
            if self.inventory_items.get(item, 0) < req[idx + 1]:
                return False
        return True

    async def handle_events(self):
        while not self.net.events.empty():
            event, data = await self.net.events.get()
            if event == "j":
                if self.state in (1, 2, 3):
                    self.state = 0
                    self.leader_dir = -1
                    self.leader_id = None
                    self.log("Ejected")
            elif event == "m":
                try:
                    dir_str, text = data.split(",", 1)
                    direction = int(dir_str.strip())
                    text = text.strip()
                    if text.startswith(f"{self.key}_R_"):
                        parts = text.split("_")
                        if len(parts) == 4 and parts[1] == "R":
                            msg_level = int(parts[2])
                            leader_id = parts[3]
                            if msg_level == self.level:
                                if self.state == 0 and self.inventory_items.get("food", 0) > 10:
                                    self.leader_dir = direction
                                    self.leader_id = leader_id
                                    self.state = 2
                                    self.wait_ticks = 0
                                    self.log(f"Following leader {leader_id} lvl {self.level} dir {direction}")
                                elif self.state == 2 and self.leader_id == leader_id:
                                    self.leader_dir = direction
                                elif self.state == 1 and leader_id < self.id:
                                    self.state = 2
                                    self.leader_id = leader_id
                                    self.leader_dir = direction
                                    self.wait_ticks = 0
                                    self.log(f"Yielding to better leader {leader_id}")
                except Exception:
                    pass

    async def life(self):
        while True:
            if not self.net.writer or self.net.writer.is_closing():
                break
            await self.handle_events()
            await self.inventory()
            if not self.net.writer or self.net.writer.is_closing():
                break
            if self.inventory_items.get("food", 0) <= 0:
                break
            if self.state == 0:
                await self.collect()
            elif self.state == 1:
                await self.lead()
            elif self.state == 2:
                await self.follow()
            elif self.state == 3:
                await self.wait_incantation()

    async def collect(self):
        global _pending

        if not self.forked and self.inventory_items.get("food", 0) > 20:
            total = stats.STATS["total"] + _pending
            if total < 35:
                free_slots_str = await self.cmd("Connect_nbr")
                if free_slots_str != "dead" and free_slots_str.isdigit():
                    free_slots = int(free_slots_str)
                    available = free_slots - _pending
                    if available > 0:
                        needed = min(available, 35 - stats.STATS["total"] - _pending)
                        if needed > 0:
                            self.log(f"Forking to spawn {needed} bot(s)")
                            resp = await self.cmd("Fork")
                            if resp == "ok":
                                self.forked = True
                                _spawn(self.team, self.host, self.port, 1, self.attack_mode)

        if self.has_stones() and self.inventory_items.get("food", 0) > 25:
            self.log(f"Have stones for level {self.level + 1}, becoming leader")
            self.state = 1
            return

        view = await self.look()
        if view is None:
            return
        cell = view[0]

        if "food" in cell:
            await self.cmd("Take food")
            return

        if self.level < 8:
            req = ELEVATION_REQS[self.level - 1]
            for idx, item in enumerate(RESOURCES[1:]):
                if self.inventory_items.get(item, 0) < req[idx + 1] and item in cell:
                    await self.cmd(f"Take {item}")
                    return

        if self.attack_mode and cell.count("player") > 1:
            if not any(s in cell for s in RESOURCES[1:]):
                roll_eject = random.random()
                if roll_eject < 0.20:
                    self.log("Attack mode: Ejecting players on my tile")
                    await self.cmd("Eject")
                    return

        roll = random.random()
        if roll < 0.15:
            await self.cmd("Left")
        elif roll < 0.30:
            await self.cmd("Right")
        await self.cmd("Forward")

    async def lead(self):
        food = self.inventory_items.get("food", 0)
        if food < 6:
            self.log("Low food, back to collect")
            self.state = 0
            return

        req = ELEVATION_REQS[self.level - 1]
        needed_players = req[0]

        await self.cmd(f"Broadcast {self.key}_R_{self.level}_{self.id}")

        view = await self.look()
        if view is None:
            return
        cell = view[0]

        if "food" in cell:
            await self.cmd("Take food")
            return

        player_count = cell.count("player")

        if player_count >= needed_players:
            self.log(f"Starting incantation ({player_count}/{needed_players} players)")

            for item in RESOURCES[1:]:
                for _ in range(cell.count(item)):
                    resp = await self.cmd(f"Take {item}")
                    if resp == "dead":
                        return

            for idx, item in enumerate(RESOURCES[1:]):
                for _ in range(req[idx + 1]):
                    resp = await self.cmd(f"Set {item}")
                    if resp == "dead":
                        return

            resp = await self.cmd("Incantation")
            if resp == "Elevation underway":
                result = await self.net.responses.get()
                if result != "dead" and result.startswith("Current level: "):
                    new_level = int(result[15:])
                    if new_level > self.level:
                        old_level = self.level
                        stats.lvl_up(old_level, new_level)
                        self.level = new_level
                        self.log(f"Level up! Now level {self.level}", True)
                    for item in RESOURCES[1:]:
                        self.inventory_items[item] = 0
            elif self.attack_mode:
                self.log("Incantation failed! Ejecting bad player")
                await self.cmd("Eject")

            self.state = 0
            self.forked = False

    async def follow(self):
        food = self.inventory_items.get("food", 0)
        if food < 5:
            self.log("Low food, back to collect")
            self.state = 0
            self.leader_dir = -1
            self.leader_id = None
            return

        if self.leader_dir == 0:
            self.log("On leader tile, waiting for incantation")
            self.state = 3
            self.wait_ticks = 0
            return

        if self.leader_dir == -1:
            self.wait_ticks += 1
            if self.wait_ticks > 8:
                self.state = 0
                self.leader_id = None
            return

        direction = self.leader_dir
        self.leader_dir = -1
        self.wait_ticks = 0

        if direction in MOVES:
            for move in MOVES[direction]:
                resp = await self.cmd(move)
                if resp == "dead":
                    return

    async def wait_incantation(self):
        self.wait_ticks += 1
        food = self.inventory_items.get("food", 0)

        view = await self.look()
        if view is not None and "food" in view[0]:
            await self.cmd("Take food")

        if self.wait_ticks > 150 or food < 3:
            self.log("Incantation wait timeout, back to collect")
            self.state = 0
            self.leader_id = None
            self.leader_dir = -1
