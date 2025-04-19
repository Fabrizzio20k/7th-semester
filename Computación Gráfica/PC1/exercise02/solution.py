import numpy as np
import cv2


def chessboard_with_custom_colors(width_in_pixels, height_in_pixels, num_of_cells_horizontal, num_of_cells_vertical, colors):
    img = np.zeros((height_in_pixels, width_in_pixels, 3), dtype=np.uint8)
    w = width_in_pixels // num_of_cells_horizontal
    h = height_in_pixels // num_of_cells_vertical

    for i in range(num_of_cells_vertical):
        for j in range(num_of_cells_horizontal):
            color = colors[(i + j) % len(colors)]
            color = color[::-1]
            img[i * h:(i + 1) * h, j *
                w:(j + 1) * w] = color

    return img
