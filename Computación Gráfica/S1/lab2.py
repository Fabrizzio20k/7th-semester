import cv2
import numpy as np


def generate_chessboard(n):
    img = np.zeros((n, n, 3), dtype=np.uint8)
    squares10 = n//10
    counterHorizontal = 0
    counter = 0
    color = [0, 0, 0]

    for i in range(n):
        for j in range(n):
            if counter < squares10:
                img[i, j] = color
                counter += 1
            else:
                counter = 1
                color = [255, 255, 255] if color == [0, 0, 0] else [0, 0, 0]
                img[i, j] = color

        counterHorizontal += 1
        if counterHorizontal == squares10:
            counterHorizontal = 0
            color = [255, 255, 255] if color == [0, 0, 0] else [0, 0, 0]
            for j in range(n):
                img[i, j] = color
    return img


def drawCircleMiddle(n, r):
    img = np.zeros((n, n, 3), dtype=np.uint8)
    center = (n // 2, n // 2)
    blue = (255, 0, 0)

    for i in range(n):
        for j in range(n):
            if (i - center[0]) ** 2 + (j - center[1]) ** 2 <= r ** 2:
                img[i, j] = blue
            else:
                img[i, j] = [255, 255, 255]
    return img


cv2.imshow("checkerboard", drawCircleMiddle(100, 30))
cv2.waitKey(0)
cv2.destroyAllWindows()
