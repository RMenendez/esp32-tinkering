# Bluetooth LE connector
# Connects to a specific Bluetooth LE device
# Tested OK

import asyncio
import sys
from bleak import BleakClient

async def main():
    ble_address = sys.argv[1]

    async with BleakClient(ble_address) as client:
        # we’ll do the read/write operations here
        print("Connected to BLE device")
        print(client.is_connected)        

asyncio.run(main())