import cv2
import numpy as np


def changeScale(img_route, target_color):
    img = cv2.imread(img_route)
    if img is None:
        raise ValueError("Image not found or unable to read the image.")

    img = img.astype(np.float32)
    tc = np.array(target_color, dtype=np.float32)
    max_tc = np.max(tc)

    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            img[i, j] = max(img[i, j])/max_tc * tc

    img = np.clip(img, 0, 255)
    img = img.astype(np.uint8)
    return img


def changeScaleOtherImage(img_route1, img_route2):
    img1 = cv2.imread(img_route1)
    if img1 is None:
        raise ValueError("Image not found or unable to read the image.")
    img2 = cv2.imread(img_route2)
    if img2 is None:
        raise ValueError("Image not found or unable to read the image.")

    img1 = img1.astype(np.float32)
    img2 = img2.astype(np.float32)

    for i in range(img1.shape[0]):
        for j in range(img1.shape[1]):
            img1[i, j] = max(img1[i, j])/max(img2[i, j]) * img2[i, j]

    img1 = np.clip(img1, 0, 255)
    img1 = img1.astype(np.uint8)
    return img1


img1 = "prueba4.jpg"
img2 = "prueba2.png"
# gray scale
target_color = [11, 14, 235]  # Red
# target_color = [255, 0, 0]  # Blue
# target_color = [0, 255, 0]  # Green
# target_color = [255, 255, 0]  # Yellow
# target_color = [255, 255, 255]  # White
# target_color = [0, 0, 0]  # Black
img = changeScaleOtherImage(img1, img2)
cv2.imshow("Brightness", img)
cv2.waitKey(0)
cv2.destroyAllWindows()
