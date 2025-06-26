import cv2
from ultralytics import YOLO
import numpy as np


def count_people_cars_and_bikes(full_path_input_video):
    m = YOLO('yolov7.pt')

    v = cv2.VideoCapture(full_path_input_video)

    fps = int(v.get(cv2.CAP_PROP_FPS))
    tc = int(v.get(cv2.CAP_PROP_FRAME_COUNT))

    s = max(1, fps // 5)

    p_ids = set()
    b_ids = set()
    c_ids = set()

    f = 0
    while True:
        ret, fr = v.read()
        if not ret:
            break

        if f % s == 0:
            h, w = fr.shape[:2]
            if w > 1280:
                nw = 1280
                nh = int(h * 1280 / w)
                fr = cv2.resize(fr, (nw, nh))

            r = m(fr, conf=0.5, verbose=False)[0]

            if r.boxes is not None:
                for bx in r.boxes:
                    cl = int(bx.cls)
                    cf = float(bx.conf)

                    if cf > 0.5:
                        x1, y1, x2, y2 = bx.xyxy[0].tolist()
                        cx = (x1 + x2) / 2
                        cy = (y1 + y2) / 2

                        id_obj = f"{cl}_{int(cx/50)}_{int(cy/50)}"

                        if cl == 0:
                            p_ids.add(id_obj)
                        elif cl == 1:
                            b_ids.add(id_obj)
                        elif cl == 2:
                            c_ids.add(id_obj)

        f += 1

        if f % 500 == 0:
            print(f"Frame {f}/{tc}")

    v.release()

    np = len(p_ids)
    nb = len(b_ids)
    nc = len(c_ids)

    return [np, nb, nc]


if __name__ == "__main__":

    video_path = "video.mp4"
    result = count_people_cars_and_bikes(video_path)
    print(f"People: {result[0]}, Bikes: {result[1]}, Cars: {result[2]}")
