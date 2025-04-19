import cv2
import numpy as np
import os


def changeColor():
    img = cv2.imread('lenna.png')
    width, height = img.shape[1], img.shape[0]

    canvas = np.ones((height, width, 3), dtype=np.uint8) * 255

    cv2.circle(canvas, (width // 2, height // 2), width//2, (255, 0, 0), -1)

    for i in range(0, width):
        for j in range(0, height):
            img[i, j] = max(img[i, j])/max(canvas[i, j])*canvas[i, j]

    if not os.path.exists('exercise04/output'):
        os.makedirs('exercise04/output')

    cv2.imwrite('exercise04/output/lenna-colorscale.png', img)


changeColor()
