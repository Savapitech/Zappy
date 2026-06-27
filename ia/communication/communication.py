import sys
import base64

max_int = sys.maxsize
Queen = 1
Workers = 2
Guard = 3

def crypt(role: int, msg: str, key: int):

    # get the len of the message
    size = len(msg)

    # get the role of the sender
    tag = ''
    if role == Queen:
        tag = 'Q'
    elif role == Workers:
        tag = 'W'
    else:
        tag = 'G'

    # get the crypted message
    crypted_msg = chr(size) + tag
    crypted_msg += ''.join(chr(ord(a) ^ key) for a in msg)
    crypted_msg = base64.b64encode(crypted_msg.encode("utf-8")).decode("utf-8")

    # update the key
    key = key + size if max_int - size > key else size - (max_int - key)

    return crypted_msg, key

def decrypt(msg: str, key: int):

    # get the size of the message
    try:
        msg = base64.b64decode(msg.encode("utf-8")).decode("utf-8")
        size = ord(msg[0])
    except:
        return "", key, 0

    # verify the size of the message
    if len(msg) - 2 != size:
        return "", key, 0

    # get the role of the sender
    role = 0
    if msg[1] == 'Q':
        role = Queen
    elif msg[1] == 'W':
        role = Workers
    else:
        role = Guard

    # get the decrypted message
    decrypted_msg = ''.join(chr(ord(a) ^ key) for a in msg[2:])

    # update the key
    key = key + size if max_int - size > key else size - (max_int - key)

    return decrypted_msg, key, role
