# Bluetooth LE scanner
# Prints the name and address of every nearby Bluetooth LE device
# Tested OK

import asyncio
from bleak import BleakScanner

async def main():
    devices = await BleakScanner.discover()

    for device in devices:
        print(device)

asyncio.run(main())

# Example results
# 14:9D:01:4C:75:89: DMRRBA-007
# 5B:4B:AF:7D:09:50: None
# EC:81:93:67:8E:0E: None
