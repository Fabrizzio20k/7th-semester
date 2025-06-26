import cv2
import numpy as np


class ImageReplacer:
    def __init__(self):
        self.img1 = None
        self.img2 = None
        self.img1_display = None
        self.selection_points = []
        self.selecting = False
        self.sift = cv2.SIFT_create()

    def mouse_callback(self, event, x, y, flags, param):
        print(f"MOUSE: event={event}, pos=({x},{y}), flags={flags}")

        if event == cv2.EVENT_LBUTTONDOWN:
            print("CLICK DOWN")
            self.selecting = True
            self.selection_points = [(x, y)]

        elif event == cv2.EVENT_MOUSEMOVE and self.selecting:
            print(f"DRAGGING to ({x},{y})")
            self.selection_points.append((x, y))

            self.img1_display = self.img1.copy()

            for i, point in enumerate(self.selection_points):
                cv2.circle(self.img1_display, point, 3, (0, 255, 0), -1)
                if i > 0:
                    cv2.line(self.img1_display,
                             self.selection_points[i-1], point, (0, 255, 0), 2)

            cv2.imshow('selector', self.img1_display)

        elif event == cv2.EVENT_LBUTTONUP:
            print("CLICK UP")
            self.selecting = False

            if len(self.selection_points) > 3:
                points = np.array(self.selection_points, dtype=np.int32)
                hull = cv2.convexHull(points)

                self.img1_display = self.img1.copy()
                cv2.polylines(self.img1_display, [hull], True, (255, 0, 0), 3)
                cv2.imshow('selector', self.img1_display)

                print("Procesando...")
                self.process_selection(hull)

    def process_selection(self, hull):
        mask = np.zeros(self.img1.shape[:2], dtype=np.uint8)
        cv2.fillPoly(mask, [hull], 255)

        x, y, w, h = cv2.boundingRect(hull)
        roi_img1 = self.img1[y:y+h, x:x+w]
        roi_mask = mask[y:y+h, x:x+w]

        gray1 = cv2.cvtColor(roi_img1, cv2.COLOR_BGR2GRAY)
        gray2 = cv2.cvtColor(self.img2, cv2.COLOR_BGR2GRAY)

        kp1, des1 = self.sift.detectAndCompute(gray1, roi_mask)
        kp2, des2 = self.sift.detectAndCompute(gray2, None)

        if des1 is None or des2 is None or len(des1) < 5:
            print("No suficientes features")
            return

        bf = cv2.BFMatcher()
        matches = bf.knnMatch(des1, des2, k=2)

        good_matches = []
        for match_pair in matches:
            if len(match_pair) == 2:
                m, n = match_pair
                if m.distance < 0.7 * n.distance:
                    good_matches.append(m)

        if len(good_matches) >= 4:
            self.replace_region(hull, good_matches, kp1, kp2, x, y, w, h)

    def replace_region(self, hull, matches, kp1, kp2, x, y, w, h):
        matched_points = [kp2[m.trainIdx].pt for m in matches]
        center_x = int(np.mean([pt[0] for pt in matched_points]))
        center_y = int(np.mean([pt[1] for pt in matched_points]))

        x2 = max(0, center_x - w//2)
        y2 = max(0, center_y - h//2)
        x2_end = min(self.img2.shape[1], x2 + w)
        y2_end = min(self.img2.shape[0], y2 + h)

        replacement = self.img2[y2:y2_end, x2:x2_end]
        replacement = cv2.resize(replacement, (w, h))

        result = self.img1.copy()
        mask = np.zeros(self.img1.shape[:2], dtype=np.uint8)
        cv2.fillPoly(mask, [hull], 255)

        roi_result = result[y:y+h, x:x+w]
        roi_mask = mask[y:y+h, x:x+w]
        roi_result[roi_mask == 255] = replacement[roi_mask == 255]

        cv2.imshow('resultado', result)

    def run(self, img1_path, img2_path):
        self.img1 = cv2.imread(img1_path)
        self.img2 = cv2.imread(img2_path)

        if self.img1 is None or self.img2 is None:
            print("Error cargando imágenes")
            return

        # Resize más conservador para Mac
        h1, w1 = self.img1.shape[:2]
        if w1 > 500:
            scale = 500 / w1
            self.img1 = cv2.resize(self.img1, (500, int(h1 * scale)))

        h2, w2 = self.img2.shape[:2]
        if w2 > 500:
            scale = 500 / w2
            self.img2 = cv2.resize(self.img2, (500, int(h2 * scale)))

        self.img1_display = self.img1.copy()

        # Crear ventanas simples
        cv2.namedWindow('selector', cv2.WINDOW_NORMAL)
        cv2.namedWindow('source', cv2.WINDOW_NORMAL)

        cv2.imshow('selector', self.img1)
        cv2.imshow('source', self.img2)

        # Registrar callback DESPUÉS de mostrar
        cv2.setMouseCallback('selector', self.mouse_callback)

        print("=== INSTRUCCIONES ===")
        print("1. CLICK Y ARRASTRA en ventana 'selector'")
        print("2. Si no funciona, prueba hacer click en la ventana primero")
        print("3. 'r' = reset, 'q' = salir")

        # Verificar que el callback se registró
        print("Callback registrado. Mueve el mouse sobre la imagen...")

        while True:
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                break
            elif key == ord('r'):
                print("RESET")
                self.selection_points = []
                self.selecting = False
                self.img1_display = self.img1.copy()
                cv2.imshow('selector', self.img1_display)

        cv2.destroyAllWindows()


if __name__ == "__main__":
    replacer = ImageReplacer()
    replacer.run('a1.jpeg', 'a2.jpeg')
