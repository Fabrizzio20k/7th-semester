import cv2
import numpy as np

marker_image = cv2.imread('marker.png', cv2.IMREAD_GRAYSCALE)
orb = cv2.ORB_create()

kp_marker, des_marker = orb.detectAndCompute(marker_image, None)
bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)

camera_matrix = np.array([[800, 0, 320],
                          [0, 800, 240],
                          [0, 0, 1]], dtype=np.float32)

dist_coeffs = np.zeros((4, 1))

marker_size = 100
marker_3d_points = np.array([
    [-marker_size/2, -marker_size/2, 0],
    [marker_size/2, -marker_size/2, 0],
    [marker_size/2, marker_size/2, 0],
    [-marker_size/2, marker_size/2, 0]
], dtype=np.float32)

cube_size = marker_size * 0.6
cube_height = marker_size * 0.8
cube_3d_points = np.array([
    [-cube_size/2, -cube_size/2, cube_height/2],
    [cube_size/2, -cube_size/2, cube_height/2],
    [cube_size/2, cube_size/2, cube_height/2],
    [-cube_size/2, cube_size/2, cube_height/2],
    [-cube_size/2, -cube_size/2, cube_height + cube_height/2],
    [cube_size/2, -cube_size/2, cube_height + cube_height/2],
    [cube_size/2, cube_size/2, cube_height + cube_height/2],
    [-cube_size/2, cube_size/2, cube_height + cube_height/2]
], dtype=np.float32)

def draw_cube(img, cube_points_2d):
    cube_points_2d = cube_points_2d.astype(int)
    
    center = np.mean(cube_points_2d, axis=0)
    
    faces = [
        ([0, 1, 2, 3], (100, 100, 255), "base"),
        ([4, 5, 6, 7], (0, 255, 255), "top"),
        ([0, 1, 5, 4], (255, 100, 100), "front"),
        ([2, 3, 7, 6], (255, 200, 100), "back"),
        ([1, 2, 6, 5], (100, 255, 100), "right"),
        ([3, 0, 4, 7], (200, 100, 255), "left")
    ]
    
    visible_faces = []
    for vertices, color, name in faces:
        p1 = cube_points_2d[vertices[0]]
        p2 = cube_points_2d[vertices[1]]
        p3 = cube_points_2d[vertices[2]]
        
        v1 = p2 - p1
        v2 = p3 - p1
        
        cross_z = v1[0] * v2[1] - v1[1] * v2[0]
        
        if cross_z > 0:
            visible_faces.append((vertices, color, name))
    
    for vertices, color, name in visible_faces:
        pts = np.array([cube_points_2d[vertices]], dtype=np.int32)
        cv2.fillPoly(img, pts, color)
    
    edges = [
        (0, 1), (1, 2), (2, 3), (3, 0),
        (4, 5), (5, 6), (6, 7), (7, 4),
        (0, 4), (1, 5), (2, 6), (3, 7)
    ]
    
    for start, end in edges:
        cv2.line(img, tuple(cube_points_2d[start]), tuple(cube_points_2d[end]), (0, 0, 0), 2)

def order_marker_corners(corners):
    center = np.mean(corners, axis=0)
    angles = np.arctan2(corners[:, 1] - center[1], corners[:, 0] - center[0])
    sorted_indices = np.argsort(angles)
    ordered_corners = corners[sorted_indices]
    return ordered_corners

cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break
    
    gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    kp_frame, des_frame = orb.detectAndCompute(gray_frame, None)
    
    if des_frame is not None:
        matches = bf.match(des_marker, des_frame)
        matches = sorted(matches, key=lambda x: x.distance)
        
        if len(matches) > 120:
            src_pts = np.float32([kp_marker[m.queryIdx].pt for m in matches]).reshape(-1, 1, 2)
            dst_pts = np.float32([kp_frame[m.trainIdx].pt for m in matches]).reshape(-1, 1, 2)
            
            matrix, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC, 5.0)
            
            if matrix is not None:
                h, w = marker_image.shape
                marker_corners_2d = np.float32([[0, 0], [w, 0], [w, h], [0, h]]).reshape(-1, 1, 2)
                detected_corners = cv2.perspectiveTransform(marker_corners_2d, matrix)
                detected_corners = detected_corners.reshape(-1, 2)
                
                ordered_corners = order_marker_corners(detected_corners)
                
                success, rvec, tvec = cv2.solvePnP(
                    marker_3d_points, 
                    ordered_corners, 
                    camera_matrix, 
                    dist_coeffs
                )
                
                if success:
                    cube_points_2d, _ = cv2.projectPoints(
                        cube_3d_points, 
                        rvec, 
                        tvec, 
                        camera_matrix, 
                        dist_coeffs
                    )
                    cube_points_2d = cube_points_2d.reshape(-1, 2)
                    
                    draw_cube(frame, cube_points_2d)
                
                frame = cv2.polylines(frame, [np.int32(detected_corners)], True, (0, 255, 255), 2)
    
    cv2.imshow('AR Cube', frame)
    
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()