#!/usr/bin/env python3
import unittest
from communication.communication import *

class TestCommunication(unittest.TestCase):

    def test_crypt(self):
        crypted_msg, key = crypt(Queen, "hello world", 1)
        self.assertEqual(crypted_msg, b'C1FpZG1tbiF2bnNtZQ==')
        self.assertEqual(key, 12)

    def test_decrypt(self):
        decrypted_msg, key, role = decrypt(b'DVFpZG1tbiF2bnNtZSFg', 1)
        self.assertEqual(decrypted_msg, "hello world a")
        self.assertEqual(key, 14)
        self.assertEqual(role, Queen)
        decrypted_msg, key, role = decrypt(b'C1dpZG1tbiF2bnNtZQ==', 1)
        self.assertEqual(role, Workers)
        decrypted_msg, key, role = decrypt(b'C0dpZG1tbiF2bnNtZQ==', 1)
        self.assertEqual(role, Guard)

    def test_sensible_case(self):
        crypted_msg, key = crypt(Queen, "hello world", 27)
        self.assertEqual(crypted_msg, b'C1Fzfnd3dDtsdGl3fw==')
        self.assertEqual(key, 38)
        decrypted_msg, key, role = decrypt(crypted_msg, 27)
        self.assertEqual(decrypted_msg, "hello world")
        self.assertEqual(key, 38)
        self.assertEqual(role, Queen)

if __name__ == '__main__':
    unittest.main()