from PIL import Image, ImageDraw

w, h = 113, 1000
img = Image.new("RGB", (w, h), (0, 0, 0))
draw = ImageDraw.Draw(img)

draw.rectangle([10, 100, 30, 150], fill=(255, 0, 0))
draw.ellipse([40, 200, 70, 230], fill=(0, 255, 0))
draw.line([0, 300, w-1, 300], fill=(0, 0, 255), width=3)
draw.polygon([(80, 400), (100, 420), (80, 440)], fill=(255, 255, 0))
draw.rectangle([20, 500, 92, 520], fill=(255, 0, 255))

img.save("output.png")