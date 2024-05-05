from PIL import Image

img_data = Image.open("sharpie_lines.png")
img_data = img_data.convert('RGB')
img_data.show()

import matplotlib.pyplot as plt
width, height = img_data.size
print(str(width)+" "+str(height))

index = []
reds = []
greens = []
blues = []
sum = []
for i in range(width):
    r, g, b = img_data.getpixel((i, 45))
    index.append(i)
    reds.append(r)
    greens.append(g)
    blues.append(b)
    sum.append(r+g+b)

plt.plot(index,reds,'r-', index,greens,'g-', index,blues,'b-',index,sum,'k-')
plt.show()