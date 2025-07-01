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
                   optical_axis_x, optical_axis_y, optical_axis_z,
                   up_vector_x, up_vector_y, up_vector_z, focal_distance,
                   output_width_in_pixels, output_height_in_pixels, full_path_output):

    v, c = leer_ply(full_path_input_mesh)

    oc = np.array([optical_center_x, optical_center_y,
                  optical_center_z], dtype=np.float32)
    oa = np.array([optical_axis_x, optical_axis_y,
                  optical_axis_z], dtype=np.float32)
    up = np.array([up_vector_x, up_vector_y, up_vector_z], dtype=np.float32)

    oa = oa / np.linalg.norm(oa)
    up = up / np.linalg.norm(up)

    z_c = -oa

    x_c = np.cross(up, z_c)
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


if __name__ == "__main__":
    # Example usage
    project_points(
        full_path_input_mesh='esfera.ply',
        optical_center_x=0.0,
        optical_center_y=0.0,
        optical_center_z=0.0,
        optical_axis_x=0.0,
        optical_axis_y=0.0,
        optical_axis_z=-1.0,
        up_vector_x=0.0,
        up_vector_y=1.0,
        up_vector_z=0.0,
        focal_distance=1.0,
        output_width_in_pixels=640,
        output_height_in_pixels=480,
        full_path_output='prueba.png'
    )
