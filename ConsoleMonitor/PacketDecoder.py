from time import sleep
import asyncio
import websockets
import logging
logger = logging.getLogger('websockets')
logger.setLevel(logging.INFO)
logger.addHandler(logging.StreamHandler())

async def hello(websocket, path):
    try:
        name = 'aaa'# await websocket.recv()
        print(f"< {name}")

        greeting = f"Hello {name}!\n"

        await websocket.send(greeting)
        print(f"> {greeting}\n")

        await websocket.send(f"{path}\n")
        c=1
        while True:
            await websocket.send(f"{c}\n")
            #await websocket.recv()
            c+=1
            sleep(1)
    finally:
        pass

start_server = websockets.serve(hello, "localhost", 8888)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()