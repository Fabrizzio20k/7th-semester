import cv2
from ultralytics import YOLO
from time import time


def count_people_cars_and_bikes(full_path_input_video):
    model = YOLO('yolov8l.pt')
    cap = cv2.VideoCapture(full_path_input_video)

    fps = int(cap.get(cv2.CAP_PROP_FPS))
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

    skip_frames = max(1, fps // 4)

    people_seen = set()
    bikes_seen = set()
    cars_seen = set()

    frame_num = 0

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        if frame_num % skip_frames == 0:
            height, width = frame.shape[:2]
            if width > 1280:
                new_width = 1280
                new_height = int(height * 1280 / width)
                frame = cv2.resize(frame, (new_width, new_height))

            results = model(frame, conf=0.65, verbose=False)[0]

            if results.boxes is not None:
                for box in results.boxes:
                    class_id = int(box.cls)
                    confidence = float(box.conf)

                    if confidence > 0.65:
                        x1, y1, x2, y2 = box.xyxy[0].tolist()

                        area = (x2 - x1) * (y2 - y1)
                        min_areas = {0: 1000, 1: 1500, 2: 2500}

                        if area >= min_areas.get(class_id, 1000):
                            center_x = (x1 + x2) / 2
                            center_y = (y1 + y2) / 2

                            grid_sizes = {0: 200, 1: 250, 2: 300}
                            grid_size = grid_sizes.get(class_id, 200)

                            grid_x = int(center_x / grid_size)
                            grid_y = int(center_y / grid_size)

                            time_window = frame_num // (fps * 3)

                            obj_id = f"{grid_x}_{grid_y}_{time_window}"

                            if class_id == 0:  # Persona
                                people_seen.add(obj_id)
                            elif class_id == 1:  # Bicicleta
                                bikes_seen.add(obj_id)
                            elif class_id == 2:  # Carro
                                cars_seen.add(obj_id)

        frame_num += 1

        if frame_num % 500 == 0:
            print(
                f"Frame {frame_num}/{total_frames} - People: {len(people_seen)}, Bikes: {len(bikes_seen)}, Cars: {len(cars_seen)}")

    cap.release()
    return [len(people_seen), len(bikes_seen), len(cars_seen)]


if __name__ == "__main__":

    # El tiempo es mayor la primera vez que se ejecuta, ya que el modelo se carga y se inicializa, luego es más rápido
    start_time = time()

    video_path = "video.mp4"
    result = count_people_cars_and_bikes(video_path)
    print(
        f"Resultado final - People: {result[0]}, Bikes: {result[1]}, Cars: {result[2]}")

    end_time = time()
    print(f"Tiempo de ejecución: {end_time - start_time:.2f} segundos")
