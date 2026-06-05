class network:
    def __init__(self, port: int, team: str, machine: str):
        self.port = port
        self.team = team
        self.machine = machine

    def connect(self):
        #connect to the server, don't worrie about raise it's handle
        return

    def send(self, msg: str):
        #send the msg to the server
        return

    def read(self):
        return str() #return the all the msg sended by the server

def connect(port: int, team: str, machine: str):
    connection = network(port, team, machine)
    try:
        connection.connect()
    except:
        raise(Exception("Connection to the server failed."))
    return connection