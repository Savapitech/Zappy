import base64
import hashlib
import hmac
import os
import struct
import time

_DERIVE_INFO = b"zappy-broadcast-v1"
_NONCE_LEN = 16
_TAG_LEN = 16
_FRESHNESS_WINDOW = 30.0
_SEP = "\x00"


def derive_key(team):
    return hmac.new(team.encode(), _DERIVE_INFO, hashlib.sha256).digest()


def _keystream(key, nonce, length):
    out = bytearray()
    counter = 0
    while len(out) < length:
        out += hmac.new(key, nonce + struct.pack(">I", counter), hashlib.sha256).digest()
        counter += 1
    return bytes(out[:length])


def seal(key, sender_id, counter, payload):
    nonce = os.urandom(_NONCE_LEN)
    head = struct.pack(">d", time.time())
    body = _SEP.join([str(sender_id), str(counter), payload]).encode()
    plaintext = head + body
    ciphertext = bytes(a ^ b for a, b in zip(plaintext, _keystream(key, nonce, len(plaintext))))
    tag = hmac.new(key, nonce + ciphertext, hashlib.sha256).digest()[:_TAG_LEN]
    return base64.b64encode(nonce + ciphertext + tag).decode()


class Verifier:
    def __init__(self, key, window=_FRESHNESS_WINDOW):
        self.key = key
        self.window = window
        self.last_counter = {}
        self.seen_nonces = {}

    def _gc(self, now):
        stale = [n for n, t in self.seen_nonces.items() if now - t > self.window]
        for n in stale:
            del self.seen_nonces[n]

    def open(self, text):
        try:
            raw = base64.b64decode(text, validate=True)
        except Exception:
            return None
        if len(raw) < _NONCE_LEN + _TAG_LEN + struct.calcsize(">d"):
            return None
        nonce = raw[:_NONCE_LEN]
        ciphertext = raw[_NONCE_LEN:-_TAG_LEN]
        tag = raw[-_TAG_LEN:]
        expected = hmac.new(self.key, nonce + ciphertext, hashlib.sha256).digest()[:_TAG_LEN]
        if not hmac.compare_digest(tag, expected):
            return None
        plaintext = bytes(a ^ b for a, b in zip(ciphertext, _keystream(self.key, nonce, len(ciphertext))))
        try:
            sent_at = struct.unpack(">d", plaintext[:8])[0]
            sender_id, counter_str, payload = plaintext[8:].decode().split(_SEP, 2)
            counter = int(counter_str)
        except Exception:
            return None
        now = time.time()
        if abs(now - sent_at) > self.window:
            return None
        self._gc(now)
        if nonce in self.seen_nonces:
            return None
        if counter <= self.last_counter.get(sender_id, -1):
            return None
        self.last_counter[sender_id] = counter
        self.seen_nonces[nonce] = now
        return sender_id, payload
