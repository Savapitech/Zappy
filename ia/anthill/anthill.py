from network.network import *
from communication.communication import *
from anthill.const import *
from player.player import *

class deathExeption(BaseException):
    def __init__(self, args, kwargs):
        self.args = args
        self.kwargs = kwargs

class ia:
    def __init__(self, connection: network):
        self.tick = 0
        self.overview = ""
        self.inv = ""
        self.broadcast = []
        self.all = ""
        self.alive = True
        self.state = Spawn
        self.level = 0
        self.player = Player(connection)

    #
    # Is usefull function
    #


    def upTick(self, val: int):
        self.tick = self.tick + val if maxInt - self.tick > val else val - maxInt - self.tick

    async def cleanServerMsg(self, excluded: str):
        # clear all excluded message
        serverResponse = await self.player.read()
        splited = serverResponse.split("\n")
        for i in range(len(splited)):
            if splited[i] != excluded:
                self.all += splited[i] + "\n"

    #
    # Call the commande to the server and clean the response
    #

    async def Forward(self):
        await self.player.Forward()
        self.upTick(7)

    async def Right(self):
        await self.player.Right()
        self.upTick(7)

    async def Look(self):
        await self.player.Look()
        self.upTick(7)
        serverResponse = await self.player.read()
        splited = serverResponse.split("\n")
        self.overview = ""
        while self.overview == "":
            for i in range(len(splited)):
                if splited[i] == "dead":
                    raise(deathExeption("You're dead"))
                if splited[i][0] == '[':
                    self.overview = splited[i]
                else:
                    self.all += splited[i] + "\n"
            if self.overview == "":
                serverResponse = await self.player.read()
                splited = serverResponse.split("\n")

    async def Inv(self):
        await self.player.Inventory()
        self.upTick(7)
        serverResponse = await self.player.read()
        splited = serverResponse.split("\n")
        self.inv = ""
        while self.inv == "":
            for i in range(len(splited)):
                if splited[i] == "dead":
                    raise(deathExeption("You're dead"))
                if splited[i][:1] == '[':
                    self.inv = splited[i]
                else:
                    self.all += splited[i] + "\n"
            if self.overview == "":
                serverResponse = await self.player.read()
                splited = serverResponse.split("\n")

    async def Broadcast(self):
        self.all += await self.player.read()
        self.broadcast += self.all.split("\n")
        self.all = ""

    async def Fork(self):
        pass

    #
    # Define each state comportement
    #

    async def Spawn(self):
        self.Look()
        if "food" in self.overview[0]:
            await self.player.Take("food")
        elif "food" in self.overview[2]:
            await self.player.Forward()
            await self.player.Take("food")
        else:
            self.fork()
        self.state = Collect

    async def Collect(self):
        pass

    #
    # point towards the right behavior
    #

    async def takeDecision(self):
        if self.role == Workers:
            if self.state == Collect:
                await self.Collect()
        elif self.role == Queen:
            if self.state == Spawn:
                await self.Spawn()
            elif self.state == Collect:
                await self.Collect()

    async def isAlive(self):
        self.all += await self.player.read()
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
        message = await self.player.read()
        if "WELCOME" not in message:
            raise Exception("No welcome message")
        await self.player.send(self.player.team + "\n")
        message = await self.player.read()
        if "ko" in message:
            raise Exception("Couldn't join")
        await self.player.Fork()

    #
    # Is the main of the ia
    #

async def runIa(connection: network):
    myIa = ia(connection)
    try:
        await myIa.landing()
        while await myIa.isAlive():
            await myIa.takeDecision()
    except deathExeption as e:
        ()
    except Exception as e:
        print(e)
    return