def find_and_take(ai, resource, tries=80, turn_every=7):
    """Walk a sweeping path (turning periodically) until `resource` is
    picked up. Returns True on success. A straight line can miss a sparse
    resource entirely on a small map; turning covers far more tiles."""
    for i in range(tries):
        ai.send(f"Take {resource}")
        if ai.read_line() == "ok":
            return True
        if i % turn_every == turn_every - 1:
            ai.send("Right")
        else:
            ai.send("Forward")
        ai.read_line()
    return False
