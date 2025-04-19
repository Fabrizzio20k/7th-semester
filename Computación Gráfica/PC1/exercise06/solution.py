import numpy as np
import cv2
import random
import math

width, height = 512, 512

rad = 30
colors = [[0, 0, 255], [0, 255, 0], [255, 0, 0],
          [255, 255, 0], [0, 255, 255], [255, 0, 255]]
randPosX = [random.randint(200, 900) for _ in range(6)]
randPosY = [random.randint(200, 900) for _ in range(6)]
randVelX = [1 if random.randint(-8, 8) ==
            0 else random.randint(-8, 8) for _ in range(6)]
randVelY = [1 if random.randint(-8, 8) ==
            0 else random.randint(-8, 8) for _ in range(6)]


while True:
    canvas = np.zeros((height, width, 3), dtype=np.uint8)

    for i in range(6):
        for j in range(i+1, 6):
            dx = randPosX[i] - randPosX[j]
            dy = randPosY[i] - randPosY[j]
            dist = math.sqrt(dx*dx + dy*dy)

            if dist < 2 * rad:
                randVelX[i], randVelX[j] = randVelX[j], randVelX[i]
                randVelY[i], randVelY[j] = randVelY[j], randVelY[i]

                if dist > 0:
                    nx = dx / dist
                    ny = dy / dist

                    c = (2 * rad - dist) / 2
                    randPosX[i] += nx * c
                    randPosY[i] += ny * c
                    randPosX[j] -= nx * c
                    randPosY[j] -= ny * c

    for i in range(6):
        x = randPosX[i]
        y = randPosY[i]
        color = colors[i % len(colors)]
        vx = randVelX[i]
        vy = randVelY[i]

        x += vx
        y += vy

        if x - rad < 0 or x + rad > width:
            randVelX[i] = -randVelX[i]
            if x - rad < 0:
                x = rad
            if x + rad > width:
                x = width - rad

        if y - rad < 0 or y + rad > height:
            randVelY[i] = -randVelY[i]
            if y - rad < 0:
                y = rad
            if y + rad > height:
                y = height - rad

        randPosX[i] = x
        randPosY[i] = y

        cv2.circle(canvas, (int(x), int(y)), rad, color, -1)

    cv2.imshow('Círculos en Movimiento', canvas)

    if cv2.waitKey(30) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
