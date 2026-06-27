STATS = {
    "total": 0,
    "levels": {1: 0, 2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0, 8: 0}
}
EVENTS = []
DEBUG = False

def add_bot():
    STATS["total"] += 1
    STATS["levels"][1] += 1

def rm_bot(level):
    STATS["total"] -= 1
    STATS["levels"][level] -= 1

def lvl_up(old_level, new_level):
    STATS["levels"][old_level] -= 1
    STATS["levels"][new_level] += 1

def log_evt(message):
    EVENTS.append(message)
    if len(EVENTS) > 10:
        EVENTS.pop(0)
