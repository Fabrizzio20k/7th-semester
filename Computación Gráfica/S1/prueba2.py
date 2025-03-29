import cv2
import numpy as np


def ocultar_codigo(imagen_path, codigo, salida_path):
    # Leer la imagen
    img = cv2.imread(imagen_path)
    h, w, _ = img.shape

    # Convertir código en bits
    bin_code = ''.join(format(ord(c), '08b') for c in codigo)
    bin_code += '00000000'  # Marca de fin con un byte nulo

    # Verificar si cabe en la imagen
    if len(bin_code) > h * w * 3:
        raise ValueError("El código es demasiado grande para esta imagen")

    # Insertar los bits en los píxeles
    index = 0
    for i in range(h):
        for j in range(w):
            for k in range(3):  # Canales RGB
                if index < len(bin_code):
                    img[i, j, k] = (img[i, j, k] & 0b11111110) | int(
                        bin_code[index])
                    index += 1

    # Guardar la imagen modificada
    cv2.imwrite(salida_path, img)
    print(f"Código oculto en {salida_path}")


# Uso
codigo = """#include <iostream>
using namespace std;
int main() {
    cout << "Hola, mundo!" << endl;
    return 0;
}"""
ocultar_codigo("prueba.png", codigo, "imagen_oculta.png")
