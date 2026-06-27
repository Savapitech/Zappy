import sys
import asyncio
import logging
import stats
from bot import _spawn

logging.getLogger("asyncio").setLevel(logging.CRITICAL)


async def tui():
    sys.stdout.write("\033[2J")
    while True:
        sys.stdout.write("\033[H")
        sys.stdout.write(f"\033[1;37mBots alive: {stats.STATS['total']}\033[0m\n\n")
        for level in range(1, 9):
            count = stats.STATS['levels'][level]
            bar = "█" * min(count, 40)
            sys.stdout.write(f"\033[1;32mLevel {level}\033[0m : [\033[1;34m{bar:<40}\033[0m] {count}\n")
        sys.stdout.write("\n\033[1;33m--- Events ---\033[0m\n")
        for entry in stats.EVENTS:
            sys.stdout.write(f"{entry:<80}\n")
        sys.stdout.write("\033[J")
        sys.stdout.flush()
        await asyncio.sleep(0.5)


async def start_swarm(team, host, port):
    _spawn(team, host, port, 1)
    if not stats.DEBUG:
        asyncio.create_task(tui())
    try:
        while True:
            await asyncio.sleep(3600)
    except asyncio.CancelledError:
        pass


USAGE = "USAGE: ./zappy_ai -p port -n name -h machine\n"


def main():
    args = sys.argv[1:]
    if "--help" in args or "-help" in args:
        sys.stdout.write(USAGE)
        sys.exit(0)
    port = -1
    team = ""
    host = "127.0.0.1"
    for i in range(len(args)):
        if args[i] == "-p" and i + 1 < len(args):
            port = int(args[i + 1])
        elif args[i] == "-n" and i + 1 < len(args):
            team = args[i + 1]
        elif args[i] == "-h" and i + 1 < len(args):
            host = args[i + 1]
        elif args[i] == "--debug":
            stats.DEBUG = True

    if port < 0 or not team:
        sys.exit(84)

    try:
        asyncio.run(start_swarm(team, host, port))
    except KeyboardInterrupt:
        sys.exit(0)


if __name__ == "__main__":
    main()
