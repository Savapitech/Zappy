#!/usr/bin/env python3
import unittest
from communication.communication import *

class TestCommunication(unittest.TestCase):

    def test_crypt(self):
        crypted_msg, key = crypt(Queen, "hello world", 1)
        self.assertEqual(crypted_msg, chr(11) + "Qifmmp!xpsme")
        self.assertEqual(key, 12)

    def test_decrypt(self):
        decrypted_msg, key, role = decrypt(chr(13) + "Qifmmp!xpsme!b", 1)
        self.assertEqual(decrypted_msg, "hello world a")
        self.assertEqual(key, 14)
        self.assertEqual(role, Queen)
        decrypted_msg, key, role = decrypt(chr(11) + "Wifmmp!xpsme", 1)
        self.assertEqual(role, Workers)
        decrypted_msg, key, role = decrypt(chr(11) + "Gifmmp!xpsme", 1)
        self.assertEqual(role, Guard)

    def test_sensible_case(self):
        crypted_msg, key = crypt(Queen, "hello world", 27)
        self.assertEqual(crypted_msg, chr(11) + "Q" + chr(3) + chr(129) + chr(7) + chr(7) + chr(130) + chr(59) + chr(18) + chr(130) + chr(13) + chr(7) + chr(127))
        self.assertEqual(key, 38)
        decrypted_msg, key, role = decrypt(crypted_msg, 27)
        self.assertEqual(decrypted_msg, "hello world")
        self.assertEqual(key, 38)
        self.assertEqual(role, Queen)

if __name__ == '__main__':
    unittest.main()