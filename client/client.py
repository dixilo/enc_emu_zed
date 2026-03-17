#!/usr/bin/env python3

import serial
import time
import sys

PORT = '/dev/cu.usbserial-210351B7C04D1'
BAUD = 115200
DATA = bytes([0x55, 0x00, 0x00, 0x00, 0x02])

PROMPT_PAYLOAD = b'PLEASE PUT 5 BYTES.'
PROMPT_MENU = b'PLEASE SELECT MENU NUM:'

def read_until_marker(ser, marker, max_wait=2.0):
    buf = b''
    deadline = time.time() + max_wait

    while time.time() < deadline:
        chunk = ser.read(256)
        if chunk:
            buf += chunk
            if marker in buf:
                return buf

    return buf

def main():
    try:
        with serial.Serial(
            port=PORT,
            baudrate=BAUD,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.05
        ) as ser:
            time.sleep(0.1)
            ser.reset_input_buffer()
            ser.reset_output_buffer()

            # 1. メニュー選択
            ser.write(b'5\n')
            ser.flush()

            # 2. ペイロード入力待ちプロンプトを待つ
            rx1 = read_until_marker(ser, PROMPT_PAYLOAD, max_wait=2.0)
            print("RX before payload:", rx1.decode(errors='replace'))

            if PROMPT_PAYLOAD not in rx1:
                print("payload prompt not found")
                sys.exit(1)

            # 3. 5バイト + 改行 を送る
            ser.write(DATA + b'\n')
            ser.flush()
            print("TX:", (DATA + b'\n').hex())

            # 4. 次のメニュープロンプトまで読む
            rx2 = read_until_marker(ser, PROMPT_MENU, max_wait=2.0)
            print("RX raw:", rx2)
            print("RX text:", rx2.decode(errors='replace'))

    except serial.SerialException as e:
        print(f"serial error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()
