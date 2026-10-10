# Read the current state of a characteristic on a BLE device
# Tested OK
# Returns bytearray(b'ESP32S3_LED_Ctrl')

import asyncio
import sys
from bleak import BleakClient

async def main():
    ble_address = sys.argv[1]
    # The chararacteristic UUID for the Device Name characteristic (0x2A00) in the Generic Access service (0x1800)
    characteristic_uuid = '00002a00-0000-1000-8000-00805f9b34fb'

    async with BleakClient(ble_address) as client:
        data = await client.read_gatt_char(characteristic_uuid)
        print(data)

asyncio.run(main())