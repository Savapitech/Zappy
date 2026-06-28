import asyncio
import random
import time
import secure
import stats
from network import Network
from player import Player
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
        bot.attach_network(net)
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
            stats.rm_bot(bot.player.level)
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
        self.player = None
        self.state = 0
        self.id = str(random.randint(0, 999999))
        self.key = secure.derive_key(team)
        self.verifier = secure.Verifier(self.key)
        self._bcast_counter = int(time.time() * 1000000)
        self.leader_dir = -1
        self.leader_id = None
        self.forked = False
        self.wait_ticks = 0

    def attach_network(self, net):
        self.net = net
        self.player = Player(net)
        self.player.on_level_up = self._on_level_up

    def _next_counter(self):
        self._bcast_counter += 1
        return self._bcast_counter

    def _on_level_up(self, old_level, new_level):
        self.state = 0
        if new_level > old_level:
            stats.lvl_up(old_level, new_level)
            self.log(f"Level up! Now level {new_level}", True)

    def log(self, msg, is_lvl=False):
        level = self.player.level if self.player else 1
        message = f"[Bot {self.id} | Lvl {level}] {msg}"
        if stats.DEBUG:
            print(message, flush=True)
        if is_lvl:
            stats.log_evt(message)

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
                    opened = self.verifier.open(text.strip())
                    if opened is not None:
                        leader_id, payload = opened
                        parts = payload.split("_")
                        if len(parts) == 2 and parts[0] == "R":
                            msg_level = int(parts[1])
                            if msg_level == self.player.level:
                                if self.state == 0 and self.player.food > 10:
                                    self.leader_dir = direction
                                    self.leader_id = leader_id
                                    self.state = 2
                                    self.wait_ticks = 0
                                    self.log(f"Following leader {leader_id} lvl {self.player.level} dir {direction}")
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
            if not self.player.is_connected():
                break
            await self.handle_events()
            await self.player.inventory()
            if not self.player.is_connected():
                break
            if self.player.food <= 0:
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

        if not self.forked and self.player.food > 20:
            total = stats.STATS["total"] + _pending
            if total < 35:
                free_slots = await self.player.connect_nbr()
                if free_slots is not None:
                    available = free_slots - _pending
                    if available > 0:
                        needed = min(available, 35 - stats.STATS["total"] - _pending)
                        if needed > 0:
                            self.log(f"Forking to spawn {needed} bot(s)")
                            if await self.player.fork() == "ok":
                                self.forked = True
                                _spawn(self.team, self.host, self.port, 1, self.attack_mode)

        if self.player.has_stones() and self.player.food > 25:
            self.log(f"Have stones for level {self.player.level + 1}, becoming leader")
            self.state = 1
            return

        view = await self.player.look()
        if view is None:
            return
        cell = view[0]

        if "food" in cell:
            await self.player.take("food")
            return

        if self.player.level < 8:
            req = ELEVATION_REQS[self.player.level - 1]
            for idx, item in enumerate(RESOURCES[1:]):
                if self.player.inventory_items.get(item, 0) < req[idx + 1] and item in cell:
                    await self.player.take(item)
                    return

        if self.attack_mode and cell.count("player") > 1:
            if not any(s in cell for s in RESOURCES[1:]):
                if random.random() < 0.20:
                    self.log("Attack mode: Ejecting players on my tile")
                    await self.player.eject()
                    return

        roll = random.random()
        if roll < 0.15:
            await self.player.left()
        elif roll < 0.30:
            await self.player.right()
        await self.player.forward()

    async def lead(self):
        if self.player.food < 6:
            self.log("Low food, back to collect")
            self.state = 0
            return

        req = ELEVATION_REQS[self.player.level - 1]
        needed_players = req[0]

        sealed = secure.seal(self.key, self.id, self._next_counter(), f"R_{self.player.level}")
        await self.player.broadcast(sealed)

        view = await self.player.look()
        if view is None:
            return
        cell = view[0]

        if "food" in cell:
            await self.player.take("food")
            return

        player_count = cell.count("player")

        if player_count >= needed_players:
            self.log(f"Starting incantation ({player_count}/{needed_players} players)")

            for item in RESOURCES[1:]:
                for _ in range(cell.count(item)):
                    if await self.player.take(item) == "dead":
                        return

            for idx, item in enumerate(RESOURCES[1:]):
                for _ in range(req[idx + 1]):
                    if await self.player.set(item) == "dead":
                        return

            result = await self.player.incantation()
            if result == "ko" and self.attack_mode:
                self.log("Incantation failed! Ejecting bad player")
                await self.player.eject()

            self.state = 0
            self.forked = False

    async def follow(self):
        if self.player.food < 5:
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
                if await self.player.cmd(move) == "dead":
                    return

    async def wait_incantation(self):
        self.wait_ticks += 1
        food = self.player.food

        view = await self.player.look()
        if view is not None and "food" in view[0]:
            await self.player.take("food")

        if self.wait_ticks > 150 or food < 3:
            self.log("Incantation wait timeout, back to collect")
            self.state = 0
            self.leader_id = None
            self.leader_dir = -1
