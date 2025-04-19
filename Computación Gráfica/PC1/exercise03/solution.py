import cv2
import numpy as np
import time


def changeContrast(img_route, factor=1.0):
    img = cv2.imread(img_route)
    if img is None:
        raise ValueError("Image not found or unable to read the image.")

    img = img.astype(np.float32)

    min_val = np.min(img)
    max_val = np.max(img)

    mid_val = 128

    new_m = mid_val - factor * mid_val
    new_M = mid_val + factor * (255 - mid_val)

    img = new_m + ((img-min_val)/(max_val-min_val))*(new_M-new_m)

    img = np.clip(img, 0, 255)
    img = img.astype(np.uint8)
    return img


img_path = "lowcontrast.png"

original_img = cv2.imread(img_path)
if original_img is None:
    raise ValueError(f"No se pudo abrir la imagen: {img_path}")

current_factor = 100
last_update_time = time.time()


def on_trackbar(val):
    global current_factor, last_update_time
    current_factor = val

    current_time = time.time()
    if (current_time - last_update_time) < 0.05:
        return

    last_update_time = current_time

    factor = val / 100.0

    result_img = changeContrast(img_path, factor)
    cv2.imshow("Ajuste de Contraste", result_img)


cv2.namedWindow("Ajuste de Contraste", cv2.WINDOW_NORMAL)

cv2.createTrackbar("Contraste", "Ajuste de Contraste",
                   current_factor, 200, on_trackbar)

on_trackbar(current_factor)

while True:
    key = cv2.waitKey(1) & 0xFF
    if key == 27:
        break

cv2.destroyAllWindows()
