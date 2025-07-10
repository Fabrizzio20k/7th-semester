import random
import numpy as np
from PIL import Image, ImageDraw
import gc


def generate_maze(width_in_pixels, height_in_pixels, wall_thickness_in_pixels, number_of_walls):
    wall_thickness = max(1, wall_thickness_in_pixels)

    grid_width = width_in_pixels // wall_thickness
    grid_height = height_in_pixels // wall_thickness

    if grid_width % 2 == 0:
        grid_width -= 1
    if grid_height % 2 == 0:
        grid_height -= 1

    grid_width = max(5, grid_width)
    grid_height = max(5, grid_height)

    maze_grid = np.zeros((grid_height, grid_width), dtype=np.uint8)

    def create_solid_borders():
        for x in range(grid_width):
            maze_grid[0, x] = 1
            maze_grid[grid_height-1, x] = 1
        for y in range(grid_height):
            maze_grid[y, 0] = 1
            maze_grid[y, grid_width-1] = 1

    def create_entrance_and_exit():
        entrance_y = random.randrange(1, grid_height - 1)
        maze_grid[entrance_y, 0] = 0

        exit_side = random.choice(['right', 'top', 'bottom'])

        if exit_side == 'right':
            exit_y = random.randrange(1, grid_height - 1)
            maze_grid[exit_y, grid_width-1] = 0
            return (0, entrance_y), (grid_width-1, exit_y)
        elif exit_side == 'top':
            exit_x = random.randrange(1, grid_width - 1)
            maze_grid[0, exit_x] = 0
            return (0, entrance_y), (exit_x, 0)
        else:
            exit_x = random.randrange(1, grid_width - 1)
            maze_grid[grid_height-1, exit_x] = 0
            return (0, entrance_y), (exit_x, grid_height-1)

    def ensure_connectivity(entrance_pos, exit_pos):
        visited = np.zeros_like(maze_grid, dtype=bool)

        def flood_fill(start_x, start_y):
            stack = [(start_x, start_y)]

            while stack:
                x, y = stack.pop()

                if (x < 0 or x >= grid_width or y < 0 or y >= grid_height or
                        visited[y, x] or maze_grid[y, x] == 1):
                    continue

                visited[y, x] = True

                for dx, dy in [(0, 1), (1, 0), (0, -1), (-1, 0)]:
                    stack.append((x + dx, y + dy))

        flood_fill(entrance_pos[0], entrance_pos[1])

        if not visited[exit_pos[1], exit_pos[0]]:
            start_x, start_y = entrance_pos
            end_x, end_y = exit_pos

            current_x, current_y = start_x, start_y

            while current_x != end_x:
                if current_x < end_x:
                    current_x += 1
                else:
                    current_x -= 1
                maze_grid[current_y, current_x] = 0

            while current_y != end_y:
                if current_y < end_y:
                    current_y += 1
                else:
                    current_y -= 1
                maze_grid[current_y, current_x] = 0

    create_solid_borders()
    entrance_pos, exit_pos = create_entrance_and_exit()

    inner_width = grid_width - 2
    inner_height = grid_height - 2
    total_inner_cells = inner_width * inner_height

    target_inner_walls = min(number_of_walls, total_inner_cells)

    inner_positions = [(x, y) for y in range(1, grid_height-1)
                       for x in range(1, grid_width-1)]
    random.shuffle(inner_positions)

    for i in range(target_inner_walls):
        if i < len(inner_positions):
            x, y = inner_positions[i]
            maze_grid[y, x] = 1

    ensure_connectivity(entrance_pos, exit_pos)

    def apply_basic_structure():
        if target_inner_walls > total_inner_cells * 0.3:
            for iteration in range(2):
                new_grid = maze_grid.copy()

                for y in range(2, grid_height - 2):
                    for x in range(2, grid_width - 2):
                        wall_neighbors = sum(1 for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]
                                             if maze_grid[y+dy, x+dx] == 1)

                        if maze_grid[y, x] == 1 and wall_neighbors < 1:
                            new_grid[y, x] = 0
                        elif maze_grid[y, x] == 0 and wall_neighbors >= 3:
                            if random.random() < 0.3:
                                new_grid[y, x] = 1

                maze_grid[:] = new_grid

    apply_basic_structure()
    ensure_connectivity(entrance_pos, exit_pos)

    current_inner_walls = sum(1 for y in range(1, grid_height-1) for x in range(1, grid_width-1)
                              if maze_grid[y, x] == 1)

    diff = target_inner_walls - current_inner_walls

    if diff > 0:
        candidates = [(x, y) for y in range(1, grid_height-1) for x in range(1, grid_width-1)
                      if maze_grid[y, x] == 0]
        random.shuffle(candidates)

        for i in range(min(diff, len(candidates))):
            x, y = candidates[i]
            maze_grid[y, x] = 1

    elif diff < 0:
        candidates = [(x, y) for y in range(1, grid_height-1) for x in range(1, grid_width-1)
                      if maze_grid[y, x] == 1]
        random.shuffle(candidates)

        for i in range(min(-diff, len(candidates))):
            x, y = candidates[i]
            maze_grid[y, x] = 0

    ensure_connectivity(entrance_pos, exit_pos)

    def add_stones_in_gaps():
        for y in range(1, grid_height - 1):
            for x in range(1, grid_width - 1):
                if maze_grid[y, x] == 0:
                    wall_neighbors = sum(1 for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]
                                         if maze_grid[y+dy, x+dx] == 1)

                    if wall_neighbors >= 2 and random.random() < 0.15:
                        original_val = maze_grid[y, x]
                        maze_grid[y, x] = 1

                        visited = np.zeros_like(maze_grid, dtype=bool)
                        stack = [(entrance_pos[0], entrance_pos[1])]
                        can_reach_exit = False

                        while stack:
                            sx, sy = stack.pop()
                            if sx < 0 or sx >= grid_width or sy < 0 or sy >= grid_height:
                                continue
                            if visited[sy, sx] or maze_grid[sy, sx] == 1:
                                continue

                            visited[sy, sx] = True

                            if (sx, sy) == exit_pos:
                                can_reach_exit = True
                                break

                            for dx, dy in [(0, 1), (1, 0), (0, -1), (-1, 0)]:
                                stack.append((sx + dx, sy + dy))

                        if not can_reach_exit:
                            maze_grid[y, x] = original_val

    add_stones_in_gaps()

    final_width = grid_width * wall_thickness
    final_height = grid_height * wall_thickness

    if final_width > 16000 or final_height > 16000:
        scale_factor = min(16000 / final_width, 16000 / final_height)
        final_width = int(final_width * scale_factor)
        final_height = int(final_height * scale_factor)
        wall_thickness = int(wall_thickness * scale_factor)

    try:
        img = Image.new('RGB', (final_width, final_height),
                        color=(140, 136, 117))
        draw = ImageDraw.Draw(img)

        wall_color = (126, 148, 103)
        path_color = (140, 136, 117)

        for grid_y in range(grid_height):
            for grid_x in range(grid_width):
                start_x = grid_x * wall_thickness
                start_y = grid_y * wall_thickness
                end_x = min(start_x + wall_thickness, final_width)
                end_y = min(start_y + wall_thickness, final_height)

        for grid_y in range(grid_height):
            for grid_x in range(grid_width):
                start_x = grid_x * wall_thickness
                start_y = grid_y * wall_thickness
                end_x = min(start_x + wall_thickness, final_width)
                end_y = min(start_y + wall_thickness, final_height)

        for grid_y in range(grid_height):
            for grid_x in range(grid_width):
                start_x = grid_x * wall_thickness
                start_y = grid_y * wall_thickness
                end_x = min(start_x + wall_thickness, final_width)
                end_y = min(start_y + wall_thickness, final_height)

                if maze_grid[grid_y, grid_x] == 1:
                    corner_radius = wall_thickness // 6
                    curve_variation = wall_thickness // 10

                    top_curve = start_y + \
                        random.randint(-curve_variation, curve_variation)
                    bottom_curve = end_y + \
                        random.randint(-curve_variation, curve_variation)
                    left_curve = start_x + \
                        random.randint(-curve_variation, curve_variation)
                    right_curve = end_x + \
                        random.randint(-curve_variation, curve_variation)

                    points = [
                        (start_x + corner_radius, top_curve),
                        (end_x - corner_radius, top_curve),
                        (right_curve, start_y + corner_radius),
                        (right_curve, end_y - corner_radius),
                        (end_x - corner_radius, bottom_curve),
                        (start_x + corner_radius, bottom_curve),
                        (left_curve, end_y - corner_radius),
                        (left_curve, start_y + corner_radius)
                    ]

                    draw.polygon(points, fill=wall_color)

        for grid_y in range(grid_height):
            for grid_x in range(grid_width):
                if maze_grid[grid_y, grid_x] == 1:
                    start_x = grid_x * wall_thickness
                    start_y = grid_y * wall_thickness
                    end_x = min(start_x + wall_thickness, final_width)
                    end_y = min(start_y + wall_thickness, final_height)

                    has_free_top = grid_y > 0 and maze_grid[grid_y -
                                                            1, grid_x] == 0
                    has_free_bottom = grid_y < grid_height - \
                        1 and maze_grid[grid_y+1, grid_x] == 0
                    has_free_left = grid_x > 0 and maze_grid[grid_y,
                                                             grid_x-1] == 0
                    has_free_right = grid_x < grid_width - \
                        1 and maze_grid[grid_y, grid_x+1] == 0

        for grid_y in range(grid_height):
            for grid_x in range(grid_width):
                if maze_grid[grid_y, grid_x] == 1:
                    start_x = grid_x * wall_thickness
                    start_y = grid_y * wall_thickness
                    end_x = min(start_x + wall_thickness, final_width)
                    end_y = min(start_y + wall_thickness, final_height)

                    has_free_top = grid_y > 0 and maze_grid[grid_y -
                                                            1, grid_x] == 0
                    has_free_bottom = grid_y < grid_height - \
                        1 and maze_grid[grid_y+1, grid_x] == 0
                    has_free_left = grid_x > 0 and maze_grid[grid_y,
                                                             grid_x-1] == 0
                    has_free_right = grid_x < grid_width - \
                        1 and maze_grid[grid_y, grid_x+1] == 0

                    curve_intensity = wall_thickness // 15

                    if has_free_top:
                        points = []
                        for i in range(start_x, end_x, 3):
                            offset = random.randint(-curve_intensity,
                                                    curve_intensity)
                            points.append((i, start_y + offset))
                        if len(points) > 1:
                            for i in range(len(points)-1):
                                draw.line([points[i], points[i+1]],
                                          fill=(0, 0, 0), width=2)

                    if has_free_bottom:
                        points = []
                        for i in range(start_x, end_x, 3):
                            offset = random.randint(-curve_intensity,
                                                    curve_intensity)
                            points.append((i, end_y-1 + offset))
                        if len(points) > 1:
                            for i in range(len(points)-1):
                                draw.line([points[i], points[i+1]],
                                          fill=(0, 0, 0), width=2)

                    if has_free_left:
                        points = []
                        for i in range(start_y, end_y, 3):
                            offset = random.randint(-curve_intensity,
                                                    curve_intensity)
                            points.append((start_x + offset, i))
                        if len(points) > 1:
                            for i in range(len(points)-1):
                                draw.line([points[i], points[i+1]],
                                          fill=(0, 0, 0), width=2)

                    if has_free_right:
                        points = []
                        for i in range(start_y, end_y, 3):
                            offset = random.randint(-curve_intensity,
                                                    curve_intensity)
                            points.append((end_x-1 + offset, i))
                        if len(points) > 1:
                            for i in range(len(points)-1):
                                draw.line([points[i], points[i+1]],
                                          fill=(0, 0, 0), width=2)

        noise_intensity = 8
        img_array = np.array(img)
        noise_mask = np.random.randint(-noise_intensity, noise_intensity + 1,
                                       (final_height, final_width, 3), dtype=np.int16)
        img_array = np.clip(img_array.astype(np.int16) +
                            noise_mask, 0, 255).astype(np.uint8)

        final_img = Image.fromarray(img_array, 'RGB')
        final_img.save("generated_maze.png")

        del img_array, noise_mask
        gc.collect()

        return final_img

    except MemoryError:
        raise MemoryError(
            f"No hay suficiente memoria para generar imagen de {final_width}x{final_height}")


if __name__ == "__main__":
    generate_maze(1000, 1000, 40, 150)
