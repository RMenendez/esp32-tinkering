# First example of writing to a characteristic
# Tested OK

import asyncio
import sys
from bleak import BleakClient

async def main():

    ble_address = sys.argv[1]

    # The chararacteristic UUID for the Nordic UART RX Device
    # Description: Nordic UART Service
    # Handle: 14
    #   Characteristics (2):
    #   ----------------------------------------------------------------------------
    #     UUID: 6e400002-b5a3-f393-e0a9-e50e24dcca9e
    #     Description: Nordic UART RX
    #     Handle: 15
    #     Properties: write-without-response, write

    characteristic_uuid = '6e400002-b5a3-f393-e0a9-e50e24dcca9e'

    async with BleakClient(ble_address) as client:
        for iteration in range(10):
            await client.write_gatt_char(characteristic_uuid, bytearray(b'on'))
            await asyncio.sleep(0.5)
            await client.write_gatt_char(characteristic_uuid, bytearray(b'off'))
            if iteration < 9:
                await asyncio.sleep(0.5)

asyncio.run(main())