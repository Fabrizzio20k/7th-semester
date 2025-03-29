import numpy as np
import cv2
import random

n = 500
r = 50


def drawCircle(y, x):
    """Dibuja un círculo en la posición dada (x, y)."""
    img = np.ones((n, n, 3), dtype=np.uint8) * 255
    center = (x, y)
    blue = (255, 0, 0)
    cv2.circle(img, center, r, blue, -1)
    return img


x, y = n // 2, n // 2 + 20

dx = random.choice([-5, -4, -3, 3, 4, 5])
dy = random.choice([-5, -4, -3, 3, 4, 5])

while True:
    img = drawCircle(y, x)
    cv2.imshow("Bouncing Ball", img)

    x += dx
    y += dy

    if x - r <= 0 or x + r >= n:
        dx = -dx

    if y - r <= 0 or y + r >= n:
        dy = -dy

    # Salir si se presiona 'q'
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
