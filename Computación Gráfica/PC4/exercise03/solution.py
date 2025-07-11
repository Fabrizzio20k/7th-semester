import cv2
import numpy as np
from time import time


def encontrar_contorno_convexo(kps):
    puntos = np.array([[int(kp.pt[0]), int(kp.pt[1])] for kp in kps])
    hull = cv2.convexHull(puntos)
    return hull.reshape(-1, 2)


def load_ply_mesh(ruta):
    verts = []
    caras = []

    with open(ruta, 'r') as f:
        ls = f.readlines()

    fin_h = 0
    n_v = 0
    n_c = 0

    for i, l in enumerate(ls):
        if l.startswith('element vertex'):
            n_v = int(l.split()[-1])
        elif l.startswith('element face'):
            n_c = int(l.split()[-1])
        elif l.startswith('end_header'):
            fin_h = i + 1
            break

    for i in range(fin_h, fin_h + n_v):
        coords = list(map(float, ls[i].split()))
        verts.append(coords[:3])

    for i in range(fin_h + n_v, fin_h + n_v + n_c):
        datos_c = list(map(int, ls[i].split()))
        vs_c = datos_c[1:]
        caras.append(vs_c)

    return np.array(verts), caras


def detectar_qr(img):
    det = cv2.QRCodeDetector()
    ret, pts = det.detect(img)

    if ret:
        return ordenar_puntos_rectangulo(pts[0])

    gris = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    sift = cv2.SIFT_create()
    kps, desc = sift.detectAndCompute(gris, None)

    if len(kps) > 50:
        hull = encontrar_contorno_convexo(kps)
        epsilon = 0.02 * cv2.arcLength(hull, True)
        approx = cv2.approxPolyDP(hull, epsilon, True)

        if len(approx) == 4:
            return ordenar_puntos_rectangulo(approx.reshape(-1, 2).astype(np.float32))

    return None


def ordenar_puntos_rectangulo(pts):
    pts = pts.reshape(-1, 2)
    centro = np.mean(pts, axis=0)

    angulos = np.arctan2(pts[:, 1] - centro[1], pts[:, 0] - centro[0])
    indices = np.argsort(angulos)

    return pts[indices]


def calcular_pose(pts_2d, K):
    if pts_2d is None:
        return None, None

    tam_real = 20.0
    pts_3d = np.array([
        [-tam_real/2, -tam_real/2, 0],
        [tam_real/2, -tam_real/2, 0],
        [tam_real/2, tam_real/2, 0],
        [-tam_real/2, tam_real/2, 0]
    ], dtype=np.float32)

    ok, rvec, tvec = cv2.solvePnP(pts_3d, pts_2d.astype(np.float32), K, None)

    if ok:
        R, _ = cv2.Rodrigues(rvec)
        return R, tvec.flatten()

    return None, None


def ajustar_mesh_a_marcador(vs, pts_2d):
    bb_min = np.min(vs, axis=0)
    bb_max = np.max(vs, axis=0)
    bb_centro = (bb_min + bb_max) / 2

    vs_centrados = vs - bb_centro
    vs_centrados[:, 2] += abs(bb_min[2])

    escala = 15.0 / max(bb_max[0] - bb_min[0], bb_max[1] - bb_min[1])
    vs_centrados *= escala

    return vs_centrados


def proyectar_3d(pts_3d, R, t, K):
    if R is None or t is None:
        return np.array([])

    pts_cam = (R @ pts_3d.T).T + t.reshape(1, 3)

    pts_2d = []
    for p in pts_cam:
        if p[2] > 0.1:
            x = K[0, 0] * p[0] / p[2] + K[0, 2]
            y = K[1, 1] * p[1] / p[2] + K[1, 2]
            pts_2d.append([x, y, p[2]])
        else:
            pts_2d.append([0, 0, -1])

    return np.array(pts_2d)


def calcular_normal(p1, p2, p3):
    v1 = p2 - p1
    v2 = p3 - p1
    n = np.cross(v1, v2)
    norm = np.linalg.norm(n)
    if norm > 0:
        return n / norm
    return np.array([0, 0, 1])


def iluminacion_coseno(normal, luz_dir=np.array([0, 0, 1])):
    coseno = np.dot(normal, luz_dir)
    return max(0.2, abs(coseno))


def dibujar_mesh_con_luz(img, vs_2d, caras, vs_3d, R):
    if len(vs_2d) == 0:
        return img

    profundidades = []

    for i, cara in enumerate(caras):
        if len(cara) < 3:
            continue

        vs_cara_2d = vs_2d[cara]
        vs_validos = [v for v in vs_cara_2d if v[2] > 0]

        if len(vs_validos) >= 3:
            prof_prom = np.mean([v[2] for v in vs_validos])
            profundidades.append((prof_prom, i))

    profundidades.sort(reverse=True, key=lambda x: x[0])

    for _, idx_cara in profundidades:
        cara = caras[idx_cara]
        pts_cara = vs_2d[cara]

        pts_validos = [p for p in pts_cara if p[2] > 0]
        if len(pts_validos) < 3:
            continue

        pts = np.array([[int(p[0]), int(p[1])] for p in pts_validos], np.int32)

        if len(pts) >= 3:
            if len(cara) >= 3:
                vs_3d_cara = vs_3d[cara[:3]]
                if R is not None:
                    vs_mundo = (R @ vs_3d_cara.T).T
                    normal = calcular_normal(
                        vs_mundo[0], vs_mundo[1], vs_mundo[2])
                else:
                    normal = calcular_normal(
                        vs_3d_cara[0], vs_3d_cara[1], vs_3d_cara[2])

                intensidad = iluminacion_coseno(normal)
                color = (int(255 * intensidad), int(255 *
                         intensidad), int(255 * intensidad))
            else:
                color = (200, 200, 200)

            pts = pts.reshape((-1, 1, 2))
            cv2.fillPoly(img, [pts], color)

    return img


def draw_mesh_on_top_of_marker(full_path_input_image, full_path_mesh, full_path_output_image):
    img = cv2.imread(full_path_input_image)
    if img is None:
        return False

    vs, caras = load_ply_mesh(full_path_mesh)

    K = np.array([
        [800, 0, img.shape[1]/2],
        [0, 800, img.shape[0]/2],
        [0, 0, 1]
    ], dtype=np.float32)

    pts_qr = detectar_qr(img)

    if pts_qr is None:
        cv2.imwrite(full_path_output_image, img)
        return False

    vs_ajustados = ajustar_mesh_a_marcador(vs, pts_qr)

    R, t = calcular_pose(pts_qr, K)

    if R is not None and t is not None:
        vs_2d = proyectar_3d(vs_ajustados, R, t, K)
        img = dibujar_mesh_con_luz(img, vs_2d, caras, vs_ajustados, R)

    cv2.imwrite(full_path_output_image, img)
    return True


if __name__ == "__main__":
    time_start = time()
    draw_mesh_on_top_of_marker('p8.jpg', 'cubo.ply', 'output.jpg')
    time_end = time()
    print(f"Tiempo de ejecución: {time_end - time_start: .2f} segundos")
