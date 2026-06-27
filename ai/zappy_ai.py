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
        sys.stdout.write(f"\033[1;37mBots alive: {stats.S['t']}\033[0m\n\n")
        for l in range(1, 9):
            c = stats.S['l'][l]
            b = "\u2588" * min(c, 40)
            sys.stdout.write(f"\033[1;32mLevel {l}\033[0m : [\033[1;34m{b:<40}\033[0m] {c}\n")
        sys.stdout.write("\n\033[1;33m--- Events ---\033[0m\n")
        for entry in stats.L:
            sys.stdout.write(f"{entry:<80}\n")
        sys.stdout.write("\033[J")
        sys.stdout.flush()
        await asyncio.sleep(0.5)


async def start_swarm(n, h, p):
    _spawn(n, h, p, 1)
    if not stats.D:
        asyncio.create_task(tui())
    try:
        while True:
            await asyncio.sleep(3600)
    except asyncio.CancelledError:
        pass


def main():
    a = sys.argv[1:]
    p = -1
    n = ""
    h = "127.0.0.1"
    for i in range(len(a)):
        if a[i] == "-p" and i + 1 < len(a):
            p = int(a[i + 1])
        elif a[i] == "-n" and i + 1 < len(a):
            n = a[i + 1]
        elif a[i] == "-h" and i + 1 < len(a):
            h = a[i + 1]
        elif a[i] == "--debug":
            stats.D = True

    if p < 0 or not n:
        sys.exit(84)

    try:
        asyncio.run(start_swarm(n, h, p))
    except KeyboardInterrupt:
        sys.exit(0)


if __name__ == "__main__":
    main()
