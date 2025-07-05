import cv2
import numpy as np
import math


def stitch_images(full_path_input_image, blender, features_finder, features_matcher, warper, full_path_output_image):
    imgs = []
    for p in full_path_input_image:
        img = cv2.imread(p)
        if img is not None:
            imgs.append(img)

    if len(imgs) < 2:
        return False

    try:
        st = cv2.Stitcher.create()
        ret, pano = st.stitch(imgs)

        if ret == cv2.Stitcher_OK:
            cv2.imwrite(full_path_output_image, pano)
            return True
    except:
        pass

    return stitch_manual(imgs, blender, features_finder, features_matcher, warper, full_path_output_image)


def stitch_manual(imgs, bl, ff_t, fm_t, wp_t, out):
    try:
        detector = get_feature_detector(ff_t)

        kp_list = []
        desc_list = []

        for img in imgs:
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            kp, desc = detector.detectAndCompute(gray, None)
            kp_list.append(kp)
            desc_list.append(desc)

        if any(d is None for d in desc_list):
            return False

        matches_list = []
        for i in range(len(desc_list) - 1):
            matches = match_features(desc_list[i], desc_list[i+1], fm_t)
            matches_list.append(matches)

        H_matrices = []
        for i, matches in enumerate(matches_list):
            H = compute_homography(kp_list[i], kp_list[i+1], matches, fm_t)
            H_matrices.append(H)

        if wp_t == 'Cylindrical':
            result = stitch_cylindrical(imgs, H_matrices)
        elif wp_t == 'Spherical':
            result = stitch_spherical(imgs, H_matrices)
        elif wp_t == 'Plane':
            result = stitch_planar(imgs, H_matrices)
        elif wp_t == 'Affine':
            result = stitch_affine(imgs, H_matrices)
        elif wp_t == 'Mercator':
            result = stitch_mercator(imgs, H_matrices)
        elif wp_t == 'Fisheye':
            result = stitch_fisheye(imgs, H_matrices)
        elif wp_t == 'Stereographic':
            result = stitch_stereographic(imgs, H_matrices)
        elif wp_t == 'Panini':
            result = stitch_panini(imgs, H_matrices)
        elif wp_t == 'PaniniPortrait':
            result = stitch_panini_portrait(imgs, H_matrices)
        elif wp_t == 'CompressedRectilinear':
            result = stitch_compressed_rectilinear(imgs, H_matrices)
        elif wp_t == 'CompressedRectilinearPortrait':
            result = stitch_compressed_rectilinear_portrait(imgs, H_matrices)
        elif wp_t == 'TransverseMercator':
            result = stitch_transverse_mercator(imgs, H_matrices)
        else:
            result = stitch_planar(imgs, H_matrices)

        if result is None:
            return False

        if bl == 'FeatherBlender':
            final = feather_blend(result, imgs, H_matrices, wp_t)
        elif bl == 'MultiBandBlender':
            final = multiband_blend(result, imgs, H_matrices, wp_t)
        else:
            final = result

        cv2.imwrite(out, final)
        return True

    except Exception as e:
        print(f"Error in manual stitching: {e}")
        return False


def get_feature_detector(ff_t):
    if ff_t == 'AKAZE':
        return cv2.AKAZE.create()
    elif ff_t == 'ORB':
        return cv2.ORB.create(nfeatures=1000)
    elif ff_t == 'SIFT':
        return cv2.SIFT.create()
    elif ff_t == 'SURF':
        return cv2.xfeatures2d.SURF.create(400)
    else:
        return cv2.SIFT.create()


def match_features(desc1, desc2, fm_t):
    if fm_t == 'AffineBestOf2Nearest':
        matcher = cv2.BFMatcher()
        raw_matches = matcher.knnMatch(desc1, desc2, k=2)
        matches = []
        for m in raw_matches:
            if len(m) == 2 and m[0].distance < 0.8 * m[1].distance:
                matches.append(m[0])
        return matches
    elif fm_t == 'BestOf2NearestRange':
        matcher = cv2.BFMatcher()
        raw_matches = matcher.knnMatch(desc1, desc2, k=2)
        matches = []
        for m in raw_matches:
            if len(m) == 2 and m[0].distance < 0.7 * m[1].distance:
                matches.append(m[0])
        return matches
    else:
        matcher = cv2.BFMatcher()
        matches = matcher.match(desc1, desc2)
        return sorted(matches, key=lambda x: x.distance)[:50]


def compute_homography(kp1, kp2, matches, fm_t):
    if len(matches) < 4:
        return np.eye(3)

    src_pts = np.float32(
        [kp1[m.queryIdx].pt for m in matches]).reshape(-1, 1, 2)
    dst_pts = np.float32(
        [kp2[m.trainIdx].pt for m in matches]).reshape(-1, 1, 2)

    if fm_t == 'AffineBestOf2Nearest':
        H, _ = cv2.estimateAffinePartial2D(src_pts, dst_pts, method=cv2.RANSAC)
        if H is not None:
            H = np.vstack([H, [0, 0, 1]])
        else:
            H = np.eye(3)
    else:
        H, _ = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC, 5.0)
        if H is None:
            H = np.eye(3)

    return H


def stitch_planar(imgs, H_list):
    if len(imgs) == 0:
        return None

    result = imgs[0].copy()

    for i in range(1, len(imgs)):
        H = H_list[i-1] if i-1 < len(H_list) else np.eye(3)

        h1, w1 = result.shape[:2]
        h2, w2 = imgs[i].shape[:2]

        corners1 = np.float32(
            [[0, 0], [0, h1], [w1, h1], [w1, 0]]).reshape(-1, 1, 2)
        corners2 = np.float32(
            [[0, 0], [0, h2], [w2, h2], [w2, 0]]).reshape(-1, 1, 2)
        corners2_transformed = cv2.perspectiveTransform(corners2, H)

        all_corners = np.concatenate((corners1, corners2_transformed), axis=0)
        [x_min, y_min] = np.int32(all_corners.min(axis=0).ravel() - 0.5)
        [x_max, y_max] = np.int32(all_corners.max(axis=0).ravel() + 0.5)

        translation = np.array([[1, 0, -x_min], [0, 1, -y_min], [0, 0, 1]])

        result_warped = cv2.warpPerspective(
            result, translation, (x_max-x_min, y_max-y_min))
        img_warped = cv2.warpPerspective(
            imgs[i], translation.dot(H), (x_max-x_min, y_max-y_min))

        mask = cv2.cvtColor(img_warped, cv2.COLOR_BGR2GRAY)
        mask = cv2.threshold(mask, 1, 255, cv2.THRESH_BINARY)[1]

        result = blend_images(result_warped, img_warped, mask)

    return result


def stitch_cylindrical(imgs, H_list):
    """Corregido: stitching curvo-aware para cylindrical"""
    focal = estimate_focal_length(imgs[0])
    warped_imgs = []

    for img in imgs:
        warped = cylindrical_warp(img, focal)
        warped_imgs.append(warped)

    return stitch_curved_aware(warped_imgs, H_list, 'Cylindrical', focal)


def stitch_spherical(imgs, H_list):
    """Corregido: stitching curvo-aware para spherical"""
    focal = estimate_focal_length(imgs[0])
    warped_imgs = []

    for img in imgs:
        warped = spherical_warp(img, focal)
        warped_imgs.append(warped)

    return stitch_curved_aware(warped_imgs, H_list, 'Spherical', focal)


def stitch_mercator(imgs, H_list):
    """Corregido: stitching geográfico-aware para mercator"""
    focal = estimate_focal_length(imgs[0])
    warped_imgs = []

    for img in imgs:
        warped = mercator_warp(img, focal)
        warped_imgs.append(warped)

    return stitch_geographic_aware(warped_imgs, H_list, 'Mercator', focal)


def stitch_fisheye(imgs, H_list):
    focal = estimate_focal_length(imgs[0]) * 0.5
    warped_imgs = []

    for img in imgs:
        warped = fisheye_warp(img, focal)
        warped_imgs.append(warped)

    return stitch_radial_aware(warped_imgs, H_list, 'Fisheye')


def stitch_stereographic(imgs, H_list):
    focal = estimate_focal_length(imgs[0])
    warped_imgs = []

    for img in imgs:
        warped = stereographic_warp(img, focal)
        warped_imgs.append(warped)

    return stitch_radial_aware(warped_imgs, H_list, 'Stereographic')


def stitch_panini(imgs, H_list):
    focal = estimate_focal_length(imgs[0])
    warped_imgs = []

    for img in imgs:
        warped = panini_warp(img, focal, d=1.0)
        warped_imgs.append(warped)

    return stitch_hybrid_aware(warped_imgs, H_list, 'Panini', focal)


def stitch_panini_portrait(imgs, H_list):
    focal = estimate_focal_length(imgs[0])
    warped_imgs = []

    for img in imgs:
        warped = panini_portrait_warp(img, focal, d=1.0)
        warped_imgs.append(warped)

    return stitch_hybrid_aware(warped_imgs, H_list, 'PaniniPortrait', focal)


def stitch_compressed_rectilinear(imgs, H_list):
    focal = estimate_focal_length(imgs[0])
    warped_imgs = []

    for img in imgs:
        warped = compressed_rectilinear_warp(img, focal, compression=0.8)
        warped_imgs.append(warped)

    return stitch_planar(warped_imgs, H_list)


def stitch_compressed_rectilinear_portrait(imgs, H_list):
    focal = estimate_focal_length(imgs[0])
    warped_imgs = []

    for img in imgs:
        warped = compressed_rectilinear_portrait_warp(
            img, focal, compression=0.8)
        warped_imgs.append(warped)

    return stitch_planar(warped_imgs, H_list)


def stitch_transverse_mercator(imgs, H_list):
    focal = estimate_focal_length(imgs[0])
    warped_imgs = []

    for img in imgs:
        warped = transverse_mercator_warp(img, focal)
        warped_imgs.append(warped)

    return stitch_geographic_aware(warped_imgs, H_list, 'TransverseMercator', focal)


def stitch_affine(imgs, H_list):
    result = imgs[0].copy()

    for i in range(1, len(imgs)):
        if i-1 < len(H_list):
            H = H_list[i-1][:2, :]
            h1, w1 = result.shape[:2]
            h2, w2 = imgs[i].shape[:2]

            warped = cv2.warpAffine(imgs[i], H, (w1 + w2, max(h1, h2)))
            canvas = np.zeros((max(h1, h2), w1 + w2, 3), dtype=np.uint8)
            canvas[:h1, :w1] = result

            mask = cv2.cvtColor(warped, cv2.COLOR_BGR2GRAY)
            mask = cv2.threshold(mask, 1, 255, cv2.THRESH_BINARY)[1]

            result = blend_images(canvas, warped, mask)

    return result


def estimate_focal_length(img):
    h, w = img.shape[:2]
    diagonal = np.sqrt(w**2 + h**2)
    aspect_ratio = w / h

    if aspect_ratio > 1.5:
        focal = diagonal * 0.6
    elif aspect_ratio < 0.7:
        focal = diagonal * 0.8
    else:
        focal = diagonal * 0.7

    return focal


def cylindrical_warp(img, focal):
    h, w = img.shape[:2]

    map_x = np.zeros((h, w), dtype=np.float32)
    map_y = np.zeros((h, w), dtype=np.float32)

    for y in range(h):
        for x in range(w):
            theta = (x - w/2) / focal
            h_val = (y - h/2) / focal

            X = np.sin(theta)
            Y = h_val
            Z = np.cos(theta)

            if Z != 0:
                x_proj = focal * X / Z + w/2
                y_proj = focal * Y / Z + h/2
            else:
                x_proj = x
                y_proj = y

            map_x[y, x] = x_proj
            map_y[y, x] = y_proj

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def spherical_warp(img, focal):
    h, w = img.shape[:2]

    map_x = np.zeros((h, w), dtype=np.float32)
    map_y = np.zeros((h, w), dtype=np.float32)

    for y in range(h):
        for x in range(w):
            theta = (x - w/2) / focal
            phi = (y - h/2) / focal

            X = np.sin(theta) * np.cos(phi)
            Y = np.sin(phi)
            Z = np.cos(theta) * np.cos(phi)

            if Z != 0:
                x_proj = focal * X / Z + w/2
                y_proj = focal * Y / Z + h/2
            else:
                x_proj = x
                y_proj = y

            map_x[y, x] = x_proj
            map_y[y, x] = y_proj

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def mercator_warp(img, focal):
    h, w = img.shape[:2]

    x, y = np.meshgrid(np.arange(w), np.arange(h))

    lon = (x - w/2) / focal
    lat = (y - h/2) / focal

    lat = np.clip(lat, -np.pi/2 + 0.01, np.pi/2 - 0.01)

    x_proj = focal * lon + w/2
    y_proj = focal * np.log(np.tan(np.pi/4 + lat/2)) + h/2

    valid_mask = (x_proj >= 0) & (x_proj < w) & (y_proj >= 0) & (y_proj < h)

    map_x = x.astype(np.float32)
    map_y = y.astype(np.float32)

    map_x[valid_mask] = x_proj[valid_mask]
    map_y[valid_mask] = y_proj[valid_mask]

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def fisheye_warp(img, focal, k1=-0.1, k2=0.05):

    h, w = img.shape[:2]

    x, y = np.meshgrid(np.arange(w), np.arange(h))

    x_norm = (x - w/2) / focal
    y_norm = (y - h/2) / focal

    r = np.sqrt(x_norm**2 + y_norm**2)

    r_distorted = r * (1 + k1 * r**2 + k2 * r**4)

    theta = np.arctan2(y_norm, x_norm)
    x_dist = r_distorted * np.cos(theta)
    y_dist = r_distorted * np.sin(theta)

    map_x = (x_dist * focal + w/2).astype(np.float32)
    map_y = (y_dist * focal + h/2).astype(np.float32)

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def stereographic_warp(img, focal):
    h, w = img.shape[:2]

    x, y = np.meshgrid(np.arange(w), np.arange(h))

    dx = (x - w/2) / focal
    dy = (y - h/2) / focal

    rho = np.sqrt(dx**2 + dy**2)
    c = 2 * np.arctan(rho / 2)

    mask = rho != 0

    map_x = np.zeros_like(x, dtype=np.float32)
    map_y = np.zeros_like(y, dtype=np.float32)

    map_x[mask] = focal * (dx[mask] * np.sin(c[mask]) / rho[mask]) + w/2
    map_y[mask] = focal * (dy[mask] * np.sin(c[mask]) / rho[mask]) + h/2

    map_x[~mask] = x[~mask]
    map_y[~mask] = y[~mask]

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def panini_warp(img, focal, d=1.0):
    h, w = img.shape[:2]

    map_x = np.zeros((h, w), dtype=np.float32)
    map_y = np.zeros((h, w), dtype=np.float32)

    for y in range(h):
        for x in range(w):
            lon = (x - w/2) / focal
            lat = (y - h/2) / focal

            S = (d + 1) / (d + np.cos(lon))
            x_proj = focal * S * np.sin(lon) + w/2
            y_proj = focal * S * np.tan(lat) + h/2

            map_x[y, x] = x_proj
            map_y[y, x] = y_proj

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def panini_portrait_warp(img, focal, d=1.0):
    h, w = img.shape[:2]

    map_x = np.zeros((h, w), dtype=np.float32)
    map_y = np.zeros((h, w), dtype=np.float32)

    for y in range(h):
        for x in range(w):
            lon = (x - w/2) / focal
            lat = (y - h/2) / focal

            S_v = (d + 1) / (d + np.cos(lat))
            S_h = (d + 1) / (d + np.cos(lon))

            x_proj = focal * S_h * np.sin(lon) + w/2
            y_proj = focal * S_v * np.tan(lat) + h/2

            map_x[y, x] = x_proj
            map_y[y, x] = y_proj

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def compressed_rectilinear_warp(img, focal, compression=0.8):
    h, w = img.shape[:2]

    map_x = np.zeros((h, w), dtype=np.float32)
    map_y = np.zeros((h, w), dtype=np.float32)

    for y in range(h):
        for x in range(w):
            dx = (x - w/2) / focal
            dy = (y - h/2) / focal

            x_proj = focal * dx * compression + w/2
            y_proj = focal * dy * compression + h/2

            map_x[y, x] = x_proj
            map_y[y, x] = y_proj

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def compressed_rectilinear_portrait_warp(img, focal, compression=0.8):
    h, w = img.shape[:2]

    map_x = np.zeros((h, w), dtype=np.float32)
    map_y = np.zeros((h, w), dtype=np.float32)

    h_compression = compression * 0.9
    v_compression = compression * 1.1

    for y in range(h):
        for x in range(w):
            dx = (x - w/2) / focal
            dy = (y - h/2) / focal

            x_proj = focal * dx * h_compression + w/2
            y_proj = focal * dy * v_compression + h/2

            map_x[y, x] = x_proj
            map_y[y, x] = y_proj

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def transverse_mercator_warp(img, focal):
    h, w = img.shape[:2]

    x, y = np.meshgrid(np.arange(w), np.arange(h))

    lon = (x - w/2) / focal
    lat = (y - h/2) / focal

    lon_safe = np.clip(lon, -np.pi/2 + 0.01, np.pi/2 - 0.01)

    x_proj = focal * np.log(np.tan(np.pi/4 + lon_safe/2)) + w/2
    y_proj = focal * lat + h/2

    valid_mask = (x_proj >= 0) & (x_proj < w) & (y_proj >= 0) & (y_proj < h)

    map_x = x.astype(np.float32)
    map_y = y.astype(np.float32)

    map_x[valid_mask] = x_proj[valid_mask]
    map_y[valid_mask] = y_proj[valid_mask]

    return cv2.remap(img, map_x, map_y, cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)


def blend_images(img1, img2, mask):
    result = img1.copy()
    y_coords, x_coords = np.where(mask > 0)

    for y, x in zip(y_coords, x_coords):
        if result[y, x].sum() == 0:
            result[y, x] = img2[y, x]
        else:
            result[y, x] = (result[y, x].astype(np.float32) * 0.5 +
                            img2[y, x].astype(np.float32) * 0.5).astype(np.uint8)

    return result


def feather_blend(result, imgs, H_matrices, warper_type):
    if result is None:
        return None

    h, w = result.shape[:2]
    gray = cv2.cvtColor(result, cv2.COLOR_BGR2GRAY)
    mask = cv2.threshold(gray, 1, 255, cv2.THRESH_BINARY)[1]

    if warper_type in ['Fisheye', 'Stereographic']:
        kernel_size = min(w, h) // 15
        center_x, center_y = w // 2, h // 2
        y, x = np.ogrid[:h, :w]
        dist_from_center = np.sqrt((x - center_x)**2 + (y - center_y)**2)
        max_radius = min(w, h) // 2
        radial_weight = np.clip(1 - dist_from_center /
                                (max_radius + 1), 0.3, 1.0)

        if kernel_size % 2 == 0:
            kernel_size += 1
        feathered = cv2.GaussianBlur(result, (kernel_size, kernel_size), 0)

        distance = cv2.distanceTransform(mask, cv2.DIST_L2, 5)
        distance = distance / (distance.max() + 1e-8)

        for c in range(3):
            blended_channel = (
                distance * result[:, :, c] + (1 - distance) * feathered[:, :, c])
            result[:, :, c] = (blended_channel * radial_weight +
                               result[:, :, c] * (1 - radial_weight)).astype(np.uint8)

    elif warper_type in ['Cylindrical', 'Spherical']:
        if warper_type == 'Cylindrical':
            kernel_size = min(w, h) // 25
            y, x = np.ogrid[:h, :w]
            horizontal_weight = 1 - np.abs(x - w/2) / (w/2)
            directional_weight = np.clip(horizontal_weight, 0.4, 1.0)
        else:
            kernel_size = min(w, h) // 20
            y, x = np.ogrid[:h, :w]
            center_weight = 1 - \
                np.sqrt((x - w/2)**2 + (y - h/2)**2) / \
                np.sqrt((w/2)**2 + (h/2)**2)
            directional_weight = np.clip(center_weight, 0.3, 1.0)

        if kernel_size % 2 == 0:
            kernel_size += 1
        feathered = cv2.GaussianBlur(result, (kernel_size, kernel_size), 0)

        distance = cv2.distanceTransform(mask, cv2.DIST_L2, 5)
        distance = distance / (distance.max() + 1e-8)

        for c in range(3):
            blended_channel = (
                distance * result[:, :, c] + (1 - distance) * feathered[:, :, c])
            result[:, :, c] = (blended_channel * directional_weight +
                               result[:, :, c] * (1 - directional_weight)).astype(np.uint8)

    elif warper_type in ['Panini', 'PaniniPortrait']:
        if warper_type == 'PaniniPortrait':
            kernel_size = min(w, h) // 18
            y, x = np.ogrid[:h, :w]
            vertical_weight = 1 - np.abs(y - h/2) / (h/2)
            directional_weight = np.clip(vertical_weight, 0.4, 1.0)
        else:
            kernel_size = min(w, h) // 22
            y, x = np.ogrid[:h, :w]
            horizontal_weight = 1 - np.abs(x - w/2) / (w/2)
            directional_weight = np.clip(horizontal_weight, 0.4, 1.0)

        if kernel_size % 2 == 0:
            kernel_size += 1
        feathered = cv2.GaussianBlur(result, (kernel_size, kernel_size), 0)

        distance = cv2.distanceTransform(mask, cv2.DIST_L2, 5)
        distance = distance / (distance.max() + 1e-8)

        for c in range(3):
            blended_channel = (
                distance * result[:, :, c] + (1 - distance) * feathered[:, :, c])
            result[:, :, c] = (blended_channel * directional_weight +
                               result[:, :, c] * (1 - directional_weight)).astype(np.uint8)

    else:
        kernel_size = min(w, h) // 20
        if kernel_size % 2 == 0:
            kernel_size += 1
        feathered = cv2.GaussianBlur(result, (kernel_size, kernel_size), 0)

        distance = cv2.distanceTransform(mask, cv2.DIST_L2, 5)
        distance = distance / (distance.max() + 1e-8)

        for c in range(3):
            result[:, :, c] = (distance * result[:, :, c] +
                               (1 - distance) * feathered[:, :, c]).astype(np.uint8)

    return result


def multiband_blend(result, imgs, H_matrices, warper_type, num_bands=5):
    if result is None or len(imgs) < 2:
        return result

    try:
        blended_result = imgs[0].copy()

        for i in range(1, min(len(imgs), len(H_matrices) + 1)):
            if i-1 < len(H_matrices):
                H = H_matrices[i-1]

                h1, w1 = blended_result.shape[:2]
                h2, w2 = imgs[i].shape[:2]

                corners1 = np.float32(
                    [[0, 0], [0, h1], [w1, h1], [w1, 0]]).reshape(-1, 1, 2)
                corners2 = np.float32(
                    [[0, 0], [0, h2], [w2, h2], [w2, 0]]).reshape(-1, 1, 2)
                corners2_transformed = cv2.perspectiveTransform(corners2, H)

                all_corners = np.concatenate(
                    (corners1, corners2_transformed), axis=0)
                [x_min, y_min] = np.int32(
                    all_corners.min(axis=0).ravel() - 0.5)
                [x_max, y_max] = np.int32(
                    all_corners.max(axis=0).ravel() + 0.5)

                translation = np.array(
                    [[1, 0, -x_min], [0, 1, -y_min], [0, 0, 1]])

                canvas_size = (x_max - x_min, y_max - y_min)
                img1_warped = cv2.warpPerspective(
                    blended_result, translation, canvas_size)
                img2_warped = cv2.warpPerspective(
                    imgs[i], translation.dot(H), canvas_size)

                mask1 = create_mask_for_blending(img1_warped)
                mask2 = create_mask_for_blending(img2_warped)

                blended_result = laplacian_pyramid_blend_aware(
                    img1_warped, img2_warped, mask1, mask2, num_bands, warper_type)

        return blended_result

    except Exception as e:
        print(f"Error in multi-band blending: {e}")
        return result


def build_gaussian_pyramid(img, levels):
    pyramid = [img.copy()]
    current = img.copy()

    for i in range(levels - 1):
        current = cv2.pyrDown(current)
        pyramid.append(current)

    return pyramid


def build_laplacian_pyramid(img, levels):
    gaussian_pyr = build_gaussian_pyramid(img, levels)
    laplacian_pyr = []

    for i in range(levels - 1):
        upsampled = cv2.pyrUp(gaussian_pyr[i + 1],
                              dstsize=(gaussian_pyr[i].shape[1], gaussian_pyr[i].shape[0]))
        laplacian = gaussian_pyr[i] - upsampled
        laplacian_pyr.append(laplacian)

    laplacian_pyr.append(gaussian_pyr[-1])

    return laplacian_pyr


def reconstruct_from_laplacian_pyramid(laplacian_pyr):
    current = laplacian_pyr[-1].copy()

    for i in range(len(laplacian_pyr) - 2, -1, -1):
        target_shape = (laplacian_pyr[i].shape[1], laplacian_pyr[i].shape[0])
        upsampled = cv2.pyrUp(current, dstsize=target_shape)

        current = upsampled + laplacian_pyr[i]

    return current


def create_mask_for_blending(img):
    if len(img.shape) == 3:
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    else:
        gray = img

    mask = cv2.threshold(gray, 1, 255, cv2.THRESH_BINARY)[1]
    return mask


def create_blend_mask_for_multiband(mask1, mask2, feather_size=50):
    overlap = cv2.bitwise_and(mask1, mask2)

    if np.sum(overlap) == 0:
        return mask2

    dist1 = cv2.distanceTransform(mask1, cv2.DIST_L2, 5)
    dist2 = cv2.distanceTransform(mask2, cv2.DIST_L2, 5)

    blend_mask = np.zeros_like(mask1, dtype=np.float32)

    overlap_bool = overlap > 0

    total_dist = dist1 + dist2

    total_dist[total_dist == 0] = 1

    weight2 = dist2 / total_dist

    if feather_size > 0:
        kernel = cv2.getStructuringElement(
            cv2.MORPH_ELLIPSE, (feather_size, feather_size))
        weight2 = cv2.morphologyEx(weight2, cv2.MORPH_CLOSE, kernel)
        weight2 = cv2.GaussianBlur(
            weight2, (feather_size//2*2+1, feather_size//2*2+1), feather_size/6)

    blend_mask[overlap_bool] = weight2[overlap_bool] * 255
    blend_mask[mask2 > mask1] = 255

    return blend_mask.astype(np.uint8)


def stitch_radial_aware(warped_imgs, H_list, warper_type):
    if len(warped_imgs) == 0:
        return None

    result = warped_imgs[0].copy()
    h, w = result.shape[:2]
    center_x, center_y = w // 2, h // 2

    for i in range(1, len(warped_imgs)):
        if i-1 < len(H_list):
            H = H_list[i-1]
            rotation_angle = extract_rotation_from_homography(H)
            translation = extract_translation_from_homography(H)

            if warper_type == 'Fisheye':
                rotation_scaling = 0.8
                translation_scaling = 0.2
            elif warper_type == 'Stereographic':
                rotation_scaling = 1.1
                translation_scaling = 0.4
            else:
                rotation_scaling = 1.0
                translation_scaling = 0.3

            scaled_rotation = rotation_angle * rotation_scaling
            scaled_translation = (
                translation[0] * translation_scaling, translation[1] * translation_scaling)

            transformed_img = apply_radial_transform(
                warped_imgs[i], scaled_rotation, scaled_translation, (center_x, center_y))

            result = blend_radial_aware(result, transformed_img, warper_type)

    return result


def stitch_curved_aware(warped_imgs, H_list, warper_type, focal):
    if len(warped_imgs) == 0:
        return None

    total_angle = estimate_total_angular_coverage(H_list, focal)

    if warper_type == 'Cylindrical':
        canvas_width = int(focal * total_angle * 1.4)
        height_multiplier = 1.1
    elif warper_type == 'Spherical':
        canvas_width = int(focal * total_angle * 1.2)
        height_multiplier = 1.3
    else:
        canvas_width = int(focal * total_angle * 1.3)
        height_multiplier = 1.2

    canvas_height = int(max(img.shape[0]
                        for img in warped_imgs) * height_multiplier)
    canvas = np.zeros((canvas_height, canvas_width, 3), dtype=np.uint8)

    current_position = canvas_width // 2

    for i, warped_img in enumerate(warped_imgs):
        if i == 0:
            offset = 0
        else:
            H = H_list[i-1] if i-1 < len(H_list) else np.eye(3)
            offset = extract_curved_offset(H, focal, warper_type)
            current_position += offset

        place_in_curved_canvas(warped_img, canvas, current_position)

    return canvas


def stitch_hybrid_aware(warped_imgs, H_list, warper_type, focal):
    if len(warped_imgs) == 0:
        return None

    total_width = sum(img.shape[1] for img in warped_imgs)
    max_height = max(img.shape[0] for img in warped_imgs)

    if warper_type == 'PaniniPortrait':
        canvas_width = int(total_width * 1.2)
        canvas_height = int(max_height * 1.6)
    else:
        canvas_width = int(total_width * 1.5)
        canvas_height = int(max_height * 1.1)

    canvas = np.zeros((canvas_height, canvas_width, 3), dtype=np.uint8)
    current_offset = canvas_width // 2

    for i, warped_img in enumerate(warped_imgs):
        if i > 0 and i-1 < len(H_list):
            H = H_list[i-1]
            offset_increment = calculate_panini_offset_from_homography(
                H, focal)
            current_offset += offset_increment

        place_in_panini_canvas(warped_img, canvas, current_offset, warper_type)

    return canvas


def stitch_geographic_aware(warped_imgs, H_list, warper_type, focal):
    if len(warped_imgs) == 0:
        return None

    if warper_type == 'Mercator':
        total_angle = estimate_total_angular_coverage(H_list, focal)
        canvas_width = int(focal * total_angle * 1.6)
        canvas_height = int(max(img.shape[0] for img in warped_imgs) * 1.4)

        canvas = np.zeros((canvas_height, canvas_width, 3), dtype=np.uint8)
        current_position = canvas_width // 2

        for i, warped_img in enumerate(warped_imgs):
            if i == 0:
                offset = 0
            else:
                H = H_list[i-1] if i-1 < len(H_list) else np.eye(3)
                tx = H[0, 2]
                ty = H[1, 2]
                mercator_scaling = focal / (focal + abs(ty) + 1)
                offset = int(tx * mercator_scaling * 0.9)
                current_position += offset

            place_in_mercator_canvas(
                warped_img, canvas, current_position, 'Mercator')

    elif warper_type == 'TransverseMercator':
        total_angle = estimate_total_angular_coverage(H_list, focal)
        canvas_width = int(focal * total_angle * 1.3)
        canvas_height = int(max(img.shape[0] for img in warped_imgs) * 1.5)

        canvas = np.zeros((canvas_height, canvas_width, 3), dtype=np.uint8)
        current_position = canvas_width // 2

        for i, warped_img in enumerate(warped_imgs):
            if i == 0:
                offset = 0
            else:
                H = H_list[i-1] if i-1 < len(H_list) else np.eye(3)
                tx = H[0, 2]
                ty = H[1, 2]
                transverse_scaling = focal / (focal + abs(tx) + 1)
                offset = int(ty * transverse_scaling * 0.85)
                current_position += offset

            place_in_mercator_canvas(
                warped_img, canvas, current_position, 'TransverseMercator')
    else:
        total_angle = estimate_total_angular_coverage(H_list, focal)
        canvas_width = int(focal * total_angle * 1.4)
        canvas_height = int(max(img.shape[0] for img in warped_imgs) * 1.3)

        canvas = np.zeros((canvas_height, canvas_width, 3), dtype=np.uint8)
        current_position = canvas_width // 2

        for i, warped_img in enumerate(warped_imgs):
            if i == 0:
                offset = 0
            else:
                H = H_list[i-1] if i-1 < len(H_list) else np.eye(3)
                tx = H[0, 2]
                ty = H[1, 2]
                # Configuración genérica para proyecciones geográficas
                geographic_factor = focal / (focal + abs(tx) + abs(ty) + 1)
                offset = int((tx + ty * 0.3) * geographic_factor * 0.8)
                current_position += offset

            place_in_mercator_canvas(
                warped_img, canvas, current_position, warper_type)

    return canvas


def place_in_mercator_canvas(warped_img, canvas, position, projection_type):
    h_img, w_img = warped_img.shape[:2]
    h_canvas, w_canvas = canvas.shape[:2]

    start_x = max(0, position - w_img // 2)
    end_x = min(w_canvas, start_x + w_img)

    if projection_type == 'Mercator':
        vertical_adjustment = int((h_canvas - h_img) * 0.45)
    elif projection_type == 'TransverseMercator':
        vertical_adjustment = int((h_canvas - h_img) * 0.4)
    else:
        vertical_adjustment = int((h_canvas - h_img) * 0.5)

    start_y = max(0, vertical_adjustment)
    end_y = min(h_canvas, start_y + h_img)

    if start_x < end_x and start_y < end_y:
        src_start_x = max(0, -start_x + position - w_img // 2)
        src_end_x = min(w_img, src_start_x + (end_x - start_x))

        canvas_region = canvas[start_y:end_y, start_x:end_x]
        img_region = warped_img[:end_y-start_y, src_start_x:src_end_x]

        mask = np.any(img_region > 0, axis=2)
        existing_mask = np.any(canvas_region > 0, axis=2)
        overlap = mask & existing_mask

        canvas_region[mask & ~
                      existing_mask] = img_region[mask & ~existing_mask]

        if np.any(overlap):
            if projection_type in ['Mercator', 'TransverseMercator']:
                for y_idx in range(overlap.shape[0]):
                    for x_idx in range(overlap.shape[1]):
                        if overlap[y_idx, x_idx]:
                            lat_factor = abs(
                                y_idx - overlap.shape[0] // 2) / (overlap.shape[0] // 2)
                            blend_factor = 0.4 + 0.3 * lat_factor

                            for c in range(3):
                                canvas_region[y_idx, x_idx, c] = (
                                    canvas_region[y_idx, x_idx, c] * (1 - blend_factor) +
                                    img_region[y_idx, x_idx, c] * blend_factor
                                ).astype(np.uint8)
            else:
                for c in range(3):
                    canvas_region[:, :, c][overlap] = (
                        canvas_region[:, :, c][overlap] * 0.6 +
                        img_region[:, :, c][overlap] * 0.4
                    ).astype(np.uint8)


def extract_rotation_from_homography(H):
    return np.arctan2(H[1, 0], H[0, 0])


def extract_translation_from_homography(H):
    return H[0, 2], H[1, 2]


def apply_radial_transform(img, rotation, translation, center):
    h, w = img.shape[:2]

    rotation_matrix = cv2.getRotationMatrix2D(
        center, np.degrees(rotation), 1.0)

    rotation_matrix[0, 2] += translation[0] * 0.3
    rotation_matrix[1, 2] += translation[1] * 0.3

    return cv2.warpAffine(img, rotation_matrix, (w, h), borderMode=cv2.BORDER_REFLECT)


def blend_radial_aware(img1, img2, warper_type):
    h, w = img1.shape[:2]
    center_x, center_y = w // 2, h // 2

    y, x = np.ogrid[:h, :w]
    dist_from_center = np.sqrt((x - center_x)**2 + (y - center_y)**2)
    max_radius = min(w, h) // 2

    if warper_type == 'Fisheye':
        radial_weight = np.clip(
            1 - (dist_from_center / (max_radius + 1))**0.8, 0, 1)
        edge_softness = 0.7
        center_preference = 0.6
    elif warper_type == 'Stereographic':
        radial_weight = np.clip(
            1 - (dist_from_center / (max_radius + 1))**1.2, 0, 1)
        edge_softness = 0.5
        center_preference = 0.8
    else:
        radial_weight = np.clip(1 - dist_from_center / (max_radius + 1), 0, 1)
        edge_softness = 0.6
        center_preference = 0.7

    mask2 = np.any(img2 > 0, axis=2)
    mask1 = np.any(img1 > 0, axis=2)

    result = img1.copy()
    blend_region = mask2 & mask1

    angles = np.arctan2(y - center_y, x - center_x)
    angular_weight = (np.cos(angles * 2) * 0.1 + 1) * radial_weight

    for c in range(3):
        weight = angular_weight[blend_region] * edge_softness + \
            center_preference * radial_weight[blend_region]
        weight = np.clip(weight, 0, 1)

        result[:, :, c][blend_region] = (
            img1[:, :, c][blend_region] * weight +
            img2[:, :, c][blend_region] * (1 - weight)
        ).astype(np.uint8)

    only_img2 = mask2 & ~mask1
    result[only_img2] = img2[only_img2]

    return result


def estimate_total_angular_coverage(H_list, focal):
    total_angle = 0
    for H in H_list:
        tx = H[0, 2]
        angle = abs(tx / focal)
        total_angle += angle
    return max(total_angle, np.pi/2)


def extract_curved_offset(H, focal, warper_type):
    tx = H[0, 2]
    ty = H[1, 2]
    rotation_component = np.arctan2(H[1, 0], H[0, 0])

    if warper_type == 'Cylindrical':
        angular_displacement = np.arctan2(tx, focal)
        linear_offset = focal * np.sin(angular_displacement)
        curvature_factor = 0.85
        vertical_correction = ty * 0.1
        rotation_correction = rotation_component * focal * 0.05
        return int(linear_offset * curvature_factor + vertical_correction + rotation_correction)

    elif warper_type == 'Spherical':
        angular_displacement_h = np.arctan2(tx, focal)
        angular_displacement_v = np.arctan2(ty, focal)

        linear_offset_h = focal * np.sin(angular_displacement_h)
        linear_offset_v = focal * np.sin(angular_displacement_v)

        combined_offset = np.sqrt(linear_offset_h**2 + linear_offset_v**2)
        if tx < 0:
            combined_offset = -combined_offset

        spherical_factor = 0.75
        rotation_correction = rotation_component * focal * 0.08
        return int(combined_offset * spherical_factor + rotation_correction)

    elif warper_type == 'Mercator':
        mercator_scaling = focal / (focal + abs(ty) + 1)
        geographic_offset = tx * mercator_scaling
        latitude_correction = ty * 0.2
        return int(geographic_offset * 0.9 + latitude_correction)

    elif warper_type == 'TransverseMercator':
        transverse_scaling = focal / (focal + abs(tx) + 1)
        geographic_offset = ty * transverse_scaling
        longitude_correction = tx * 0.15
        return int(geographic_offset * 0.85 + longitude_correction)

    elif warper_type in ['Fisheye', 'Stereographic']:
        radial_distance = np.sqrt(tx**2 + ty**2)
        if radial_distance > 0:
            radial_factor = 1 / (1 + radial_distance / focal)
            offset = tx * radial_factor * 0.7
        else:
            offset = tx * 0.7
        return int(offset)

    elif warper_type in ['Panini', 'PaniniPortrait']:
        panini_factor = focal / (focal + abs(tx) * 0.1 + 1)
        if warper_type == 'PaniniPortrait':
            offset = (tx * 0.6 + ty * 0.3) * panini_factor
        else:
            offset = (tx * 0.8 + ty * 0.1) * panini_factor
        return int(offset)

    elif warper_type in ['CompressedRectilinear', 'CompressedRectilinearPortrait']:
        compression = 0.8
        if warper_type == 'CompressedRectilinearPortrait':
            h_compression = compression * 0.9
            v_compression = compression * 1.1
            offset = tx * h_compression + ty * v_compression * 0.2
        else:
            offset = (tx + ty * 0.1) * compression
        return int(offset)

    else:
        conservative_factor = focal / (focal + abs(tx) + abs(ty) + 1)
        offset = tx * conservative_factor * 0.75
        return int(offset)


def place_in_curved_canvas(warped_img, canvas, position):
    h_img, w_img = warped_img.shape[:2]
    h_canvas, w_canvas = canvas.shape[:2]

    start_x = max(0, position - w_img // 2)
    end_x = min(w_canvas, start_x + w_img)

    vertical_adjustment = int((h_canvas - h_img) * 0.48)
    start_y = max(0, vertical_adjustment)
    end_y = min(h_canvas, start_y + h_img)

    if start_x < end_x and start_y < end_y:
        src_start_x = max(0, -start_x + position - w_img // 2)
        src_end_x = min(w_img, src_start_x + (end_x - start_x))

        canvas_region = canvas[start_y:end_y, start_x:end_x]
        img_region = warped_img[:end_y-start_y, src_start_x:src_end_x]

        mask = np.any(img_region > 0, axis=2)
        existing_mask = np.any(canvas_region > 0, axis=2)
        overlap = mask & existing_mask

        canvas_region[mask & ~
                      existing_mask] = img_region[mask & ~existing_mask]

        if np.any(overlap):
            overlap_width = np.sum(overlap, axis=0)
            max_overlap = np.max(overlap_width) if np.max(
                overlap_width) > 0 else 1

            for y_idx in range(overlap.shape[0]):
                for x_idx in range(overlap.shape[1]):
                    if overlap[y_idx, x_idx]:
                        distance_weight = overlap_width[x_idx] / max_overlap
                        blend_factor = 0.3 + 0.4 * (1 - distance_weight)

                        for c in range(3):
                            canvas_region[y_idx, x_idx, c] = (
                                canvas_region[y_idx, x_idx, c] * (1 - blend_factor) +
                                img_region[y_idx, x_idx, c] * blend_factor
                            ).astype(np.uint8)


def calculate_panini_offset_from_homography(H, focal):
    tx = H[0, 2]
    ty = H[1, 2]
    rotation_angle = np.arctan2(H[1, 0], H[0, 0])

    base_offset = tx

    angular_correction = focal * np.sin(rotation_angle) * 0.3

    vertical_influence = ty * 0.15

    panini_distortion_factor = focal / (focal + abs(tx) * 0.1)

    final_offset = (base_offset + angular_correction +
                    vertical_influence) * panini_distortion_factor * 0.7

    return int(final_offset)


def place_in_panini_canvas(warped_img, canvas, offset, warper_type):
    h_img, w_img = warped_img.shape[:2]
    h_canvas, w_canvas = canvas.shape[:2]

    if warper_type == 'PaniniPortrait':
        start_x = max(0, offset - w_img // 2)
        end_x = min(w_canvas, start_x + w_img)

        vertical_center_bias = int(h_canvas * 0.45)
        start_y = max(0, vertical_center_bias - h_img // 2)
        end_y = min(h_canvas, start_y + h_img)

        if start_x < end_x and start_y < end_y:
            src_start_x = max(0, -start_x + offset - w_img // 2)
            src_end_x = min(w_img, src_start_x + (end_x - start_x))
            src_start_y = max(0, -start_y + vertical_center_bias - h_img // 2)
            src_end_y = min(h_img, src_start_y + (end_y - start_y))

            canvas_region = canvas[start_y:end_y, start_x:end_x]
            img_region = warped_img[src_start_y:src_end_y,
                                    src_start_x:src_end_x]

            mask = np.any(img_region > 0, axis=2)
            existing_mask = np.any(canvas_region > 0, axis=2)
            overlap_mask = mask & existing_mask

            y_weight = np.linspace(0.3, 1.0, img_region.shape[0])
            y_weight = np.expand_dims(y_weight, axis=1)

            canvas_region[mask & ~
                          existing_mask] = img_region[mask & ~existing_mask]

            if np.any(overlap_mask):
                for c in range(3):
                    blend_weight = y_weight[overlap_mask] * 0.6
                    canvas_region[:, :, c][overlap_mask] = (
                        canvas_region[:, :, c][overlap_mask] * (1 - blend_weight) +
                        img_region[:, :, c][overlap_mask] * blend_weight
                    ).astype(np.uint8)
    else:
        start_x = max(0, offset - w_img // 2)
        end_x = min(w_canvas, start_x + w_img)
        start_y = max(0, (h_canvas - h_img) // 2)
        end_y = min(h_canvas, start_y + h_img)

        if start_x < end_x and start_y < end_y:
            src_start_x = max(0, -start_x + offset - w_img // 2)
            src_end_x = min(w_img, src_start_x + (end_x - start_x))

            canvas_region = canvas[start_y:end_y, start_x:end_x]
            img_region = warped_img[:end_y-start_y, src_start_x:src_end_x]

            mask = np.any(img_region > 0, axis=2)
            existing_mask = np.any(canvas_region > 0, axis=2)
            overlap_mask = mask & existing_mask

            x_weight = np.linspace(0.4, 1.0, img_region.shape[1])
            x_weight = np.expand_dims(x_weight, axis=0)

            canvas_region[mask & ~
                          existing_mask] = img_region[mask & ~existing_mask]

            if np.any(overlap_mask):
                for c in range(3):
                    blend_weight = x_weight[overlap_mask] * 0.5
                    canvas_region[:, :, c][overlap_mask] = (
                        canvas_region[:, :, c][overlap_mask] * (1 - blend_weight) +
                        img_region[:, :, c][overlap_mask] * blend_weight
                    ).astype(np.uint8)


def laplacian_pyramid_blend_aware(img1, img2, mask1, mask2, num_bands, warper_type):
    h, w = img1.shape[:2]
    img2 = cv2.resize(img2, (w, h))
    mask1 = cv2.resize(mask1, (w, h))
    mask2 = cv2.resize(mask2, (w, h))

    overlap_mask = create_blend_mask_for_multiband_aware(
        mask1, mask2, warper_type)

    img1_f = img1.astype(np.float32)
    img2_f = img2.astype(np.float32)

    lap_pyr1 = build_laplacian_pyramid(img1_f, num_bands)
    lap_pyr2 = build_laplacian_pyramid(img2_f, num_bands)

    mask_pyr = build_gaussian_pyramid(
        overlap_mask.astype(np.float32), num_bands)

    blended_pyr = []
    for i in range(num_bands):
        mask_level = mask_pyr[i] / 255.0
        mask_level = np.stack([mask_level] * 3, axis=-1)

        if warper_type in ['Fisheye', 'Stereographic']:
            blended_level = blend_pyramid_level_radial(
                lap_pyr1[i], lap_pyr2[i], mask_level, warper_type)
        elif warper_type in ['Cylindrical', 'Spherical']:
            blended_level = blend_pyramid_level_curved(
                lap_pyr1[i], lap_pyr2[i], mask_level, warper_type)
        elif warper_type in ['Panini', 'PaniniPortrait']:
            blended_level = blend_pyramid_level_hybrid(
                lap_pyr1[i], lap_pyr2[i], mask_level, warper_type)
        else:
            blended_level = lap_pyr1[i] * \
                (1 - mask_level) + lap_pyr2[i] * mask_level

        blended_pyr.append(blended_level)

    result = reconstruct_from_laplacian_pyramid(blended_pyr)

    final_mask1 = (mask1 > 0) & (mask2 == 0)
    final_mask2 = (mask2 > 0) & (mask1 == 0)

    result[final_mask1] = img1_f[final_mask1]
    result[final_mask2] = img2_f[final_mask2]

    result = np.clip(result, 0, 255).astype(np.uint8)

    return result


def create_blend_mask_for_multiband_aware(mask1, mask2, warper_type, feather_size=50):
    overlap = cv2.bitwise_and(mask1, mask2)

    if np.sum(overlap) == 0:
        return mask2

    if warper_type in ['Fisheye', 'Stereographic']:
        return create_radial_blend_mask(mask1, mask2, feather_size)
    elif warper_type in ['Cylindrical', 'Spherical']:
        return create_curved_blend_mask(mask1, mask2, feather_size, warper_type)
    elif warper_type in ['Panini', 'PaniniPortrait']:
        return create_hybrid_blend_mask(mask1, mask2, feather_size, warper_type)
    else:
        return create_blend_mask_for_multiband(mask1, mask2, feather_size)


def create_radial_blend_mask(mask1, mask2, feather_size):
    h, w = mask1.shape[:2]
    center_x, center_y = w // 2, h // 2

    overlap = cv2.bitwise_and(mask1, mask2)
    if np.sum(overlap) == 0:
        return mask2

    y, x = np.ogrid[:h, :w]
    dist_from_center = np.sqrt((x - center_x)**2 + (y - center_y)**2)

    dist1 = cv2.distanceTransform(mask1, cv2.DIST_L2, 5)
    dist2 = cv2.distanceTransform(mask2, cv2.DIST_L2, 5)

    radial_weight = dist_from_center / (np.max(dist_from_center) + 1)

    blend_mask = np.zeros_like(mask1, dtype=np.float32)
    overlap_bool = overlap > 0

    total_dist = dist1 + dist2
    total_dist[total_dist == 0] = 1

    weight2 = dist2 / total_dist

    weight2[overlap_bool] *= (1 + radial_weight[overlap_bool] * 0.3)
    weight2 = np.clip(weight2, 0, 1)

    if feather_size > 0:
        kernel = cv2.getStructuringElement(
            cv2.MORPH_ELLIPSE, (feather_size, feather_size))
        weight2 = cv2.morphologyEx(weight2, cv2.MORPH_CLOSE, kernel)
        weight2 = cv2.GaussianBlur(
            weight2, (feather_size//2*2+1, feather_size//2*2+1), feather_size/6)

    blend_mask[overlap_bool] = weight2[overlap_bool] * 255
    blend_mask[mask2 > mask1] = 255

    return blend_mask.astype(np.uint8)


def create_curved_blend_mask(mask1, mask2, feather_size, warper_type):
    h, w = mask1.shape[:2]

    overlap = cv2.bitwise_and(mask1, mask2)
    if np.sum(overlap) == 0:
        return mask2

    dist1 = cv2.distanceTransform(mask1, cv2.DIST_L2, 5)
    dist2 = cv2.distanceTransform(mask2, cv2.DIST_L2, 5)

    blend_mask = np.zeros_like(mask1, dtype=np.float32)
    overlap_bool = overlap > 0

    total_dist = dist1 + dist2
    total_dist[total_dist == 0] = 1

    weight2 = dist2 / total_dist

    if warper_type == 'Cylindrical':
        y_coords, x_coords = np.ogrid[:h, :w]
        horizontal_bias = np.abs(x_coords - w/2) / (w/2)
        weight2 *= (1 + horizontal_bias * 0.2)
    elif warper_type == 'Spherical':
        y_coords, x_coords = np.ogrid[:h, :w]
        center_bias = np.sqrt((x_coords - w/2)**2 + (y_coords - h/2)**2)
        center_bias = center_bias / np.max(center_bias)
        weight2 *= (1 + center_bias * 0.15)

    weight2 = np.clip(weight2, 0, 1)

    if feather_size > 0:
        kernel = cv2.getStructuringElement(
            cv2.MORPH_ELLIPSE, (feather_size, feather_size))
        weight2 = cv2.morphologyEx(weight2, cv2.MORPH_CLOSE, kernel)
        weight2 = cv2.GaussianBlur(
            weight2, (feather_size//2*2+1, feather_size//2*2+1), feather_size/6)

    blend_mask[overlap_bool] = weight2[overlap_bool] * 255
    blend_mask[mask2 > mask1] = 255

    return blend_mask.astype(np.uint8)


def create_hybrid_blend_mask(mask1, mask2, feather_size, warper_type):
    h, w = mask1.shape[:2]

    overlap = cv2.bitwise_and(mask1, mask2)
    if np.sum(overlap) == 0:
        return mask2

    dist1 = cv2.distanceTransform(mask1, cv2.DIST_L2, 5)
    dist2 = cv2.distanceTransform(mask2, cv2.DIST_L2, 5)

    blend_mask = np.zeros_like(mask1, dtype=np.float32)
    overlap_bool = overlap > 0

    total_dist = dist1 + dist2
    total_dist[total_dist == 0] = 1

    weight2 = dist2 / total_dist

    y_coords, x_coords = np.ogrid[:h, :w]

    if warper_type == 'PaniniPortrait':
        vertical_bias = np.abs(y_coords - h/2) / (h/2)
        weight2 *= (1 + vertical_bias * 0.25)
    else:
        horizontal_bias = np.abs(x_coords - w/2) / (w/2)
        weight2 *= (1 + horizontal_bias * 0.2)

    weight2 = np.clip(weight2, 0, 1)

    if feather_size > 0:
        kernel = cv2.getStructuringElement(
            cv2.MORPH_ELLIPSE, (feather_size, feather_size))
        weight2 = cv2.morphologyEx(weight2, cv2.MORPH_CLOSE, kernel)
        weight2 = cv2.GaussianBlur(
            weight2, (feather_size//2*2+1, feather_size//2*2+1), feather_size/6)

    blend_mask[overlap_bool] = weight2[overlap_bool] * 255
    blend_mask[mask2 > mask1] = 255

    return blend_mask.astype(np.uint8)


def blend_pyramid_level_radial(lap1, lap2, mask_level, warper_type):
    h, w = lap1.shape[:2]
    center_x, center_y = w // 2, h // 2

    y, x = np.ogrid[:h, :w]
    dist_from_center = np.sqrt((x - center_x)**2 + (y - center_y)**2)
    max_dist = np.sqrt(center_x**2 + center_y**2)

    if warper_type == 'Fisheye':
        radial_influence = 1 - (dist_from_center / (max_dist + 1))**0.6
        angular_frequency = 8
        angles = np.arctan2(y - center_y, x - center_x)
        angular_modulation = np.cos(angles * angular_frequency) * 0.05 + 1
        base_blend_strength = 0.65
        radial_influence *= angular_modulation

    elif warper_type == 'Stereographic':
        radial_influence = 1 - (dist_from_center / (max_dist + 1))**1.4

        stereographic_radius = 2 * \
            np.arctan(dist_from_center / (2 * max_dist + 1))
        stereographic_weight = np.cos(stereographic_radius) * 0.5 + 0.5
        radial_influence *= stereographic_weight
        base_blend_strength = 0.75

    else:
        radial_influence = 1 - (dist_from_center / (max_dist + 1))
        base_blend_strength = 0.7

    radial_influence = np.stack([radial_influence] * 3, axis=-1)

    effective_mask = mask_level * \
        (base_blend_strength + (1 - base_blend_strength) * radial_influence)
    effective_mask = np.clip(effective_mask, 0, 1)

    edge_regions = dist_from_center > (max_dist * 0.8)
    edge_regions_3d = np.stack([edge_regions] * 3, axis=-1)

    effective_mask[edge_regions_3d] *= 0.5

    return lap1 * (1 - effective_mask) + lap2 * effective_mask


def blend_pyramid_level_curved(lap1, lap2, mask_level, warper_type):
    h, w = lap1.shape[:2]

    if warper_type == 'Cylindrical':
        y, x = np.ogrid[:h, :w]
        horizontal_influence = 1 - np.abs(x - w/2) / (w/2)
        horizontal_influence = np.stack([horizontal_influence] * 3, axis=-1)
        effective_mask = mask_level * (0.8 + 0.2 * horizontal_influence)
    else:
        y, x = np.ogrid[:h, :w]
        center_influence = 1 - \
            np.sqrt((x - w/2)**2 + (y - h/2)**2) / np.sqrt((w/2)**2 + (h/2)**2)
        center_influence = np.stack([center_influence] * 3, axis=-1)
        effective_mask = mask_level * (0.8 + 0.2 * center_influence)

    effective_mask = np.clip(effective_mask, 0, 1)
    return lap1 * (1 - effective_mask) + lap2 * effective_mask


def blend_pyramid_level_hybrid(lap1, lap2, mask_level, warper_type):
    h, w = lap1.shape[:2]
    y, x = np.ogrid[:h, :w]

    if warper_type == 'PaniniPortrait':
        vertical_influence = 1 - np.abs(y - h/2) / (h/2)
        influence = np.stack([vertical_influence] * 3, axis=-1)
    else:
        horizontal_influence = 1 - np.abs(x - w/2) / (w/2)
        influence = np.stack([horizontal_influence] * 3, axis=-1)

    effective_mask = mask_level * (0.75 + 0.25 * influence)
    effective_mask = np.clip(effective_mask, 0, 1)

    return lap1 * (1 - effective_mask) + lap2 * effective_mask


if __name__ == "__main__":
    rutas = [
        "resources/1.JPG",
        "resources/2.JPG",
        "resources/3.JPG",
        "resources/4.JPG",
        "resources/5.JPG",
        "resources/6.JPG",
        "resources/7.JPG",
        "resources/8.JPG",
        "resources/9.JPG",
        "resources/10.JPG",
        "resources/11.JPG",
    ]

    configurations = [
        {
            "blender": "FeatherBlender",
            "features_finder": "SIFT",
            "features_matcher": "BestOf2NearestRange",
            "warper": "Fisheye",
            "output": "panorama_fisheye.jpg"
        },
        {
            "blender": "MultiBandBlender",
            "features_finder": "SIFT",
            "features_matcher": "BestOf2NearestRange",
            "warper": "Spherical",
            "output": "panorama_spherical.jpg"
        },
        {
            "blender": "FeatherBlender",
            "features_finder": "SIFT",
            "features_matcher": "BestOf2NearestRange",
            "warper": "Panini",
            "output": "panorama_panini.jpg"
        },
        {
            "blender": "MultiBandBlender",
            "features_finder": "SIFT",
            "features_matcher": "BestOf2NearestRange",
            "warper": "PaniniPortrait",
            "output": "panorama_panini_portrait.jpg"
        },
        {
            "blender": "FeatherBlender",
            "features_finder": "SIFT",
            "features_matcher": "BestOf2NearestRange",
            "warper": "CompressedRectilinear",
            "output": "panorama_compressed_rectilinear.jpg"
        },
        {
            "blender": "MultiBandBlender",
            "features_finder": "SIFT",
            "features_matcher": "BestOf2NearestRange",
            "warper": "CompressedRectilinearPortrait",
            "output": "panorama_compressed_rectilinear_portrait.jpg"
        },
        {
            "blender": "FeatherBlender",
            "features_finder": "SIFT",
            "features_matcher": "BestOf2NearestRange",
            "warper": "TransverseMercator",
            "output": "panorama_transverse_mercator.jpg"
        },

    ]

    for config in configurations:
        print(f"Creating panorama with {config['warper']} projection...")

        ok = stitch_images(
            full_path_input_image=rutas,
            blender=config["blender"],
            features_finder=config["features_finder"],
            features_matcher=config["features_matcher"],
            warper=config["warper"],
            full_path_output_image=config["output"]
        )

        if ok:
            print(f"✓ {config['warper']} panorama created: {config['output']}")
        else:
            print(f"✗ Error creating {config['warper']} panorama")
