from ultralytics import YOLO
from time import time


def count_people_cars_and_bikes(full_path_input_video):
    m = YOLO("yolov8l.pt")

    tc = {0: set(), 1: set(), 2: set()}

    res = m.track(
        full_path_input_video,
        persist=True,
        tracker="bytetrack.yaml",
        verbose=False,
        stream=True,
    )

    for r in res:
        if r.boxes is not None and r.boxes.id is not None:
            ids = r.boxes.id.cpu().numpy().astype(int)
            cls = r.boxes.cls.cpu().numpy().astype(int)
            conf = r.boxes.conf.cpu().numpy()

            for tid, c, cf in zip(ids, cls, conf):
                if c in [0, 1, 2] and cf > 0.5:
                    tc[c].add(tid)

    return [len(tc[0]), len(tc[1]), len(tc[2])]


if __name__ == "__main__":
    full_path_input_video = "video.mp4"

    start_time = time()
    people, cars, bikes = count_people_cars_and_bikes(full_path_input_video)
    end_time = time()

    print(f"Processing time: {end_time - start_time:.2f} seconds")

    print(f"People: {people}, Cars: {cars}, Bikes: {bikes}")
