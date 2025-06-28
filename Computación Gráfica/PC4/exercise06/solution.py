import cv2
import numpy as np
from ultralytics import YOLO, SAM
import time


def highlight_people_cars_and_bikes(full_path_input_image, color_scale_image, color_scale_people, color_scale_cars, color_scale_bikes, full_path_output_image):
    i = cv2.imread(full_path_input_image)
    h, w = i.shape[:2]

    yl = YOLO('yolov8l.pt')
    sm = SAM('sam2.1_t.pt')

    r = yl(i, conf=0.3, verbose=False)[0]

    o = np.full((h, w, 3), color_scale_image, dtype=np.uint8)

    if r.boxes is not None:
        bxs = []
        cls = []

        for b in r.boxes:
            c = int(b.cls)
            if c <= 2:
                x1, y1, x2, y2 = b.xyxy[0].tolist()
                bxs.append([x1, y1, x2, y2])
                cls.append(c)

        if bxs:
            rs = sm(i, bboxes=bxs, verbose=False)

            cols = [color_scale_people, color_scale_bikes, color_scale_cars]

            if len(rs) > 0 and rs[0].masks is not None:
                masks = rs[0].masks.data
                for j in range(len(bxs)):
                    if j < len(masks):
                        m = masks[j].cpu().numpy()
                        o[m > 0.5] = cols[cls[j]]

    cv2.imwrite(full_path_output_image, o)


if __name__ == "__main__":
    # La primera vez demora mas de 5 segundos al cargar los modelos por primera vez, luego es mas rapido
    start = time.time()
    highlight_people_cars_and_bikes(
        full_path_input_image='p1.jpeg',
        color_scale_image=(255, 255, 255),
        color_scale_people=(0, 255, 0),
        color_scale_cars=(0, 0, 255),
        color_scale_bikes=(255, 0, 0),
        full_path_output_image='output.jpg'
    )
    end = time.time()
    print(f"Execution time: {end - start:.2f} seconds")
