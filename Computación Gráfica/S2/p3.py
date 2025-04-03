import cv2
import numpy as np


def changeGreenScreen(img_green, img_t2):
    img_green = img_green.astype(np.float32)
    img_t2 = img_t2.astype(np.float32)

    for i in range(img_green.shape[0]):
        for j in range(img_green.shape[1]):
            if img_green[i, j][0] < 100 and img_green[i, j][1] > 150 and img_green[i, j][2] < 100:
                img_green[i, j] = img_t2[i, j]
            else:
                img_green[i, j] = img_green[i, j]
    img_green = np.clip(img_green, 0, 255)
    img_green = img_green.astype(np.uint8)
    return img_green


img1 = "p1.jpg"
img2 = "p2.jpg"
img1 = cv2.imread(img1)
if img1 is None:
    raise ValueError("Image not found or unable to read the image.")
img2 = cv2.imread(img2)
if img2 is None:
    raise ValueError("Image not found or unable to read the image.")
img1 = img1.astype(np.float32)
img2 = img2.astype(np.float32)
img = changeGreenScreen(img1, img2)
cv2.imshow("Brightness", img)
cv2.waitKey(0)
cv2.destroyAllWindows()
