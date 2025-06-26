import numpy as np
from PIL import Image


def leer_ply(path):
    verts = []
    cols = []
    tiene_color = False

    with open(path, 'r') as f:
        linea = f.readline().strip()
        if linea != 'ply':
            raise ValueError("archivo no es ply")

        num_v = 0
        props = []
        en_header = True

        while en_header:
            linea = f.readline().strip()
            if linea.startswith('element vertex'):
                num_v = int(linea.split()[-1])
            elif linea.startswith('property'):
                props.append(linea.split()[2])
            elif linea == 'end_header':
                en_header = False

        tiene_color = 'red' in props and 'green' in props and 'blue' in props

        for _ in range(num_v):
            datos = f.readline().strip().split()
            x, y, z = float(datos[0]), float(datos[1]), float(datos[2])
            verts.append([x, y, z])

            if tiene_color and len(datos) >= 6:
                r, g, b = int(datos[3]), int(datos[4]), int(datos[5])
                cols.append([r, g, b])

    v = np.array(verts, dtype=np.float32)
    c = np.array(cols, dtype=np.uint8) if tiene_color else None

    return v, c


def project_points(full_path_input_mesh, optical_center_x, optical_center_y, optical_center_z,
                   optical_axis_x, optical_axis_y, optical_axis_z, focal_distance,
                   output_width_in_pixels, output_height_in_pixels, full_path_output):

    v, c = leer_ply(full_path_input_mesh)

    oc = np.array([optical_center_x, optical_center_y,
                  optical_center_z], dtype=np.float32)
    oa = np.array([optical_axis_x, optical_axis_y,
                  optical_axis_z], dtype=np.float32)

    oa = oa / np.linalg.norm(oa)

    z_c = -oa

    if abs(z_c[0]) < 0.9:
        x_c = np.cross(z_c, np.array([1, 0, 0], dtype=np.float32))
    else:
        x_c = np.cross(z_c, np.array([0, 1, 0], dtype=np.float32))

    x_c = x_c / np.linalg.norm(x_c)
    y_c = np.cross(z_c, x_c)
    y_c = y_c / np.linalg.norm(y_c)

    R = np.array([x_c, y_c, z_c], dtype=np.float32)

    v_cam = (v - oc) @ R.T

    mask = v_cam[:, 2] < 0
    v_cam = v_cam[mask]

    if c is not None:
        c_val = c[mask]

    if len(v_cam) == 0:
        img = Image.new('RGB', (output_width_in_pixels,
                        output_height_in_pixels), (0, 0, 0))
        img.save(full_path_output)
        return

    x_p = (v_cam[:, 0] * focal_distance) / (-v_cam[:, 2])
    y_p = (v_cam[:, 1] * focal_distance) / (-v_cam[:, 2])

    cx = output_width_in_pixels / 2.0
    cy = output_height_in_pixels / 2.0

    u = x_p + cx
    vv = -y_p + cy

    img_arr = np.zeros(
        (output_height_in_pixels, output_width_in_pixels, 3), dtype=np.uint8)

    for i in range(len(u)):
        px = int(round(u[i]))
        py = int(round(vv[i]))

        if 0 <= px < output_width_in_pixels and 0 <= py < output_height_in_pixels:
            if c is not None:
                col = c_val[i]
            else:
                col = [255, 255, 255]

            img_arr[py, px] = col

    img = Image.fromarray(img_arr)
    img.save(full_path_output)


def sequence_of_projections(full_path_input_mesh, optical_center_x, optical_center_y, optical_center_z,
                            optical_axis_x, optical_axis_y, optical_axis_z, focal_distance,
                            output_width_in_pixels, output_height_in_pixels, prefix_output_files):

    n_frames = len(optical_center_x)

    for i in range(n_frames):
        output_path = f"{prefix_output_files}-{i+1}.png"

        project_points(
            full_path_input_mesh,
            optical_center_x[i],
            optical_center_y[i],
            optical_center_z[i],
            optical_axis_x[i],
            optical_axis_y[i],
            optical_axis_z[i],
            focal_distance,
            output_width_in_pixels,
            output_height_in_pixels,
            output_path
        )


def gen_ply(path, v, c=None):
    with open(path, 'w') as f:
        f.write('ply\n')
        f.write('format ascii 1.0\n')
        f.write(f'element vertex {len(v)}\n')
        f.write('property float x\n')
        f.write('property float y\n')
        f.write('property float z\n')

        if c is not None:
            f.write('property uchar red\n')
            f.write('property uchar green\n')
            f.write('property uchar blue\n')

        f.write('end_header\n')

        for i in range(len(v)):
            if c is not None:
                f.write(
                    f'{v[i][0]} {v[i][1]} {v[i][2]} {c[i][0]} {c[i][1]} {c[i][2]}\n')
            else:
                f.write(f'{v[i][0]} {v[i][1]} {v[i][2]}\n')


def gen_esfera(r=1, res=50):
    v = []
    c = []

    for i in range(res):
        theta = i * np.pi / (res - 1)
        for j in range(res):
            phi = j * 2 * np.pi / res

            x = r * np.sin(theta) * np.cos(phi)
            y = r * np.sin(theta) * np.sin(phi)
            z = r * np.cos(theta)

            v.append([x, y, z])

            col_r = int(255 * (x + r) / (2 * r))
            col_g = int(255 * (y + r) / (2 * r))
            col_b = int(255 * (z + r) / (2 * r))
            c.append([col_r, col_g, col_b])

    return np.array(v), np.array(c, dtype=np.uint8)


if __name__ == "__main__":
    import math

    verts, colors = gen_esfera(2, 100)
    gen_ply('esfera.ply', verts, colors)

    n = 36
    r = 5.0

    pos_x = [r * math.cos(2 * math.pi * i / n) for i in range(n)]
    pos_y = [r * math.sin(2 * math.pi * i / n) for i in range(n)]
    pos_z = [2.0] * n

    ax_x = [-math.cos(2 * math.pi * i / n) for i in range(n)]
    ax_y = [-math.sin(2 * math.pi * i / n) for i in range(n)]
    ax_z = [-0.4] * n

    sequence_of_projections(
        full_path_input_mesh='esfera.ply',
        optical_center_x=pos_x,
        optical_center_y=pos_y,
        optical_center_z=pos_z,
        optical_axis_x=ax_x,
        optical_axis_y=ax_y,
        optical_axis_z=ax_z,
        focal_distance=500,
        output_width_in_pixels=800,
        output_height_in_pixels=600,
        prefix_output_files='anim/frame'
    )
