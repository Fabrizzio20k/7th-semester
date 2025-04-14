import pytest
import numpy as np
import cv2
from exercise01.solution import resize


def create_test_image(width, height, channels=1, value=255):
    """
    Crea una imagen de tamaño (width x height) llena de un valor específico.
    Si channels = 1, será en escala de grises, si es 3, será RGB.
    """
    return np.full((height, width, channels), value, dtype=np.uint8)


def test_resize_rgb():
    img = create_test_image(5, 5, channels=3, value=100)
    resized = resize(img, 10, 10, 'ZEROS')
    assert resized.shape == (
        10, 10, 3), "El tamaño de la imagen no es correcto"
    assert np.all(
        resized == 100), "La interpolación no está funcionando correctamente"


def test_resize_grayscale():
    img = create_test_image(5, 5, channels=1, value=150)
    resized = resize(img, 10, 10, 'ZEROS')
    assert resized.shape == (10, 10), "El tamaño de la imagen no es correcto"
    assert np.all(
        resized == 150), "La interpolación no está funcionando correctamente"


def test_resize_padding_zeros():
    img = create_test_image(2, 2, channels=1, value=200)
    resized = resize(img, 4, 4, 'ZEROS')
    assert resized.shape == (4, 4), "El tamaño de la imagen no es correcto"
    assert np.all(resized[0, 0] ==
                  0), "El padding con ceros no se aplicó correctamente"


def test_resize_padding_last_pixel():
    img = create_test_image(2, 2, channels=1, value=255)
    resized = resize(img, 4, 4, 'LAST_PIXEL')
    assert resized.shape == (4, 4), "El tamaño de la imagen no es correcto"
    assert np.all(
        resized[0, 0] == 255), "El padding con último pixel no se aplicó correctamente"


def test_resize_extreme_small_image():
    img = create_test_image(1, 1, channels=3, value=255)
    resized = resize(img, 3, 3, 'ZEROS')
    assert resized.shape == (3, 3, 3), "El tamaño de la imagen no es correcto"
    assert np.all(
        resized == 255), "La interpolación no se aplica correctamente a una imagen pequeña"


def test_resize_extreme_large_image():
    img = create_test_image(50, 50, channels=1, value=100)
    resized = resize(img, 1000, 1000, 'LAST_PIXEL')
    assert resized.shape == (
        1000, 1000), "El tamaño de la imagen no es correcto"
    assert np.all(
        resized[0, 0] == 100), "La interpolación con último pixel no está funcionando correctamente"


def test_resize_grayscale_to_rgb():
    img = create_test_image(3, 3, channels=1, value=75)
    resized = resize(img, 6, 6, 'ZEROS')
    assert resized.shape == (6, 6), "El tamaño de la imagen no es correcto"
    assert np.all(
        resized == 75), "La interpolación no está funcionando correctamente"


def test_resize_with_padding_on_edges():
    img = np.array([[1, 2], [3, 4]], dtype=np.uint8)
    padded_img = np.pad(img, ((1, 1), (1, 1)), mode='edge')
    resized = resize(padded_img, 5, 5, 'LAST_PIXEL')
    assert resized.shape == (5, 5), "El tamaño de la imagen no es correcto"
    assert resized[0, 0] == 1, "Padding de bordes con último pixel no funciona"
    assert resized[4, 4] == 4, "Padding de bordes con último pixel no funciona"


@pytest.mark.parametrize("width, height, new_width, new_height, padding", [
    (5, 5, 10, 10, 'ZEROS'),
    (2, 2, 4, 4, 'LAST_PIXEL'),
    (10, 10, 20, 20, 'ZEROS'),
    (1, 1, 3, 3, 'LAST_PIXEL')
])
def test_resize_various_cases(width, height, new_width, new_height, padding):
    img = create_test_image(width, height, channels=1, value=100)
    resized = resize(img, new_width, new_height, padding)
    assert resized.shape == (
        new_height, new_width), "El tamaño de la imagen no es correcto"
    assert np.all(
        resized == 100), "La interpolación no está funcionando correctamente"
