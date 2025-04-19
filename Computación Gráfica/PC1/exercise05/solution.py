import cv2
import numpy as np
import os
import math


def retImageMode(imagen_path, mode="normal"):
    img = cv2.imread(imagen_path)
    if mode == "normal":
        img = img
    elif mode == "grayscale":
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    else:
        raise ValueError("Invalid mode. Use 'normal' or 'grayscale'.")

    return img


def saveImage(folderName, nameFile, img):
    folderName = "output/" + folderName
    if not os.path.exists(folderName):
        os.makedirs(folderName)
    cv2.imwrite(os.path.join(folderName, nameFile), img)
    print(f"Image saved as {os.path.join(folderName, nameFile)}")


def convolucion(img, kernel):
    if len(img.shape) == 2:
        h, w = img.shape
        kernel_h, kernel_w = kernel.shape

        pad_h = kernel_h // 2
        pad_w = kernel_w // 2

        padded_img = np.pad(img, ((pad_h, pad_h), (pad_w, pad_w)), 'edge')

        out_img = np.zeros((h, w), np.float32)

        for y in range(h):
            for x in range(w):
                region = padded_img[y:y+kernel_h, x:x+kernel_w]
                out_img[y, x] = np.sum(region * kernel)

        out_img = cv2.normalize(out_img, None, 0, 255, cv2.NORM_MINMAX)
        out_img = np.uint8(out_img)

    else:
        h, w = img.shape[:2]
        kernel_h, kernel_w = kernel.shape

        pad_h = kernel_h // 2
        pad_w = kernel_w // 2

        padded_img = np.pad(
            img, ((pad_h, pad_h), (pad_w, pad_w), (0, 0)), 'edge')

        out_img = np.zeros((h, w, 3), np.float32)

        for y in range(h):
            for x in range(w):
                for c in range(3):
                    region = padded_img[y:y+kernel_h, x:x+kernel_w, c]
                    out_img[y, x, c] = np.sum(region * kernel)

        out_img = cv2.normalize(out_img, None, 0, 255, cv2.NORM_MINMAX)
        out_img = np.uint8(out_img)

    return out_img


def boxFilter(imagen_path, kernelSize, mode="normal"):
    img = retImageMode(imagen_path, mode)

    kernel = np.ones((kernelSize, kernelSize), np.float32) / \
        (kernelSize * kernelSize)

    outImg = convolucion(img, kernel)
    nameFile = "box" + mode.capitalize() + str(kernelSize) + ".png"
    saveImage("boxFilter", nameFile, outImg)


def barletFilter(imagen_path, kernelSize, mode="normal"):
    img = retImageMode(imagen_path, mode)

    if kernelSize == 3:
        kernel_1d = np.array([1/2, 1, 1/2])
    elif kernelSize == 5:
        kernel_1d = np.array([1/3, 2/3, 1, 2/3, 1/3])
    else:
        medio = kernelSize // 2
        kernel_1d = np.zeros(kernelSize)

        for i in range(kernelSize):
            kernel_1d[i] = (medio + 1 - abs(i - medio))

        kernel_1d = kernel_1d / kernel_1d.sum()

    kernel_2d = np.outer(kernel_1d, kernel_1d)

    kernel_2d = kernel_2d / kernel_2d.sum()

    outImg = convolucion(img, kernel_2d)

    nameFile = "barlet" + mode.capitalize() + str(kernelSize) + ".png"
    saveImage("barletFilter", nameFile, outImg)


def gaussianFilter(imagen_path, kernelSize, mode="normal"):
    img = retImageMode(imagen_path, mode)

    if kernelSize == 3:
        kernel_2d = np.array([[1, 2, 1],
                              [2, 4, 2],
                              [1, 2, 1]], dtype=np.float32) / 16.0
    elif kernelSize == 5:
        kernel_2d = np.array([[1, 4, 6, 4, 1],
                              [4, 16, 24, 16, 4],
                              [6, 24, 36, 24, 6],
                              [4, 16, 24, 16, 4],
                              [1, 4, 6, 4, 1]], dtype=np.float32) / 256.0
    else:
        n = kernelSize - 1
        kernel_1d = np.zeros(kernelSize, dtype=np.float32)
        for i in range(kernelSize):
            kernel_1d[i] = math.comb(n, i)
        kernel_2d = np.outer(kernel_1d, kernel_1d)
        kernel_2d = kernel_2d / np.sum(kernel_2d)

    outImg = convolucion(img, kernel_2d)
    nameFile = "gaussian" + mode.capitalize() + str(kernelSize) + ".png"
    saveImage("gaussianFilter", nameFile, outImg)


def laplacianFilter(imagen_path, kernelSize, mode="normal"):
    img = retImageMode(imagen_path, mode)
    if kernelSize == 3:
        kernel_2d = np.array([[0, 1, 0],
                              [1, -4, 1],
                              [0, 1, 0]], dtype=np.float32)
    elif kernelSize == 5:
        kernel_2d = np.array([[0, 0, 1, 0, 0],
                              [0, 1, 2, 1, 0],
                              [1, 2, -17, 2, 1],
                              [0, 1, 2, 1, 0],
                              [0, 0, 1, 0, 0]], dtype=np.float32)

    else:
        raise ValueError("Invalid kernel size. Use 3 or 5.")

    outImg = convolucion(img, kernel_2d)
    nameFile = "laplacian" + mode.capitalize() + str(kernelSize) + ".png"
    saveImage("laplacianFilter", nameFile, outImg)


for i in range(3, 26, 2):
    boxFilter("lenna.png", i, "normal")
    boxFilter("lenna.png", i, "grayscale")
    barletFilter("lenna.png", i, "normal")
    barletFilter("lenna.png", i, "grayscale")
    gaussianFilter("lenna.png", i, "normal")
    gaussianFilter("lenna.png", i, "grayscale")
    laplacianFilter("lenna.png", i, "normal")
    laplacianFilter("lenna.png", i, "grayscale")
