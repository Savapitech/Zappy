import logging
import asyncio

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

# Queue function that allows reading from stream to be nonblocking.
# It stops once it reaches EOF.
# It also shuts down the queue to notify processes that depend on it.
async def readStream(reader: asyncio.StreamReader, queue: asyncio.Queue):
    logger.info("Reader started.")
    while True:
        # wait for data
        data = await reader.read(4096)
        logger.debug("Recieved data : <%s>", data)
        # if eof is reached
        if not data:
            break
        # push read data to the queue
        await queue.put(data.decode("utf-8"))

    logger.info("Reader stopped.")

    # Once loop is broken, shut down the queue
    queue.shutdown(immediate=True)

# Writer function that takes input from the queue and writes it to the stream.
# If it recieves empty input, it shuts down the queue and stops.
async def writeStream(writer: asyncio.StreamWriter, queue: asyncio.Queue):
    logger.info("Writer started.")
    while True:
        # Wait until data is available in the queue
        data: str = await queue.get()
        
        # If data is empty (simulate eof), shut down the queue and stop
        if not data:
            break
        
        # Write data to stream
        writer.write(data.encode("utf-8"))
        
        # Wait for the buffer to flush (confirm that the write was succesful)
        await writer.drain()
        
        logger.debug("Sent data : <%s>", data)

    logger.info("Writer stopped.")

    # Once loop is broken, shut down the queue
    queue.shutdown(immediate=True)

class network:
    def __init__(self, port: int, machine: str, tg: asyncio.TaskGroup):
        self.machine = machine
        self.port = port
        self.reader = None
        self.writer = None
        self.iQueue = asyncio.Queue()
        self.oQueue = asyncio.Queue()
        self.taskGroup = tg
        self.readerTask = None
        self.writerTask = None
        self.up = False

    async def connect(self):
        # Open connection and create stream objects. Can except in case connection fails.
        self.reader, self.writer = await asyncio.open_connection(self.machine, self.port)

        logger.info("Connection opened.")

        # Set connection indicator 
        self.up = True

        # Instantiate reader task
        self.readerTask = asyncio.create_task(
            readStream(
                self.reader,
                self.iQueue
            )
        )

        # Instantiate writer task
        self.writerTask = asyncio.create_task(
            writeStream(
                self.writer,
                self.oQueue
            )
        )

    async def disconnect(self):
        # Close connection
        self.writer.close()

        logger.info("Connection closed.")

        # Cancel reader and writer task if they are still running
        for task in (self.readerTask, self.writerTask):
            task.cancel()
            # This must be wrapped in a try block as it raises an exception if it cancelled (even if is its intended behavior)
            try:
                await task
            except asyncio.CancelledError:
                logger.info("%s stopped.", task.get_name())
                pass

        # Shutdown both input and output queues
        self.iQueue.shutdown(immediate=True)
        self.oQueue.shutdown(immediate=True)

        # Set connection indicator to false
        self.up = False

    def sendNoWait(self, msg: str):
        # Do nothing in case connection is closed
        if not self.up:
            logger.error("Cannot send : connection is not open.")
            return
        try:
            # Add element to queue
            self.oQueue.put_nowait(msg)

        # This should not happen as queues are initialised without a limit.
        except asyncio.QueueFull:
            logger.error("Cannot send : queue is full.")
            pass
        # In case of something happening on the writer task, queue is shut down.
        # This sets the connection indicator to false, and will prevent further I/O operations.
        except asyncio.QueueShutDown:
            logger.error("Cannot send : connection is shutdown.")
            self.up = False
    
    async def send(self, msg: str):
        # Do nothing in case connection is closed
        if not self.up:
            logger.error("Cannot send : connection is not open.")
            return
        try:
            # Add element to queue
            await self.oQueue.put(msg)

        # In case of something happening on the writer task, queue is shut down.
        # This sets the connection indicator to false, and will prevent further I/O operations.
        except asyncio.QueueShutDown:
            logger.error("Cannot send : connection is shutdown.")
            self.up = False

    async def sendTimeout(self, msg:str, timeout: float):
        # Do nothing in case connection is closed
        if not self.up:
            logger.error("Cannot send : connection is not open.")
            return
        
        # Try to read from queue
        try:
            # Add element to queue with set timeout
            return await asyncio.wait_for(self.oQueue.put(msg), timeout=timeout)
        
        # If no room is available for a new element, nothing is done.
        except asyncio.TimeoutError:
            logger.error("Cannot send : timed out.")
            pass
        # In case of something happening on the writer task, queue is shut down.
        # This sets the connection indicator to false, and will prevent further I/O operations.
        except asyncio.QueueShutDown:
            logger.error("Cannot send : connection is shutdown.")
            self.up = False

    def readNoWait(self):
        # Do nothing in case connection is closed
        if not self.up:
            logger.error("Cannot read : connection is not open.")
            return
        
        # Try to read from queue
        try:
            # Try to get one element from queue
            return self.iQueue.get_nowait()
        
        # In case no elements are present in the queue.
        # This is an expected, frequent case while polling, not an error.
        # It is done that way to ensure that if the stream is empty, the QueueShutdown state can be detected.
        except asyncio.QueueEmpty:
            logger.debug("Cannot read : queue is empty.")
            pass

        # In case of something happening on the reader task, queue is shut down.
        # This sets the connection indicator to false, and will prevent further I/O operations.
        except asyncio.QueueShutDown:
            logger.error("Cannot read : connection is shutdown.")
            self.up = False

        # If no elements are present in the queue (or queue is shutdown), fallback to empty str
        return ""

    async def read(self):
        # Do nothing in case connection is closed
        if not self.up:
            logger.error("Cannot read : connection is not open.")
            return
        
        # Try to read from queue
        try:
            # Try to get one element from queue
            return await self.iQueue.get()
        
        # In case of something happening on the reader task, queue is shut down.
        # This sets the connection indicator to false, and will prevent further I/O operations.
        except asyncio.QueueShutDown:
            logger.error("Cannot read : connection is shutdown.")
            self.up = False

    async def readTimeout(self, timeout: float):
        # Do nothing in case connection is closed
        if not self.up:
            logger.error("Cannot read : connection is not open.")
            return
        
        # Try to read from queue
        try:
            # Try to get one element from queue with set timeout
            return await asyncio.wait_for(self.iQueue.get(), timeout=timeout)
        
        # If no element comes up in the queue during the timeout, the function returns an empty element.
        except asyncio.TimeoutError:
            logger.error("Cannot read : timed out.")
            return ""
        # In case of something happening on the reader task, queue is shut down.
        # This sets the connection indicator to false, and will prevent further I/O operations.
        except asyncio.QueueShutDown:
            logger.error("Cannot read : connection is shutdown.")
            self.up = False

async def connect(port: int, machine: str, tg: asyncio.TaskGroup):
    # Instantiate object
    connection = network(port, machine, tg)
    # Wrap the connection in a try block as it can except
    try:
        # Wait for connection
        await connection.connect()
    except Exception:
        # In case anything happens, the client cannot be launched (dependant on connection), and it raises an error
        raise(Exception("Connection to the server failed."))
    
    # Return connection object after it is initialised
    return connection