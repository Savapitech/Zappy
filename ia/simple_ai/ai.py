from network.network import *
import sys

max_int = sys.maxsize

class deathExeption(BaseException):
    def __init__(self, args, kwargs):
        self.args = args
        self.kwargs = kwargs

class ia:
    def __init__(self, connection: network):
        self.connection = connection
        self.tick = 0
        self.overview = ""
        self.inv = ""
        self.broadcast = []
        self.all = ""
        self.alive = True

    def up_tick(self, val: int):
        self.tick = self.tick + val if max_int - self.tick > val else val - max_int - self.tick

    async def Look(self):
        await self.connection.send("Look\n")
        self.up_tick(7)
        server_response = await self.connection.read()
        splited = server_response.split("\n")
        self.overview = ""
        while self.overview == "":
            for i in range(len(splited)):
                if splited[i] == "dead":
                    raise(deathExeption("You're dead"))
                if splited[i][:1] == '[':
                    self.overview = splited[i]
                else:
                    self.all += splited[i] + "\n"
            if self.overview != "":
                break
            server_response = await self.connection.read()
            splited = server_response.split("\n")

    async def Inv(self):
        await self.connection.send("Inventory\n")
        self.up_tick(7)
        server_response = await self.connection.read()
        splited = server_response.split("\n")
        self.inv = ""
        while self.inv == "":
            for i in range(len(splited)):
                if splited[i] == "dead":
                    raise(deathExeption("You're dead"))
                if splited[i][:1] == "[":
                    self.inv = splited[i]
                else:
                    self.all += splited[i] + "\n"
            if self.inv != "":
                break
            server_response = await self.connection.read()
            splited = server_response.split("\n")

    async def Broadcast(self):
        self.all += await self.connection.read()
        self.broadcast += self.all.split("\n")
        self.all = ""

    async def get_info(self):
        await self.Inv()
        await self.Look()

    async def take_decision(self):
        cell_content = self.overview.split(",")
        if "food" in cell_content[0]:
            await self.connection.send("Take food\n")
        if "food" in cell_content[2]:
            await self.connection.send("Forward\n")
            await self.connection.send("Take food\n")
            if "food" in cell_content[1]:
                await self.connection.send("Left\n")
                await self.connection.send("Forward\n")
                await self.connection.send("Take food\n")
            elif "food" in cell_content[3]:
                await self.connection.send("Right\n")
                await self.connection.send("Forward\n")
                await self.connection.send("Take food\n")
        elif "food" in cell_content[1]:
            await self.connection.send("Forward\n")
            await self.connection.send("Left\n")
            await self.connection.send("Forward\n")
            await self.connection.send("Take food\n")
        elif "food" in cell_content[3]:
            await self.connection.send("Forward\n")
            await self.connection.send("Right\n")
            await self.connection.send("Forward\n")
            await self.connection.send("Take food\n")
        else:
            await self.connection.send("Forward\n")
        server_response = await self.connection.read()
        splited = server_response.split("\n")
        for i in range(len(splited)):
            if splited[i] != "ok":
                self.all += splited[i] + "\n"

    async def is_alive(self):
        for msg in self.broadcast:
            if msg == "dead":
                self.alive = False
                break
        splited = self.all.split("\n")
        for msg in splited:
            if msg == "dead":
                self.alive = False
                break
        return self.alive
    
    async def landing(self):
        message = await self.connection.read()
        if "WELCOME" not in message:
            raise Exception("No welcome message")
        await self.connection.send(self.connection.team + "\n")
        message = await self.connection.read()
        if "ko" in message:
            raise Exception("Couldn't join")
        await self.connection.send("Fork\n")

async def run_ia(connection: network):
    my_ia = ia(connection)
    try:
        await my_ia.landing()
        while await my_ia.is_alive():
            await my_ia.get_info()
            await my_ia.take_decision()
    except deathExeption as e:
        pass
    except Exception as e:
        print(e)
    finally:
        await connection.disconnect()
    return