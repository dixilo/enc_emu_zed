#!/usr/bin/env python3

import serial
import time
import sys

# 送信したいバイト列
data = bytes([0x55, 0x00, 0x00, 0x00, 0x00])

# シリアルポートを開く
ser = serial.Serial(
    port='/dev/cu.usbserial-210351B7C04D1',        # Windows例。Linuxなら '/dev/ttyUSB0' など
    baudrate=115200,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=1
)

time.sleep(0.1)  # デバイスによっては開始待ちが必要


ser.write(b'5\n')
for _i in range(10):
    lines = ser.readlines()

    if (lines[-1].decode().strip() == 'PLEASE PUT UART STRING:'):
        ser.write(data)
        print("transmission complete:", data.hex())

        ser.close()
        sys.exit(0)

    ser.write(b'5\n')

print("Failed to transmit data.")
ser.close()
