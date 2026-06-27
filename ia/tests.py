#!/usr/bin/env python3
import unittest
from unittest.mock import AsyncMock, Mock, patch
from communication.communication import *
import network.network as net
import asyncio

class TestCommunication(unittest.TestCase):

    def test_crypt(self):
        crypted_msg, key = crypt(Queen, "hello world", 1)
        self.assertEqual(crypted_msg, "C1FpZG1tbiF2bnNtZQ==")
        self.assertEqual(key, 12)

    def test_decrypt(self):
        decrypted_msg, key, role = decrypt("DVFpZG1tbiF2bnNtZSFg", 1)
        self.assertEqual(decrypted_msg, "hello world a")
        self.assertEqual(key, 14)
        self.assertEqual(role, Queen)
        decrypted_msg, key, role = decrypt("C1dpZG1tbiF2bnNtZQ==", 1)
        self.assertEqual(role, Workers)
        decrypted_msg, key, role = decrypt("C0dpZG1tbiF2bnNtZQ==", 1)
        self.assertEqual(role, Guard)

    def test_sensible_case(self):
        crypted_msg, key = crypt(Queen, "hello world", 27)
        self.assertEqual(crypted_msg, "C1Fzfnd3dDtsdGl3fw==")
        self.assertEqual(key, 38)
        decrypted_msg, key, role = decrypt(crypted_msg, 27)
        self.assertEqual(decrypted_msg, "hello world")
        self.assertEqual(key, 38)
        self.assertEqual(role, Queen)

class TestNetwork(unittest.IsolatedAsyncioTestCase):

    async def test_send_puts_message_in_queue(self):
        conn = net.network(8000, "localhost", None)
        conn.up = True

        await conn.send("hello")

        msg = await conn.oQueue.get()

        self.assertEqual(msg, "hello")

    async def test_read_returns_message(self):
        conn = net.network(8000, "localhost", None)
        conn.up = True

        await conn.iQueue.put("hello")

        result = await conn.read()

        self.assertEqual(result, "hello")

    @patch("network.network.asyncio.open_connection")
    async def test_connect(self, mock_open_connection):
        reader = AsyncMock()
        reader.read.side_effect = [b""]

        writer = Mock()
        writer.drain = AsyncMock()

        mock_open_connection.return_value = (reader, writer)

        conn = net.network(8000, "localhost", None)
        conn.iQueue.shutdown = Mock()

        await conn.connect()

        self.assertTrue(conn.up)
        self.assertIs(conn.reader, reader)
        self.assertIs(conn.writer, writer)

    @patch("network.network.asyncio.open_connection")
    async def test_connect_failure(self, mock_open_connection):
        mock_open_connection.side_effect = OSError()

        conn = net.network(8000, "localhost", None)

        with self.assertRaises(OSError):
            await conn.connect()

    async def test_reads_until_eof(self):
        reader = AsyncMock()

        reader.read.side_effect = [
            b"hello",
            b"world",
            b""
        ]

        queue = asyncio.Queue()

        queue.shutdown = Mock()

        await net.readStream(reader, queue)

        self.assertEqual(await queue.get(), "hello")
        self.assertEqual(await queue.get(), "world")
        queue.shutdown.assert_called_once_with(immediate=True)

    async def test_writes_data(self):
        writer = Mock()
        writer.drain = AsyncMock()

        queue = asyncio.Queue()
        queue.shutdown = Mock()

        await queue.put("hello")
        await queue.put("")  # stop signal

        await net.writeStream(writer, queue)

        writer.write.assert_called_once_with(
            b"hello"
        )

        writer.drain.assert_awaited_once()
        queue.shutdown.assert_called_once_with(immediate=True)

    async def test_disconnect(self):
        conn = net.network(8000, "localhost", None)

        conn.up = True

        conn.writer = Mock()

        conn.readerTask = asyncio.create_task(
            asyncio.sleep(100)
        )

        conn.writerTask = asyncio.create_task(
            asyncio.sleep(100)
        )

        conn.iQueue.shutdown = Mock()
        conn.oQueue.shutdown = Mock()

        await conn.disconnect()

        conn.writer.close.assert_called_once()

        self.assertFalse(conn.up)
        self.assertTrue(conn.readerTask.cancelled())
        self.assertTrue(conn.writerTask.cancelled())
        conn.iQueue.shutdown.assert_called_once_with(immediate=True)
        conn.oQueue.shutdown.assert_called_once_with(immediate=True)

if __name__ == '__main__':
    unittest.main()