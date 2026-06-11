from ia.network.network import *
from ia.communication.communication import *
from ia.anthill.const import *

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
        self.state = Spawn
        self.level = 0

    #
    # Is usefull function
    #

    def up_tick(self, val: int):
        self.tick = self.tick + val if max_int - self.tick > val else val - max_int - self.tick

    def clean_server_msg(self, excluded: str):
        # clear all excluded message
        server_response = self.connection.read()
        splited = server_response.split("\n")
        for i in range(len(splited)):
            if splited[i] != excluded:
                self.all += splited[i] + "\n"

    #
    # Call the commande to the server and clean the response
    #

    def Look(self):
        self.connection.send("Look")
        self.up_tick(7)
        server_response = self.connection.read()
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
            server_response = self.connection.read()
            splited = server_response.split("\n")

    def Inv(self):
        self.connection.send("Inventory")
        self.up_tick(7)
        server_response = self.connection.read()
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
            server_response = self.connection.read()
            splited = server_response.split("\n")

    def Broadcast(self):
        self.all += self.connection.read()
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
            self.connection.send("Take food")
        elif "food" in self.overview[2]:
            self.connection.send("Forward")
            self.connection.send("Take food")
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
        self.all += self.connection.read()
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

    #
    # Is the main of the ia
    #

def run_ia(connection: network):
    my_ia = ia(connection)
    try:
        while my_ia.is_alive():
            my_ia.take_decision()
    except deathExeption as e:
        ()
    except Exception as e:
        print(e)
    return