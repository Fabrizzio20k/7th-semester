import cv2
import numpy as np

imagen = cv2.imread("prueba.webp")


def interpolacion_bilineal(img, new_width, new_height):
    height, width, channels = img.shape
    print(height, width, channels)
    new_img = np.zeros((new_height, new_width, channels), dtype=np.uint8)

    for i in range(new_height):
        for j in range(new_width):
            x = i * (height - 1) / (new_height - 1)
            y = j * (width - 1) / (new_width - 1)

            x0 = int(x)
            x1 = min(x0 + 1, height - 1)
            y0 = int(y)
            y1 = min(y0 + 1, width - 1)

            a = x - x0
            b = y - y0

            for c in range(channels):
                new_img[i, j, c] = (
                    (1 - a) * (1 - b) * img[x0, y0, c]
                    + a * (1 - b) * img[x1, y0, c]
                    + (1 - a) * b * img[x0, y1, c]
                    + a * b * img[x1, y1, c]
                )

    return new_img


# Definir el nuevo tamaño
nuevo_ancho = 1920
nuevo_alto = 1080
# Aplicar la interpolación bilineal
imagen = interpolacion_bilineal(imagen, nuevo_ancho, nuevo_alto)


cv2.imshow("Imagen", imagen)
cv2.waitKey(0)
cv2.destroyAllWindows()


def interpolacion(img, new_width, new_height):
    height, width = img.shape
    new_img = np.zeros((new_height, new_width), dtype=np.uint8)

    for i in range(new_height):
        for j in range(new_width):
            # Nuevas coordenadas
            x = i * (height - 1) / (new_height - 1)
            y = j * (width - 1) / (new_width - 1)

            
