import sys
from ia.network.network import *
from ia.simple_ai.ai import *
arg = sys.argv[1:]

def help():
    print("USAGE: ./zappy_ai -p port -n name -h machine")
    print("-p port:\t\tport number")
    print("-n name:\t\tname of the team")
    print("-h machine:\t\tname of the machine; localhost by default")

def get_argument(arg):
    port = -1
    name = ""
    machine = "localhost"
    machine_changed = False
    for i in range(0, len(arg), 2):
        if arg[i] == "-p":
            if port >= 0:
                raise(Exception("Multiple definition of port"))
            port = int(arg[i + 1])
        elif arg[i] == "-n":
            if name != "":
                raise(Exception("Multiple definition of team name"))
            name = arg[i + 1]
        elif arg[i] == "-h":
            if machine_changed:
                raise(Exception("Multiple definition of machine name"))
            machine = arg[i + 1]
            machine_changed = True
        else:
            raise(Exception(f"Unknow flag: {arg[i]}"))
    if port < 0:
        raise(Exception("Missing port"))
    if name == "":
        raise(Exception("Missing team name"))
    return port, name, machine


def main():
    if len(arg) < 4:
        help()
        try:
            if arg[0] == "-h" or arg[0] == "--help":
                return 0
        except:
            return 84
    port = 0
    name = ""
    machine = "localhost"
    try:
        port, name, machine = get_argument(arg)
        connection = connect(port, name, machine)
        ia(connection)
    except IndexError:
        print("Incomplete argument")
    except Exception as e:
        print(e)
        return 84
    return 0

if __name__ == "__main__":
    exit(main())