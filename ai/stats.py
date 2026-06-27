S = {
    "t": 0,
    "l": {1: 0, 2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0, 8: 0}
}
L = []
D = False

def add_bot():
    S["t"] += 1
    S["l"][1] += 1

def rm_bot(l):
    S["t"] -= 1
    S["l"][l] -= 1

def lvl_up(o, n):
    S["l"][o] -= 1
    S["l"][n] += 1

def log_evt(m):
    L.append(m)
    if len(L) > 10:
        L.pop(0)
