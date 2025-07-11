import numpy as np
from PIL import Image
import math


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
                   optical_axis_x, optical_axis_y, optical_axis_z,
                   up_vector_x, up_vector_y, up_vector_z, focal_distance,
                   output_width_in_pixels, output_height_in_pixels, full_path_output):

    v, c = leer_ply(full_path_input_mesh)

    oc = np.array([optical_center_x, optical_center_y,
                  optical_center_z], dtype=np.float32)
    oa = np.array([optical_axis_x, optical_axis_y,
                  optical_axis_z], dtype=np.float32)
    up = np.array([up_vector_x, up_vector_y, up_vector_z], dtype=np.float32)

    if np.linalg.norm(oa) < 1e-6:
        print(
            "Warning: optical_axis es vector cero, usando valor por defecto [0, 0, -1]")
        oa = np.array([0, 0, -1], dtype=np.float32)
    else:
        oa = oa / np.linalg.norm(oa)

    if np.linalg.norm(up) < 1e-6:
        print(
            "Warning: up_vector es vector cero, usando valor por defecto [0, 1, 0]")
        up = np.array([0, 1, 0], dtype=np.float32)
    else:
        up = up / np.linalg.norm(up)

    z_c = -oa

    cross_prod = np.cross(up, z_c)
    cross_norm = np.linalg.norm(cross_prod)

    if cross_norm < 1e-6:
        if abs(z_c[0]) < 0.9:
            alt_up = np.array([1.0, 0.0, 0.0], dtype=np.float32)
        else:
            alt_up = np.array([0.0, 1.0, 0.0], dtype=np.float32)
        x_c = np.cross(alt_up, z_c)
        x_c = x_c / np.linalg.norm(x_c)
    else:
        x_c = cross_prod / cross_norm

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
                            optical_axis_x, optical_axis_y, optical_axis_z,
                            up_vector_x, up_vector_y, up_vector_z, focal_distance,
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
            up_vector_x[i],
            up_vector_y[i],
            up_vector_z[i],
            focal_distance[i],
            output_width_in_pixels,
            output_height_in_pixels,
            output_path
        )


if __name__ == "__main__":
    n_frames = 10

    radius = 5.0

    camera_height = 2.0

    optical_center_x = []
    optical_center_y = []
    optical_center_z = []
    optical_axis_x = []
    optical_axis_y = []
    optical_axis_z = []
    up_vector_x = []
    up_vector_y = []
    up_vector_z = []
    focal_distance = []

    for i in range(n_frames):
        angle = (2 * math.pi * i) / n_frames

        cam_x = radius * math.cos(angle)
        cam_y = camera_height
        cam_z = radius * math.sin(angle)

        target_x, target_y, target_z = 0, 0, 0

        axis_x = target_x - cam_x
        axis_y = target_y - cam_y
        axis_z = target_z - cam_z

        axis_length = math.sqrt(axis_x**2 + axis_y**2 + axis_z**2)
        if axis_length > 0:
            axis_x /= axis_length
            axis_y /= axis_length
            axis_z /= axis_length

        up_x, up_y, up_z = 0, 1, 0

        optical_center_x.append(cam_x)
        optical_center_y.append(cam_y)
        optical_center_z.append(cam_z)
        optical_axis_x.append(axis_x)
        optical_axis_y.append(axis_y)
        optical_axis_z.append(axis_z)
        up_vector_x.append(up_x)
        up_vector_y.append(up_y)
        up_vector_z.append(up_z)
        focal_distance.append(500)

    sequence_of_projections(
        full_path_input_mesh="esfera.ply",
        optical_center_x=optical_center_x,
        optical_center_y=optical_center_y,
        optical_center_z=optical_center_z,
        optical_axis_x=optical_axis_x,
        optical_axis_y=optical_axis_y,
        optical_axis_z=optical_axis_z,
        up_vector_x=up_vector_x,
        up_vector_y=up_vector_y,
        up_vector_z=up_vector_z,
        focal_distance=focal_distance,
        output_width_in_pixels=800,
        output_height_in_pixels=600,
        prefix_output_files="output/frame"
    )
