import numpy as np


def resize(InputImage, NEW_WIDTH, NEW_HEIGHT, PADDING_STRATEGY):
    if len(InputImage.shape) == 2:
        InputImage = InputImage[:, :, np.newaxis]

    h, w, c = InputImage.shape

    if PADDING_STRATEGY == 'ZEROS':
        img = np.pad(InputImage, ((1, 1), (1, 1), (0, 0)),
                     mode='constant', constant_values=0)
    elif PADDING_STRATEGY == 'LAST_PIXEL':
        img = np.pad(InputImage, ((1, 1), (1, 1), (0, 0)), mode='edge')
    else:
        raise ValueError("Estrategia no valida")

    new_h, new_w = img.shape[:2]

    scaleX = (w) / NEW_WIDTH
    scaleY = (h) / NEW_HEIGHT

    new_img = np.zeros((NEW_HEIGHT, NEW_WIDTH, c), dtype=np.uint8)

    for y in range(NEW_HEIGHT):
        for x in range(NEW_WIDTH):
            src_x = x * scaleX + 1
            src_y = y * scaleY + 1

            x0 = int(np.floor(src_x))
            x1 = min(x0 + 1, new_w - 1)
            y0 = int(np.floor(src_y))
            y1 = min(y0 + 1, new_h - 1)

            dx = src_x - x0
            dy = src_y - y0
            for channel in range(c):
                val = (
                    img[y0, x0, channel] * (1 - dx) * (1 - dy) +
                    img[y0, x1, channel] * dx * (1 - dy) +
                    img[y1, x0, channel] * (1 - dx) * dy +
                    img[y1, x1, channel] * dx * dy
                )

                new_img[y, x, channel] = np.clip(val, 0, 255)

    if new_img.shape[2] == 1:
        return new_img[:, :, 0]

    return new_img
