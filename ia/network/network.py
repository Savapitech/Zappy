import socket

class network:
    def __init__(self, host: str, port: int, team: str, machine: str):
        self.machine = machine
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.team = team

    def connect(self):
        self.socket.connect((self.machine, self.port)) #connect to the machine without handling raise

    def disconnect(self):
        self.socket.close()

    def send(self, msg: str):
        self.socket.send(msg.encode("utf-8"))

    def read(self):
        buffer = self.socket.recv(1024)
        return buffer.decode("utf-8") #return the all the msg sended by the server

def connect(port: int, team: str, machine: str):
    connection = network(port, team, machine)
    try:
        connection.connect()
    except:
        raise(Exception("Connection to the server failed."))
    return connection