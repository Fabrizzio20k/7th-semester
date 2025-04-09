import cv2
import mediapipe as mp
import os


def extract_faces(video_path, output_folder, max_faces=300):
    os.makedirs(output_folder, exist_ok=True)
    cap = cv2.VideoCapture(video_path)
    detector = mp.solutions.face_detection.FaceDetection(
        model_selection=0, min_detection_confidence=0.5)

    count = 0
    while count < max_faces:
        ret, frame = cap.read()
        if not ret:
            break
        results = detector.process(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
        if results.detections:
            bbox = results.detections[0].location_data.relative_bounding_box
            h, w, _ = frame.shape
            x, y = int(bbox.xmin * w), int(bbox.ymin * h)
            w_box, h_box = int(bbox.width * w), int(bbox.height * h)
            face = frame[y:y+h_box, x:x+w_box]
            face = cv2.resize(face, (128, 128))
            cv2.imwrite(f"{output_folder}/face_{count}.jpg", face)
            count += 1
    cap.release()
    print(f"Extraídas {count} caras en {output_folder}")


# Usa esto para ambos videos
extract_faces("videos/source.mp4", "data/A")
extract_faces("videos/target.mp4", "data/B")
