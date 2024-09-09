from PIL import Image
from time import sleep
from picamera import PiCamera
import serial

ser = serial.Serial(port='/dev/ttyS0', baudrate = 115200, timeout=1)
print("Serial open")

camera = PiCamera()
camera.resolution = (100, 50)
width, height = 100, 50

while True:
    camera.capture('tmp.jpg')

    img_data = Image.open("tmp.jpg")
    img_data = img_data.convert('RGB')

    index = []
    white = []
    winds = []
    for i in range(width):
        r, g, b = img_data.getpixel((i, 5))
        index.append(i)
        white.append(r+g+b)
        if white[-1] > 400:
            winds.append(i)

    if len(winds) == 0:
        print(-1)
        ser.write((str(-1)+ '\n').encode())
    else:
        res = int((winds[0]+winds[-1])/2)
        print(res)
        ser.write((str(res)+ '\n').encode())