# import cv2
# import numpy as np


# def changeBrightness(img_route, brightness):
#     img = cv2.imread(img_route)
#     if img is None:
#         raise ValueError("Image not found or unable to read the image.")

#     img = img.astype(np.float32)
#     img += brightness

#     img = np.clip(img, 0, 255)
#     img = img.astype(np.uint8)

#     return img


# def changeContrast(img_route, new_M, new_m):
#     img = cv2.imread(img_route)
#     if img is None:
#         raise ValueError("Image not found or unable to read the image.")

#     img = img.astype(np.float32)

#     min_val = np.min(img)
#     max_val = np.max(img)
#     img = new_m + ((img-min_val)/(max_val-min_val))*(new_M-new_m)

#     img = np.clip(img, 0, 255)
#     img = img.astype(np.uint8)
#     return img


# img = "prueba.jpeg"
# brightness = 100
# img = changeContrast(img, 200, 50)


# cv2.imshow("Brightness", img)
# cv2.waitKey(0)
# cv2.destroyAllWindows()


import cv2
import numpy as np


def applyAdjustments(img, brightness, contrast_M, contrast_m):
    img = img.astype(np.float32)

    # Ajuste de brillo
    img += brightness

    # Ajuste de contraste
    min_val = np.min(img)
    max_val = np.max(img)
    if max_val - min_val != 0:
        img = contrast_m + ((img - min_val) / (max_val -
                            min_val)) * (contrast_M - contrast_m)

    img = np.clip(img, 0, 255)
    return img.astype(np.uint8)


def nothing(x):
    pass


img_path = 'prueba.jpeg'
original = cv2.imread(img_path)
if original is None:
    raise ValueError("No se pudo leer la imagen.")

cv2.namedWindow('Editor')

cv2.createTrackbar('Brillo', 'Editor', 100, 200, nothing)
cv2.createTrackbar('Contraste M', 'Editor', 200,
                   400, nothing)
cv2.createTrackbar('Contraste m', 'Editor', 50, 255,
                   nothing)

while True:
    brillo = cv2.getTrackbarPos('Brillo', 'Editor') - 100
    M = cv2.getTrackbarPos('Contraste M', 'Editor')
    m = cv2.getTrackbarPos('Contraste m', 'Editor')

    adjusted = applyAdjustments(original.copy(), brillo, M, m)

    cv2.imshow('Editor', adjusted)

    if cv2.waitKey(1) & 0xFF == 27:
        break

cv2.destroyAllWindows()
