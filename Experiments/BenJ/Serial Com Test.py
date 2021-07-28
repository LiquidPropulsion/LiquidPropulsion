import serial
import time

port = 'COM31'
baud = 115200
testDigit = str(100)
printout = testDigit.encode()

ser = serial.Serial(port, baud)
print(ser.name)

stop_time = time.time() + 10
while True:
    for j in range(0, 8):
        ser.write(printout + b',')

    ser.write(b'\n')

    if time.time() > stop_time:
        break



