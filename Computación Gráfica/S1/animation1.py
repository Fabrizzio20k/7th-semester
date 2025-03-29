import numpy as np
import cv2

n = 500
r = 50


def drawCircle(y, x = 0):
    """Dibuja un círculo en la posición vertical 'y'."""
    img = np.ones((n, n, 3), dtype=np.uint8) * 255
    center = (n // 2 + x, y)
    blue = (255, 0, 0)
    cv2.circle(img, center, r, blue, -1)
    return img


y = r
dy = 10

while True:
    img = drawCircle(y)
    cv2.imshow("Circle Animation", img)

    y += dy
    if y + r >= n or y - r <= 0:
        dy = -dy

    if cv2.waitKey(10) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
