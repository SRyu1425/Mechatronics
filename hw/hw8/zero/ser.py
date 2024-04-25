import serial
ser = serial.Serial(port='/dev/ttyS0', baudrate = 115200, timeout=1)
print("Serial open")
while 1:
    x=ser.readline()
    print(x)
    if len(x) > 0:
        num = int(x)
        print(num)
        ser.write((str(num)+ '\n').encode())