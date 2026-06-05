from ia.network.network import *
import sys

max_int = sys.maxsize

class ia:
    def __init__(self):
        self.tick = 0

    def up_tick(self, val: int):
        self.tick = self.tick + val if max_int - self.tick > val else val - max_int - self.tick

def Look(connection: network, all: str, ia: ia):
    connection.send("Look")
    ia.up_tick(7)
    server_response = connection.read()
    splited = server_response.split("\n")
    overview = ""
    while overview == "":
        for i in range(len(splited)):
            if splited[i][0] == '[':
                overview = splited[i]
            else:
                all += splited[i] + "\n"
        server_response = connection.read()
        splited = server_response.split("\n")
    return overview, all

def Inv(connection: network, all: str, ia: ia):
    connection.send("Inventory")
    ia.up_tick(7)
    server_response = connection.read()
    splited = server_response.split("\n")
    inv = ""
    while inv == "":
        for i in range(len(splited)):
            if splited[i][0] == '[':
                inv = splited[i]
                splited.pop(i)
            else:
                all += splited[i] + "\n"
        server_response = connection.read()
        splited = server_response.split("\n")
    return inv, all

def take_decision(all:str, ):
    return

def get_info(connection: network, all: str, ia: ia):
    overview, all = Look(connection, all, ia)
    inv, all = Inv(connection, all, ia)
    # broadcast = Broadcast(all)
    return overview, inv, all

def run_ia(connection: network):
    my_ia = ia()
    overview, inv, broadcast = get_info(connection, my_ia)
    take_decision(inv, overview)

    return