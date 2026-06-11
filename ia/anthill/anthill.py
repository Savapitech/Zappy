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

    def up_tick(self, val: int):
        self.tick = self.tick + val if max_int - self.tick > val else val - max_int - self.tick

    def clean_server_msg(self, excluded: str):
        # clear all excluded message
        server_response = self.player.read()
        splited = server_response.split("\n")
        for i in range(len(splited)):
            if splited[i] != excluded:
                self.all += splited[i] + "\n"

    #
    # Call the commande to the server and clean the response
    #

    def Look(self):
        self.player.Look()
        self.up_tick(7)
        server_response = self.player.read()
        splited = server_response.split("\n")
        self.overview = ""
        while self.overview == "":
            for i in range(len(splited)):
                if splited[i] == "dead":
                    raise(deathExeption("You're dead"))
                if splited[i][0] == '[':
                    self.overview = splited[i]
                else:
                    self.all += splited[i] + "\n"
            server_response = self.player.read()
            splited = server_response.split("\n")

    def Inv(self):
        self.player.Inventory()
        self.up_tick(7)
        server_response = self.player.read()
        splited = server_response.split("\n")
        self.inv = ""
        while self.inv == "":
            for i in range(len(splited)):
                if splited[i] == "dead":
                    raise(deathExeption("You're dead"))
                if splited[i][:1] == '[':
                    self.inv = splited[i]
                    splited.pop(i)
                else:
                    self.all += splited[i] + "\n"
            server_response = self.player.read()
            splited = server_response.split("\n")

    def Broadcast(self):
        self.all += self.player.read()
        self.broadcast += self.all.split("\n")
        self.all = ""

    def Fork(self):
        ()

    #
    # Define each state comportement
    #

    def Spawn(self):
        self.Look()
        if "food" in self.overview[0]:
            self.player.Take("food")
        elif "food" in self.overview[2]:
            self.player.Forward()
            self.player.Take("food")
        else:
            self.fork()
        self.state = Collect

    def Collect(self):
        ()

    #
    # point towards the right behavior
    #

    def take_decision(self):
        if self.role == Workers:
            if self.state == Collect:
                self.Collect()
        elif self.role == Queen:
            if self.state == Spawn:
                self.Spawn()
            elif self.state == Collect:
                self.Collect()

    def is_alive(self):
        self.all += self.player.read()
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

async def run_ia(connection: network):
    my_ia = ia(connection)
    try:
        await my_ia.landing()
        while my_ia.is_alive():
            my_ia.take_decision()
    except deathExeption as e:
        ()
    except Exception as e:
        print(e)
    return