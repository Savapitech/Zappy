import sys

max_int = sys.maxsize
Queen = 1
Workers = 2
Guard = 3

size_of_char = 128

invalid_char = [['\0', chr(size_of_char + 1)], ['\n', chr(size_of_char + 2)]]

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
    for c in msg:
        char = chr((ord(c) + key) % size_of_char)
        for t in invalid_char:
            if ord(char) == ord(t[0]):
                char = t[1]
                break
        crypted_msg += char

    # update the key
    key = key + size if max_int - size > key else size - (max_int - key)

    return crypted_msg, key

def decrypt(msg: str, key: int):

    # get the size of the message
    try:
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
    decrypted_msg = ""
    for c in msg[2:]:
        for t in invalid_char:
            if c == t[1]:
                c = t[0]
        decrypted_msg += chr(((ord(c) + size_of_char) - (key % size_of_char)) % size_of_char)

    # update the key
    key = key + size if max_int - size > key else size - (max_int - key)

    return decrypted_msg, key, role
