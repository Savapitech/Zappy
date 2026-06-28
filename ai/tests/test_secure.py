"""The secure broadcast layer: a sealed message round-trips for its own team,
stays opaque/unforgeable to others, and cannot be replayed."""

import secure


def test_same_team_round_trips():
    key = secure.derive_key("team1")
    v = secure.Verifier(key)
    sealed = secure.seal(key, "42", 1, "R_3")
    assert v.open(sealed) == ("42", "R_3")


def test_derived_key_is_deterministic_per_team():
    assert secure.derive_key("team1") == secure.derive_key("team1")
    assert secure.derive_key("team1") != secure.derive_key("team2")


def test_sealed_text_hides_the_payload():
    key = secure.derive_key("team1")
    sealed = secure.seal(key, "42", 1, "R_3")
    assert "R_3" not in sealed
    assert "42" not in sealed


def test_foreign_team_cannot_open():
    sealed = secure.seal(secure.derive_key("team1"), "42", 1, "R_3")
    v = secure.Verifier(secure.derive_key("rival"))
    assert v.open(sealed) is None


def test_tampered_ciphertext_is_rejected():
    import base64

    key = secure.derive_key("team1")
    sealed = secure.seal(key, "42", 1, "R_3")
    raw = bytearray(base64.b64decode(sealed))
    raw[secure._NONCE_LEN] ^= 0x01
    forged = base64.b64encode(bytes(raw)).decode()
    assert secure.Verifier(key).open(forged) is None


def test_garbage_input_is_rejected():
    v = secure.Verifier(secure.derive_key("team1"))
    assert v.open("not base64 !!!") is None
    assert v.open("") is None


def test_verbatim_replay_is_rejected():
    key = secure.derive_key("team1")
    v = secure.Verifier(key)
    sealed = secure.seal(key, "42", 5, "R_3")
    assert v.open(sealed) == ("42", "R_3")
    assert v.open(sealed) is None


def test_older_or_equal_counter_from_same_sender_is_rejected():
    key = secure.derive_key("team1")
    v = secure.Verifier(key)
    assert v.open(secure.seal(key, "42", 10, "R_3")) is not None
    assert v.open(secure.seal(key, "42", 10, "R_3")) is None
    assert v.open(secure.seal(key, "42", 9, "R_3")) is None
    assert v.open(secure.seal(key, "42", 11, "R_3")) is not None


def test_counters_are_tracked_per_sender():
    key = secure.derive_key("team1")
    v = secure.Verifier(key)
    assert v.open(secure.seal(key, "alice", 100, "R_3")) is not None
    assert v.open(secure.seal(key, "bob", 1, "R_3")) is not None


def test_stale_message_is_rejected():
    key = secure.derive_key("team1")
    v = secure.Verifier(key)
    real_time = secure.time.time
    secure.time.time = lambda: real_time() - 120
    try:
        sealed = secure.seal(key, "42", 1, "R_3")
    finally:
        secure.time.time = real_time
    assert v.open(sealed) is None
