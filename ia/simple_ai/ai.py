from ia.network.network import *
import sys

max_int = sys.maxsize

class ia:
    def __init__(self, connection: network):
        self.connection = connection
        self.tick = 0
        self.overview = ""
        self.inv = ""
        self.broadcast = []
        self.all = ""

    def up_tick(self, val: int):
        self.tick = self.tick + val if max_int - self.tick > val else val - max_int - self.tick

    def Look(self):
        self.connection.send("Look")
        self.up_tick(7)
        server_response = self.connection.read()
        splited = server_response.split("\n")
        self.overview = ""
        while self.overview == "":
            for i in range(len(splited)):
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
                if splited[i][0] == '[':
                    self.inv = splited[i]
                    splited.pop(i)
                else:
                    self.all += splited[i] + "\n"
            server_response = self.connection.read()
            splited = server_response.split("\n")

    def Broadcast(self):
        server_response = self.connection.read()
        self.all += server_response
        self.broadcast = self.all.split("\n")

    def get_info(self):
        self.Inv()
        self.Look()
        self.Broadcast()

    def take_decision(self):
        cell_content = self.overview.split(",")
        if "food" in cell_content[0]:
            self.connection.send("Take food")
        if "food" in cell_content[2]:
            self.connection.send("Forward")
            self.connection.send("Take food")
            if "food" in cell_content[1]:
                self.connection.send("Left")
                self.connection.send("Forward")
                self.connection.send("Take food")
            elif "food" in cell_content[3]:
                self.connection.send("Right")
                self.connection.send("Forward")
                self.connection.send("Take food")
        elif "food" in cell_content[1]:
            self.connection.send("Forward")
            self.connection.send("Left")
            self.connection.send("Forward")
            self.connection.send("Take food")
        elif "food" in cell_content[3]:
            self.connection.send("Forward")
            self.connection.send("Right")
            self.connection.send("Forward")
            self.connection.send("Take food")
        server_response = self.connection.read()
        splited = server_response.split("\n")
        for i in range(len(splited)):
            if splited[i] != "ok":
                self.all += splited[i] + "\n"

def run_ia(connection: network):
    my_ia = ia()
    ia.get_info()
    ia.take_decision()
    return