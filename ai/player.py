from const import RESOURCES, ELEVATION_REQS


class Player:
    def __init__(self, net):
        self.net = net
        self.level = 1
        self.inventory_items = {item: 0 for item in RESOURCES}
        self.on_level_up = None

    def is_connected(self):
        return self.net.writer is not None and not self.net.writer.is_closing()

    @property
    def events(self):
        return self.net.events

    @property
    def food(self):
        return self.inventory_items.get("food", 0)

    async def cmd(self, command):
        if not self.is_connected():
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
                self._apply_level(int(resp[15:]))
                continue
            return resp

    def _apply_level(self, reported):
        old = self.level
        if reported > self.level:
            self.level = reported
        for item in RESOURCES[1:]:
            self.inventory_items[item] = 0
        if self.on_level_up:
            self.on_level_up(old, self.level)

    async def forward(self):
        return await self.cmd("Forward")

    async def left(self):
        return await self.cmd("Left")

    async def right(self):
        return await self.cmd("Right")

    async def take(self, item):
        return await self.cmd(f"Take {item}")

    async def set(self, item):
        return await self.cmd(f"Set {item}")

    async def broadcast(self, text):
        return await self.cmd(f"Broadcast {text}")

    async def fork(self):
        return await self.cmd("Fork")

    async def eject(self):
        return await self.cmd("Eject")

    async def connect_nbr(self):
        resp = await self.cmd("Connect_nbr")
        if resp != "dead" and resp.isdigit():
            return int(resp)
        return None

    async def look(self):
        resp = await self.cmd("Look")
        if resp == "dead":
            return None
        resp = resp.strip("[]").split(",")
        return [tile.strip().split() for tile in resp]

    async def inventory(self):
        resp = await self.cmd("Inventory")
        if resp == "dead":
            return None
        resp = resp.strip("[]").split(",")
        for entry in resp:
            parts = entry.strip().split()
            if len(parts) == 2 and parts[0] in self.inventory_items:
                try:
                    self.inventory_items[parts[0]] = int(parts[1])
                except ValueError:
                    pass
        return self.inventory_items

    async def incantation(self):
        resp = await self.cmd("Incantation")
        if resp != "Elevation underway":
            return resp
        result = await self.net.responses.get()
        if result == "dead":
            return "dead"
        if result.startswith("Current level: "):
            self._apply_level(int(result[15:]))
        return result

    def has_stones(self):
        if self.level >= 8:
            return False
        req = ELEVATION_REQS[self.level - 1]
        for idx, item in enumerate(RESOURCES[1:]):
            if self.inventory_items.get(item, 0) < req[idx + 1]:
                return False
        return True
