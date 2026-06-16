from network.network import *
from communication.communication import *
from anthill.const import *
from player.player import *

class deathExeption(BaseException):
    pass

class ia:
    def __init__(self, team: str, connection: network, tick:int, key:int, role: int, state = Spawn):
        self.tick = tick
        self.overview = ""
        self.inv = ""
        self.broadcast = []
        self.all = ""
        self.alive = True
        self.state = state
        self.level = 0
        self.player = Player(connection)
        self.team = team
        self.role = role
        self.key = key
        self.prev = 0

    #
    # Fork
    #

    def startNewIa(self, role: int, state: int = Collect):
        self.player.connection.taskGroup.create_task(
            runIa(
                    port= self.player.connection.port,
                    teamName= self.team,
                    machine= self.player.connection.machine,
                    tg= self.player.connection.taskGroup,
                    tick= self.tick,
                    key= self.key,
                    role= role,
                    state= state
            )
        )

    async def collectRessource(self):
        for i in range(0, 7):
            if ressources[i] in self.overview[0]:
                await self.Take(ressources[i])

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

    async def readUntil(self):
        msg = ""
        needToContinue = True
        result = ""
        while needToContinue:
            msg = await self.player.read()
            for s in msg.split("\n")[:-1]:
                if s == "dead":
                    raise(deathExeption("You're dead"))
                if "message" not in s:
                    needToContinue = False
                    result = s
                self.broadcast += s
        return result

    #
    # Call the command to the server and clean the response
    #

    async def Forward(self):
        """
        Make the ai move forward
        """
        await self.player.Forward()
        self.upTick(7)
        await self.readUntil()

    async def Right(self):
        """
        Make the ai turn to its right
        """
        await self.player.Right()
        self.upTick(7)
        await self.readUntil()

    async def Left(self):
        """
        Make the ai turn to its left
        """
        await self.player.Left()
        self.upTick(7)
        await self.readUntil()

    async def Look(self):
        """
        Get the information of all elements in front of the ai
        """
        await self.player.Look()
        self.upTick(7)
        self.overview = (await self.readUntil()).split(",")

    async def Inv(self):
        """
        Get the information of the current inventory of the ai
        """
        await self.player.Inventory()
        self.upTick(7)
        self.inv = (await self.readUntil()).split(",")
        for i, elem in enumerate(self.inv):
            self.inv[i] = int(elem.split(" ")[2])

    async def Broadcast(self, msg: str):
        """
        Send a message to everyone
        """
        msg, self.key = crypt(self.role, msg, self.key)
        await self.player.Broadcast(msg)
        self.upTick(7)
        await self.readUntil()

    async def Connect_nbr(self):
        """
        Send a message to everyone
        """
        await self.player.Connect_nbr()
        return int(await self.readUntil())

    async def Fork(self, role: int):
        """
        Create a new slot for another ai
        """
        self.all += self.player.readNoWait()
        await self.player.Fork()
        self.upTick(42)
        if await self.readUntil() == "ok":
            self.startNewIa(role)

    async def Eject(self):
        """
        Push all ai on the same cell
        """
        await self.player.Eject()
        self.upTick(7)
        await self.readUntil()

    async def Take(self, obj: str):
        """
        Try to take an object
        """
        await self.player.Take(obj)
        self.upTick(7)
        await self.readUntil()

    async def Set(self, obj: str):
        """
        Try removing an item from its inventory
        """
        await self.player.Set(obj)
        self.upTick(7)
        await self.readUntil()

    async def Incantation(self):
        """
        Try to process an elevation
        """
        await self.player.Incantation()
        self.upTick(300)
        res = await self.readUntil()
        if res == "Elevation underway":
            await self.readUntil()
            self.level += 1

    #
    # Define each state comportement
    #

    async def Spawn(self):

        """
        Is the first behavior of the ai when she spawn
        """
        await self.Look()
        if "food" in self.overview[0]:
            await self.Take("food")
        elif "food" in self.overview[2]:
            await self.Forward()
            await self.Take("food")
        await self.Fork(Workers)
        self.state = Collect

    async def Collect(self):
        """
        Is the behavior of an ai when she's in collect state
        """
        await self.Look()
        if "food" in self.overview[0]:
            await self.collectRessource()
        elif "food" in self.overview[2]:
            await self.Forward()
            await self.collectRessource()
            if "food" in self.overview[1]:
                await self.Left()
                await self.Forward()
                await self.collectRessource()
            elif "food" in self.overview[3]:
                await self.Right()
                await self.Forward()
                await self.collectRessource()
        elif "food" in self.overview[1]:
            await self.Forward()
            await self.Left()
            await self.Forward()
            await self.collectRessource()
        elif "food" in self.overview[3]:
            await self.Forward()
            await self.Right()
            await self.Forward()
            await self.collectRessource()
        else:
            await self.Forward()

    async def Call(self):
        """
        Is the behavior of the queen when she want to level up
        """
        needed = int(steps[self.level][0]) - 1

        for i in range(1, 7):
            if steps[self.level][i] > self.inv[i]:
                self.state = Collect
                return
        await self.Broadcast("WHO IS ALIVE ?")

        for i in range(30):
            self.broadcast += self.player.readNoWait().split("\n")

        nbAlive = 0
        for broad in self.broadcast:
            msg, new_key, _ = decrypt(broad[11:], self.key)
            if msg == "ME MY QUEEN":
                self.key = new_key
                nbAlive += 1
        self.broadcast = []

        if nbAlive < needed:
            await self.Fork(Workers)
            self.state = Collect
            return

        nbHere = 0
        while nbHere < needed:
            await self.Broadcast("WHERE ARE YOU NOW ?")
            for i in range(30):
                self.broadcast += self.player.readNoWait().split("\n")
            for broad in self.broadcast:
                msg, new_key, _ = decrypt(broad[11:], self.key)
                if msg == "HERE MY QUEEN" and broad[8:9] == "0":
                    self.key = new_key
                    nbHere += 1
            self.broadcast = []
        await self.Incantation()
        await self.Broadcast("END OF THE INCANTATION")
        self.state = Collect

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
                if self.tick - self.prev >= 210:
                    self.prev = self.tick
                    await self.Inv()
                    if self.inv[FOOD] > 10:
                        await self.Fork(Workers)
                    if self.inv[FOOD] > 30 and self.level < 7:
                        self.state = Call
            elif self.state == Call:
                await self.Call()


    async def isAlive(self):
        """
        Return if the ai is alive
        """
        self.all += self.player.readNoWait()
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
        message = message.split("\n")
        if len(message) < 2:
            await self.readUntil()

    #
    # Is the main of the ia
    #

async def runIa(port: int, teamName: str, machine: str, tg: asyncio.TaskGroup, tick: int = 0, key: int = 0, role: int = Queen, state: int = Spawn):
    try:
        connection = await connect(port, machine, tg)
        myIa = ia(teamName, connection, tick, key, role, state)
        await myIa.landing()
        while await myIa.isAlive():
            await myIa.takeDecision()
    except deathExeption as e:
        pass
    except Exception as e:
        print(e)
    return