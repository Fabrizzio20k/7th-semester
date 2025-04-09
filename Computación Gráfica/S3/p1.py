import cv2
import numpy as np
import os


def applyFilter(img, kernel):
    imagen = cv2.imread(img)

    if imagen is None:
        print("Error: Could not read the image.")
        return None

    canales = cv2.split(imagen)
    canales_filtrados = [cv2.filter2D(canal, -1, kernel) for canal in canales]
    imagen_filtrada = cv2.merge(canales_filtrados)

    return imagen_filtrada


def applySpecificFilter(img, filterName, order):
    if filterName == "box":
        kernel = np.ones((order, order), np.float32) / (order * order)

    elif filterName == "barlett":
        kernel = np.zeros((order, order), np.float32)
        for i in range(order):
            for j in range(order):
                kernel[i, j] = 1 - (abs(i - (order // 2)) / (order // 2)) * \
                    (abs(j - (order // 2)) / (order // 2))
        kernel /= np.sum(kernel)

    elif filterName == "gaussian":
        kernel1D = cv2.getGaussianKernel(order, -1)
        kernel = np.outer(kernel1D, kernel1D.transpose())

    elif filterName == "laplacian":
        kernel = np.array([[0, 1, 0],
                           [1, -4, 1],
                           [0, 1, 0]], np.float32)

    elif filterName == "highpass":
        # handled separately
        return applyHighpassSubtraction(img, order)

    else:
        print("Error: Filter not recognized.")
        return None

    return applyFilter(img, kernel)


def applyHighpassSubtraction(img_path, order):
    original = cv2.imread(img_path)
    if original is None:
        print("Error: could not read the image.")
        return None

    kernel_blur = np.ones((order, order), np.float32) / (order * order)
    blurred = applyFilter(img_path, kernel_blur)

    highpass = cv2.subtract(original, blurred)
    return highpass


def enhanceEdgesInBlurredImage(img_path, blur_order=5):
    original = cv2.imread(img_path)
    if original is None:
        print("Error: could not read the image.")
        return None

    # Apply blur
    kernel_blur = np.ones((blur_order, blur_order),
                          np.float32) / (blur_order * blur_order)
    blurred = applyFilter(img_path, kernel_blur)

    # Apply sharpening filter
    sharpen_kernel = np.array([[0, -1,  0],
                               [-1, 5, -1],
                               [0, -1,  0]], np.float32)
    sharpened = cv2.filter2D(blurred, -1, sharpen_kernel)

    return sharpened


img = "lenna.png"

output_folder = "resultados"
os.makedirs(output_folder, exist_ok=True)

filtros_ordenes = {
    "box": [3, 5, 7],
    "barlett": [3, 5, 7],
    "gaussian": [3, 5, 7],
    "laplacian": [3, 5],
    "highpass": [3, 5, 7]
}

for filtro, ordenes in filtros_ordenes.items():
    for orden in ordenes:
        print(f"Aplicando filtro {filtro} de orden {orden}...")
        resultado = applySpecificFilter(img, filtro, orden)
        if resultado is not None:
            nombre_archivo = f"{output_folder}/{filtro}_{orden}.png"
            cv2.imwrite(nombre_archivo, resultado)


print("Aplicando realce de bordes en imagen borrosa...")
bordes_enfatizados = enhanceEdgesInBlurredImage(img, blur_order=5)
if bordes_enfatizados is not None:
    cv2.imwrite(f"{output_folder}/realce_bordes.png", bordes_enfatizados)

print("¡Listo! Todas las imágenes fueron procesadas y guardadas.")
