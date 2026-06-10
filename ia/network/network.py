import asyncio

# Queue function that allows reading from stream to be nonblocking.
# It stops once it reaches EOF.
# It also shuts down the queue to notify processes that depend on it.
async def read_stream(reader: asyncio.StreamReader, queue: asyncio.Queue):
    while True:
        # wait for data
        data = await reader.read(4096)
        # if eof is reached
        if not data:
            break
        # push read data to the queue
        await queue.put(data.decode("utf-8"))
    # Once loop is broken, shut down the queue
    queue.shutdown(immediate=True)

# Writer function that takes input from the queue and writes it to the stream.
# If it recieves empty input, it shuts down the queue and stops.
async def write_stream(writer: asyncio.StreamWriter, queue: asyncio.Queue):
    while True:
        # Wait until data is available in the queue
        data: str = await queue.get()
        
        # If data is empty (simulate eof), shut down the queue and stop
        if not data:
            queue.shutdown(immediate=True)
            return
        
        # Write data to stream
        writer.write(data.encode("utf-8"))
        
        # Wait for the buffer to flush (confirm that the write was succesful)
        await writer.drain()

class network:
    def __init__(self, port: int, team: str, machine: str):
        self.machine = machine
        self.port = port
        self.reader = None
        self.writer = None
        self.oqueue = asyncio.Queue()
        self.iqueue = asyncio.Queue()
        self.reader_task = None
        self.writer_task = None
        self.team = team
        self.up = False

    async def connect(self):
        # Open connection and create stream objects. Can except in case connection fails.
        self.reader, self.writer = await asyncio.open_connection(self.machine, self.port)

        # Set connection indicator 
        self.up = True

        # Instantiate reader task
        self.reader_task = asyncio.create_task(
            read_stream(
                self.reader,
                self.oqueue
            )
        )

        # Instantiate writer task
        self.writer_task = asyncio.create_task(
            write_stream(
                self.writer,
                self.iqueue
            )
        )

    async def disconnect(self):
        # Close connection
        self.writer.close()

        # Cancel reader and writer task if they are still running
        for task in (self.reader_task, self.writer_task):
            task.cancel()
            # This must be wrapped in a try block as it raises an exception if it cancelled (even if is its intended behavior)
            try:
                await task
            except asyncio.CancelledError:
                pass

        # Shutdown both input and output queues
        self.oqueue.shutdown(immediate=True)
        self.iqueue.shutdown(immediate=True)

        # Set connection indicator to false
        self.up = False

    def send(self, msg: str):
        # Do nothing in case connection is closed
        if not self.up:
            return
        try:
            # Add element to queue
            self.iqueue.put_nowait(msg)

        # In case of something happening on the writer task, queue is shut down.
        # This sets the connection indicator to false, and will prevent further I/O operations.
        except asyncio.QueueShutDown:
            self.up = False

    def read(self):
        # Do nothing in case connection is closed
        if not self.up:
            return
        
        # Try to read from queue
        try:
            # Try to get one element from queue
            return self.oqueue.get_nowait()
        
        # In case of something happening on the reader task, queue is shut down.
        # This sets the connection indicator to false, and will prevent further I/O operations.
        except asyncio.QueueShutDown:
            self.up = False

        # In case no elements are present in the queue.
        # It is done that way to ensure that if the stream is empty, the QueueShutdown state can be detected
        except asyncio.QueueEmpty:
            pass

        # If no elements are present in the queue (or queue is shutdown), fallback to empty str
        return ""

async def connect(port: int, team: str, machine: str):
    # Instantiate object
    connection = network(port, team, machine)
    # Wrap the connection in a try block as it can except
    try:
        # Wait for connection
        await connection.connect()
    except Exception:
        # In case anything happens, the client cannot be launched (dependant on connection), and it raises an error
        raise(Exception("Connection to the server failed."))
    
    # Return connection object after it is initialised
    return connection