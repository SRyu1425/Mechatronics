from time import sleep
from picamera import PiCamera

iso_settings = [100,200,400,800,1600]

camera = PiCamera(resolution=(1280, 720), framerate=30)

for i in range(5):
    camera.color_effects = (128,128) # black and white image
    # Set ISO to the desired value
    camera.iso = iso_settings[i]
    # Wait for the automatic gain control to settle
    sleep(2)
    camera.capture('iso%02d.jpg' % iso_settings[i]) # set name of image
