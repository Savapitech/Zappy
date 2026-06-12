from network.network import *
from communication.communication import *
from anthill.const import *
from player.player import *

class deathExeption(BaseException):
    def __init__(self, args, kwargs):
        self.args = args
        self.kwargs = kwargs

class ia:
    def __init__(self, team: str, connection: network, tick:int, key:int):
        self.tick = tick
        self.overview = ""
        self.inv = ""
        self.broadcast = []
        self.all = ""
        self.alive = True
        self.state = Spawn
        self.level = 0
        self.player = Player(connection)
        self.team = team
        self.role = Queen

    #
    # Fork
    #

    def startNewIa(self):
        self.player.connection.taskGroup.create_task(
            runIa(
                    self.player.connection.port,
                    self.team,
                    self.player.connection.machine,
                    self.player.connection.taskGroup,
                    self.tick,
                    #replace with key variable
                    0
            )
        )

    #
    # Is useful function
    #

    def upTick(self, val: int):
        """
        Update the curent tick of the ai by avoiding an overflow
        """
        self.tick = self.tick + val if maxInt - self.tick > val else val - maxInt - self.tick

    async def cleanServerMsg(self, excluded: str):
        """
        Clear all excluded message
        """
        serverResponse = await self.player.read()
        splited = serverResponse.split("\n")
        for i in range(len(splited)):
            if splited[i] != excluded:
                self.all += splited[i] + "\n"

    #
    # Call the command to the server and clean the response
    #

    async def Forward(self):
        """
        Make the ai move forward
        """
        await self.player.Forward()
        self.upTick(7)

    async def Right(self):
        """
        Make the ai turn to its right
        """
        await self.player.Right()
        self.upTick(7)

    async def Left(self):
        """
        Make the ai turn to its left
        """
        await self.player.Left()
        self.upTick(7)

    async def Look(self):
        """
        Get the information of all elements in front of the ai
        """
        await self.player.Look()
        self.upTick(7)
        serverResponse = await self.player.read()
        splited = serverResponse.split("\n")
        self.overview = ""
        while self.overview == "":
            for i in range(len(splited)):
                if splited[i] == "dead":
                    raise(deathExeption("You're dead"))
                if splited[i][:1] == '[':
                    self.overview = splited[i].split(",")
                else:
                    self.all += splited[i] + "\n"
            if self.overview == "":
                serverResponse = await self.player.read()
                splited = serverResponse.split("\n")

    async def Inv(self):
        """
        Get the information of the current inventory of the ai
        """
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

    async def Broadcast(self, msg: str):
        """
        Send a message to everyone
        """
        await self.player.Broadcast(msg)
        self.upTick(7)

    async def Fork(self):
        """
        Create a new slot for another ai
        """
        await self.player.Broadcast("Fork")
        self.all += self.player.readNoWait()
        await self.player.Fork()
        self.upTick(42)

    async def Eject(self):
        """
        Push all ai on the same cell
        """
        await self.player.Eject()
        self.upTick(7)

    async def Take(self, obj: str):
        """
        Try to take an object
        """
        await self.player.Take(obj)
        self.upTick(7)

    async def Set(self, obj: str):
        """
        Try removing an item from its inventory
        """
        await self.player.Set(obj)
        self.upTick(7)

    async def Incantation(self):
        """
        Try to process an elevation
        """
        await self.player.Incantation()
        self.upTick(300)

    #
    # Define each state comportement
    #

    async def Spawn(self):

        """
        Is the first behavior of the ai when she spawn
        """
        await self.Look()
        if "food" in self.overview[0]:
            await self.player.Take("food")
        elif "food" in self.overview[2]:
            await self.player.Forward()
            await self.player.Take("food")
        await self.Fork()
        self.state = Collect

    async def Collect(self):
        """
        Is the behavior of an ai when she's in collect state
        """
        pass

    #
    # point towards the right behavior
    #

    async def takeDecision(self):
        """
        Point towards the right behavior
        """
        if self.role == Workers:
            if self.state == Collect:
                await self.Collect()
        elif self.role == Queen:
            if self.state == Spawn:
                await self.Spawn()
            elif self.state == Collect:
                await self.Collect()

    async def isAlive(self):
        """
        Return if the ai is alive
        """
        self.all += await self.player.readNoWait()
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
        """
        Is the first step to connect the ai to the server
        """
        message = await self.player.read()
        if "WELCOME" not in message:
            raise Exception("No welcome message")
        await self.player.send(self.team + "\n")
        message = await self.player.read()
        if "ko" in message:
            raise Exception("Couldn't join")

    #
    # Is the main of the ia
    #

async def runIa(port: int, teamName: str, machine: str, tg: asyncio.TaskGroup, tick:int = 0, key:int = 0):
    try:
        connection = await connect(port, machine, tg)
        myIa = ia(teamName, connection, tick, key)
        await myIa.landing()
        while await myIa.isAlive():
            await myIa.takeDecision()
    except deathExeption as e:
        pass
    except Exception as e:
        print(e)
    return