from network.network import *

class Player:
    def __init__(self, connection: network):
        self.connection = connection

    async def Forward(self):
        await self.connection.send("Forward\n")

    async def Right(self):
        await self.connection.send("Right\n")

    async def Left(self):
        await self.connection.send("Left\n")

    async def Look(self):
        await self.connection.send("Look\n")

    async def Inventory(self):
        await self.connection.send("Inventory\n")

    async def Broadcast(self, msg: bytes):
        await self.connection.send("Broadcast " + str(msg) + "\n")

    async def Connect_nbr(self):
        await self.connection.send("Connect_nbr\n")

    async def Fork(self):
        await self.connection.send("Fork\n")

    async def Eject(self):
        await self.connection.send("Eject\n")

    async def Take(self, obj: str):
        await self.connection.send("Take " + obj + "\n")

    async def Set(self, obj: str):
        await self.connection.send("Set " + obj + "\n")

    async def Incantation(self):
        await self.connection.send("Incantation\n")

    async def send(self, msg: str):
        await self.connection.send(msg)

    async def read(self):
        return await self.connection.read()

    def readNoWait(self):
        return self.connection.readNoWait()