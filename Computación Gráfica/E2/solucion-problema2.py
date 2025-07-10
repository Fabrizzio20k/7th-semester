import cv2
import numpy as np
from collections import deque


def solve_maze(input_image_path, output_image_name):
    img = cv2.imread(input_image_path)
    height, width = img.shape[:2]
    grid_size = 17
    cell_width = width // grid_size
    cell_height = height // grid_size

    maze_matrix = np.ones((grid_size, grid_size), dtype=int)

    for grid_y in range(grid_size):
        for grid_x in range(grid_size):
            start_x = grid_x * cell_width
            start_y = grid_y * cell_height
            end_x = min(start_x + cell_width, width)
            end_y = min(start_y + cell_height, height)

            cell = img[start_y:end_y, start_x:end_x]
            avg_color = np.mean(cell.reshape(-1, 3), axis=0)

            b, g, r = avg_color

            is_green_obstacle = (g > r + 15 and g > b + 10 and g > 100)
            is_very_dark = (r + g + b) < 200

            if is_green_obstacle or is_very_dark:
                maze_matrix[grid_y, grid_x] = 0
            else:
                maze_matrix[grid_y, grid_x] = 1

    for x in range(grid_size):
        maze_matrix[0, x] = 0
        maze_matrix[16, x] = 0

    for y in range(8):
        maze_matrix[y, 0] = 0
        maze_matrix[y, 16] = 0

    for y in range(9, 17):
        maze_matrix[y, 0] = 0
        maze_matrix[y, 16] = 0

    maze_matrix[8, 0] = 1
    maze_matrix[8, 16] = 1

    start_pos = (8, 0)
    end_pos = (8, 16)

    def bfs_shortest_path(matrix, start, end):
        queue = deque([(start, [start])])
        visited = set([start])
        directions = [(0, 1), (1, 0), (0, -1), (-1, 0)]

        while queue:
            (y, x), path = queue.popleft()

            if (y, x) == end:
                return path

            for dy, dx in directions:
                ny, nx = y + dy, x + dx

                if (0 <= ny < grid_size and 0 <= nx < grid_size and
                        (ny, nx) not in visited and matrix[ny, nx] == 1):
                    visited.add((ny, nx))
                    queue.append(((ny, nx), path + [(ny, nx)]))

        return None

    path = bfs_shortest_path(maze_matrix, start_pos, end_pos)

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    output_img = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)

    if path:
        for y, x in path:
            start_x_pixel = x * cell_width
            start_y_pixel = y * cell_height
            end_x_pixel = min(start_x_pixel + cell_width, width)
            end_y_pixel = min(start_y_pixel + cell_height, height)

            output_img[start_y_pixel:end_y_pixel,
                       start_x_pixel:end_x_pixel] = [0, 0, 255]

    cv2.imwrite(output_image_name, output_img)
    return output_image_name


if __name__ == "__main__":
    solve_maze("laberinto2.png", "maze_solved.png")
