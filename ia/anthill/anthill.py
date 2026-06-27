from network.network import *
from communication.communication import *
from anthill.const import *
from player.player import *
import base64


def sortWeaknest(weak):
    return weak[1]



class deathExeption(BaseException):
    pass



class ia:
    def __init__(self, team: str, connection: network, tick:int, key:int, role: int, state = Spawn, id: int = 0):
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
        self.id = id
        self.nextid = 1
        self.follow = ""
        self.waiting = ""
        self.savedBroadcast = []
        self.sabotage = []
        self.weakest = []
        self.needed = 0
        self.nbReady = 0
        self.connectNbr = 0
        self.waitIncanting = ""
        self.strId = chr((int(self.id * 0.01) + A)) + chr((int(self.id * 0.1) % 100 + A)) + chr((self.id % 10 + A))

    #
    # Fork
    #

    def startNewIa(self, role: int, state: int = Collect):
        """
        Start a new ai on an empty egg.
        """
        self.player.connection.taskGroup.create_task(
            runIa(
                    port= self.player.connection.port,
                    teamName= self.team,
                    machine= self.player.connection.machine,
                    tg= self.player.connection.taskGroup,
                    tick= self.tick,
                    key= self.key,
                    role= role,
                    state= state,
                    id= self.nextid
            )
        )
        self.nextid += 1



    async def collectRessource(self):
        """
        Collect all ressources depending on the current state.
        """
        if self.state == Collect:
            for i in range(0, 7):
                if ressources[i] in self.overview[0]:
                    await self.Take(ressources[i])
        else:
            await self.Take("food")





    #
    # Is useful function
    #





    def upTick(self, val: int):
        """
        Update the curent tick of the ai by avoiding an overflow.
        """
        self.tick = self.tick + val if maxInt - self.tick > val else val - maxInt - self.tick



    async def readUntil(self):
        """
        Read all data received until a data is not equal to a broadcast, and return this data.
        """
        msg = ""
        needToContinue = True
        result = ""
        while needToContinue:
            msg = await self.player.read()
            for s in msg.split("\n")[:-1]:
                if s == "dead":
                    raise(deathExeption("You're dead"))
                elif "message" not in s:
                    needToContinue = False
                    result = s
                else:
                    self.broadcast.append(s)
        return result



    async def wait(self, wait: int):
        """
        Make the ai wait.
        """
        for i in range(0, wait):
            await self.Look()





    #
    # Call the command to the server and clean the response
    #






    async def Forward(self):
        """
        Make the ai move forward.
        """
        await self.player.Forward()
        self.upTick(7)
        await self.readUntil()

    async def Right(self):
        """
        Make the ai turn to its right.
        """
        await self.player.Right()
        self.upTick(7)
        await self.readUntil()

    async def Left(self):
        """
        Make the ai turn to its left.
        """
        await self.player.Left()
        self.upTick(7)
        await self.readUntil()

    async def Look(self):
        """
        Get the information of all elements in front of the ai.
        """
        await self.player.Look()
        self.upTick(7)
        self.overview = (await self.readUntil()).split(",")



    async def Inv(self):
        """
        Get the information of the current inventory of the ai.
        """
        await self.player.Inventory()
        self.upTick(7)
        self.inv = (await self.readUntil()).split(",")
        self.inv[-1] = self.inv[-1][:-1]
        for i, elem in enumerate(self.inv):
            self.inv[i] = int(elem.split(" ")[-1])



    async def Broadcast(self, msg: str):
        """
        Send a message to everyone.
        """
        msg, self.key = crypt(self.role, msg, self.key)
        await self.player.Broadcast(msg)
        self.upTick(7)
        await self.readUntil()



    async def Connect_nbr(self):
        """
        Send a message to everyone.
        """
        await self.player.Connect_nbr()
        return int(await self.readUntil())



    async def Fork(self, role: int):
        """
        Create a new slot for another ai.
        """
        await self.readQueen()
        if self.connectNbr:
            self.startNewIa(role, Survivor)
        else:
            await self.player.Fork()
            self.upTick(42)
            if await self.readUntil() == "ok":
                self.startNewIa(role, Survivor)



    async def Eject(self):
        """
        Push all ai on the same cell.
        """
        await self.player.Eject()
        self.upTick(7)
        await self.readUntil()



    async def Take(self, obj: str):
        """
        Try to take an object.
        """
        await self.player.Take(obj)
        self.upTick(7)
        await self.readUntil()



    async def Set(self, obj: str):
        """
        Try removing an item from its inventory.
        """
        await self.player.Set(obj)
        self.upTick(7)
        await self.readUntil()



    async def Incantation(self):
        """
        Try to process an elevation.
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
        self.connectNbr = await self.Connect_nbr()
        self.state = Survivor



    async def Collect(self):
        """
        Is the behavior of an ai when she's in collect state.
        """
        await self.Look()
        if "food" in self.overview[0]:
            await self.collectRessource()
        elif "food" in self.overview[2] and not "player" in self.overview[2]:
            await self.Forward()
            await self.collectRessource()
            if "food" in self.overview[1] and not "player" in self.overview[1]:
                await self.Left()
                await self.Forward()
                await self.collectRessource()
            elif "food" in self.overview[3] and not "player" in self.overview[3]:
                await self.Right()
                await self.Forward()
                await self.collectRessource()
        elif "food" in self.overview[1] and not "player" in self.overview[1]:
            await self.Forward()
            await self.Left()
            await self.Forward()
            await self.collectRessource()
        elif "food" in self.overview[3] and not "player" in self.overview[3]:
            await self.Forward()
            await self.Right()
            await self.Forward()
            await self.collectRessource()
        else:
            await self.Forward()



    async def Call(self):
        """
        Is the behavior of an ai when she want to level up.
        """
        self.needed = int(steps[self.level][0]) - 1
        self.weakest = []

        for i in range(1, 7):
            if steps[self.level][i] > self.inv[i]:
                self.state = Collect
                if self.role != Queen:
                   await self.Broadcast("END OF THE INCANTATION" + self.strId)
                return
        if self.needed >= 1:
            await self.Broadcast("WHO IS ALIVE ?" + chr(self.level + A) + self.strId)
            await self.wait(10)

            for i in range(30):
                self.broadcast += self.player.readNoWait().split("\n")

            self.nbReady = 0
            prevKey = self.key
            for broad in self.broadcast:
                self.savedBroadcast.append(broad)
                msg, new_key, _ = decrypt(broad[11:], self.key)
                if "NOT ME MY QUEEN" in msg:
                    prevKey = self.key
                    self.key = new_key
                elif "I CAN'T MY QUEEN" in msg:
                    prevKey = self.key
                    self.key = new_key
                    self.weakest.append([msg[-4:-1], msg[-1:]])
                elif "ME MY QUEEN" in msg:
                    prevKey = self.key
                    self.key = new_key
                    self.nbReady += 1
                else:
                    msg, new_key, _ = decrypt(broad[11:], prevKey)
                    if "NOT ME MY QUEEN" in msg:
                        pass
                    elif "ME MY QUEEN" in msg:
                        self.nbReady += 1
                    elif "I CAN'T MY QUEEN" in msg:
                        self.weakest.append([msg[-4:-1], msg[-1:]])
            if self.nbReady < self.needed:
                self.state = Collect
                if len(self.weakest):
                    self.weakest.sort(key=sortWeaknest)
                    await self.Broadcast("IMPROVE YOURSELF" + self.weakest[-1:][0][0])
                    self.state = WaitingIncanting
                    self.waitIncanting = self.weakest[-1:][0][0]
                else:
                    self.state = Survivor
                return

            nbHere = 0
            prevKey = self.key
            while nbHere < self.needed:
                await self.Broadcast("WHERE ARE YOU NOW ?" + self.strId)
                await self.wait(8)
                for i in range(30):
                    self.broadcast += self.player.readNoWait().split("\n")
                self.savedBroadcast += self.broadcast
                for broad in self.broadcast:
                    msg, new_key, _ = decrypt(broad[11:], self.key)
                    if "HERE MY QUEEN" in msg:
                        prevKey = self.key
                        self.key = new_key
                        if broad[8:9] == "0":
                            nbHere += 1
                    else:
                        msg, new_key, _ = decrypt(broad[11:], prevKey)
                        if "HERE MY QUEEN" in msg:
                            if broad[8:9] == "0":
                                nbHere += 1
                self.broadcast = []

        await self.Broadcast("START OF THE INCANTATION" + self.strId)
        for i in range(1, 7):
            for _ in range(steps[self.level][i]):
                await self.Set(ressources[i])
        await self.Incantation()
        await self.Broadcast("END OF THE INCANTATION" + self.strId)
        self.state = Collect
        self.needed = 0





    #
    # point towards the right behavior
    #





    async def readWorker(self):
        """
        Make the workers read all message and processes them.
        """
        self.broadcast += self.player.readNoWait().split("\n")
        state = []
        keep = ""
        for broad in self.broadcast:
            msg, new_key, _ = decrypt(broad[11:], self.key)
            if msg[-3:] == self.strId:
                pass
            if "WHO IS ALIVE" in msg:
                self.key = new_key
                self.follow = msg[-3:]
                await self.Inv()
                for i in range(1, 7):
                    if steps[self.level][i] > self.inv[i]:
                        state.append(Hungry)
                        if self.inv[FOOD] > 15:
                            self.state = Collect
                        else:
                            self.state = Survivor
                if Hungry not in state:
                    if ord(msg[14:15]) == self.level + A:
                        if self.inv[FOOD] >= 20:
                            state.append(Join)
                        else:
                            state.append(Hungry)
                    elif ord(msg[14:15]) < self.level + A:
                        pass
                    else:
                        state.append(Weak)
            elif "WHERE ARE YOU NOW" in msg:
                self.key = new_key
                if self.state == Join and msg[-3:] == self.follow:
                    keep = broad[8:9]
                    state.append(Joining)
            elif "IMPROVE YOURSELF" in msg:
                self.key = new_key
                keep = msg[-3:]
                state.append(Improving)
            elif "ME MY QUEEN" in msg or "HERE MY QUEEN" in msg or "NOT ME MY QUEEN" in msg or "I CAN'T MY QUEEN" in msg:
                self.key = new_key
            elif "END OF THE INCANTATION" in msg:
                self.key = new_key
                if self.state == WaitingIncanting and msg[-3:] == self.waitIncanting:
                    if len(self.weakest):
                        self.weakest.pop()
                    self.needed -= 1
                    state.append(Improving)
                    keep = self.strId
                if self.follow == msg[-3:] or self.waiting == msg[-3:]:
                    self.follow = ""
                    self.state = Survivor
            elif "START OF THE INCANTATION" in msg:
                self.key = new_key
                if self.state != Here and self.state != WaitingIncanting:
                    self.follow = ""
                    self.state = ForcedSurvivor
                    self.waiting = msg[-3:]
            elif "ME MY QUEEN" in msg :
                self.key = new_key
            elif broad != "":
                self.sabotage.append(broad[11:])
        self.broadcast = []
        for s in state:
            if s == Join :
                self.state = Join
                await self.Broadcast("ME MY QUEEN         ")
            elif s == Weak:
                await self.Broadcast("I CAN'T MY QUEEN" + self.strId + chr(self.level + 1))
                self.follow = ""
            elif s == Hungry:
                await self.Broadcast("NOT ME MY QUEEN     ")
                self.follow = ""
            elif s == Improving:
                if keep == self.strId:
                    if self.needed and len(self.weakest):
                        await self.Broadcast("IMPROVE YOURSELF" + self.weakest[-1:][0][0])
                        self.waitIncanting = self.weakest[-1:][0][0]
                    else:
                        await self.Call()
            elif s == Joining:
                await self.Broadcast("HERE MY QUEEN" + self.follow)
                dir = keep
                if dir == "1" or dir == "2" or dir == "8":
                    await self.Forward()
                elif dir == "3" or dir == "4":
                    await self.Left()
                    await self.Forward()
                elif dir == "7" or dir == "6":
                    await self.Right()
                    await self.Forward()
                elif dir == "5":
                    await self.Left()
                    await self.Left()
                    await self.Forward()
                elif dir == "0":
                    self.state = Here



    async def readQueen(self):
        """
        Make the queen read all message and processes them.
        """
        self.broadcast += self.player.readNoWait().split("\n")
        for broad in self.broadcast:
            msg, new_key, _ = decrypt(broad[11:], self.key)
            if msg[-3:] == self.strId:
                pass
            if ("WHO IS ALIVE" in msg or "WHERE ARE YOU NOW" in msg or "IMPROVE YOURSELF" in msg or "ME MY QUEEN" in msg or "NOT ME MY QUEEN" in msg or "I CAN'T MY QUEEN" in msg or "HERE MY QUEEN" in msg or "START OF THE INCANTATION" in msg) and msg[-3:] != self.strId:
                self.key = new_key
            elif "END OF THE INCANTATION" in msg:
                self.key = new_key
                if msg[-3:] == self.waitIncanting:
                    self.weakest.pop()
                    self.needed -= 1
                    if not self.needed:
                        await self.Call()
                    elif len(self.weakest):
                        await self.Broadcast("IMPROVE YOURSELF" + self.weakest[-1:][0][0])
                        self.waitIncanting = self.weakest[-1:][0][0]
                    else:
                        self.state = Survivor
        self.broadcast = []



    async def takeDecision(self):
        """
        Point towards the right behavior
        """
        if self.role == Workers:
            if self.state == Collect or self.state == Survivor or self.state == ForcedSurvivor or self.state == WaitingIncanting:
                await self.Collect()
                if self.tick - self.prev >= 700 and self.state != ForcedSurvivor and self.state != WaitingIncanting:
                    await self.Inv()
                    self.prev = self.tick
                    if self.inv[FOOD] > 20:
                        self.state = Collect
                    elif self.inv[FOOD] <= 20:
                        self.state == Survivor
            elif self.state == Improving:
                await self.Call()
            elif self.state == Join:
                await self.Inv()
            elif self.state == Here:
                result = await self.readUntil()
                if result == "Elevation underway":
                    await self.readUntil()
                    self.level += 1
                elif "Current level:" in result:
                    self.level += 1
                self.state = Survivor
            await self.readWorker()
        elif self.role == Queen:
            if self.state == Spawn:
                await self.Spawn()
            elif self.state == Collect or self.state == Survivor or self.state == WaitingIncanting:
                await self.Collect()
                if self.state != WaitingIncanting and self.tick - self.prev >= 700:
                    await self.Inv()
                    self.prev = self.tick
                    if self.inv[FOOD] > 20 and self.inv[PHIRAS] < 6:
                        self.state = Collect
                    elif self.inv[FOOD] <= 20 or self.inv[PHIRAS] >= 6:
                        self.state == Survivor
                    if self.inv[FOOD] > 20:
                        await self.Fork(Workers)
                    if self.inv[FOOD] > 30 and self.level < 7:
                        self.state = Call
                await self.readQueen()
            elif self.state == Call:
                await self.Call()
                self.prev = self.tick



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
        self.broadcast += [base64.b64decode(broad.encode()).decode("ascii") for broad in splited]
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
        if len(message) < 2 or message[1] == "":
            await self.readUntil()





    #
    # Is the main of the ia
    #





async def runIa(port: int, teamName: str, machine: str, tg: asyncio.TaskGroup, tick: int = 0, key: int = 0, role: int = Queen, state: int = Spawn, id: int = 0):
    try:
        connection = await connect(port, machine, tg)
        myIa = ia(teamName, connection, tick, key, role, state, id)
        await myIa.landing()
        while await myIa.isAlive():
            await myIa.takeDecision()
    except deathExeption as e:
        pass
    except Exception as e:
        print(e)
    return