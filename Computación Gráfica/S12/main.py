import cv2
import numpy as np
import os
import pickle


class DefectClassifierLowMemory:
    def __init__(self, batch_size=50):
        self.sift = cv2.SIFT_create()
        self.batch_size = batch_size
        self.def_centroid = None
        self.ok_centroid = None

    def extract_features(self, image_path):
        img = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
        if img is None:
            return None
        keypoints, descriptors = self.sift.detectAndCompute(img, None)
        if descriptors is None:
            return np.zeros(128, dtype=np.float32)
        return np.mean(descriptors, axis=0).astype(np.float32)

    def process_folder_batch(self, folder_path):
        files = [f for f in os.listdir(
            folder_path) if f.lower().endswith(('.jpg', '.jpeg', '.png'))]
        sum_features = np.zeros(128, dtype=np.float64)
        count = 0

        for i in range(0, len(files), self.batch_size):
            batch_files = files[i:i+self.batch_size]
            batch_features = []

            for filename in batch_files:
                feat = self.extract_features(
                    os.path.join(folder_path, filename))
                if feat is not None:
                    batch_features.append(feat)

            if batch_features:
                batch_array = np.array(batch_features)
                sum_features += np.sum(batch_array, axis=0)
                count += len(batch_array)
                del batch_array, batch_features

        return sum_features / count if count > 0 else np.zeros(128)

    def train(self, data_path):
        train_path = os.path.join(data_path, 'train')

        def_path = os.path.join(train_path, 'def_front')
        ok_path = os.path.join(train_path, 'ok_front')

        self.def_centroid = self.process_folder_batch(def_path)
        self.ok_centroid = self.process_folder_batch(ok_path)

        with open('centroids.pkl', 'wb') as f:
            pickle.dump({'def': self.def_centroid, 'ok': self.ok_centroid}, f)

    def load_centroids(self):
        with open('centroids.pkl', 'rb') as f:
            data = pickle.load(f)
            self.def_centroid = data['def']
            self.ok_centroid = data['ok']

    def predict_image(self, image_path):
        features = self.extract_features(image_path)
        if features is None:
            return None

        dist_def = np.linalg.norm(features - self.def_centroid)
        dist_ok = np.linalg.norm(features - self.ok_centroid)

        if dist_def < dist_ok:
            return {'prediction': 'Defectuoso', 'confidence': dist_ok / (dist_def + dist_ok)}
        else:
            return {'prediction': 'Normal', 'confidence': dist_def / (dist_def + dist_ok)}

    def evaluate_batch(self, data_path):
        test_path = os.path.join(data_path, 'test')
        correct = 0
        total = 0

        for class_name, expected in [('def_front', 'Defectuoso'), ('ok_front', 'Normal')]:
            folder_path = os.path.join(test_path, class_name)
            files = [f for f in os.listdir(
                folder_path) if f.lower().endswith(('.jpg', '.jpeg', '.png'))]

            for i in range(0, len(files), self.batch_size):
                batch_files = files[i:i+self.batch_size]

                for filename in batch_files:
                    result = self.predict_image(
                        os.path.join(folder_path, filename))
                    if result and result['prediction'] == expected:
                        correct += 1
                    total += 1

        return correct / total if total > 0 else 0


class DefectClassifierKMeansLowMemory:
    def __init__(self, batch_size=50, k=2):
        self.sift = cv2.SIFT_create()
        self.batch_size = batch_size
        self.k = k
        self.centers = None
        self.def_cluster = None
        self.ok_cluster = None

    def extract_features(self, image_path):
        img = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
        if img is None:
            return None
        keypoints, descriptors = self.sift.detectAndCompute(img, None)
        if descriptors is None:
            return np.zeros(128, dtype=np.float32)
        return np.mean(descriptors, axis=0).astype(np.float32)

    def train(self, data_path):
        train_path = os.path.join(data_path, 'train')
        temp_features = []
        temp_labels = []

        for class_name, label in [('def_front', 1), ('ok_front', 0)]:
            folder_path = os.path.join(train_path, class_name)
            files = [f for f in os.listdir(
                folder_path) if f.lower().endswith(('.jpg', '.jpeg', '.png'))]

            for i in range(0, len(files), self.batch_size):
                batch_files = files[i:i+self.batch_size]
                batch_features = []

                for filename in batch_files:
                    feat = self.extract_features(
                        os.path.join(folder_path, filename))
                    if feat is not None:
                        batch_features.append(feat)
                        temp_labels.append(label)

                if batch_features:
                    np.save(f'temp_batch_{len(temp_features)}.npy', np.array(
                        batch_features))
                    temp_features.append(
                        f'temp_batch_{len(temp_features)}.npy')

        all_data = []
        for temp_file in temp_features:
            batch_data = np.load(temp_file)
            all_data.append(batch_data)
            os.remove(temp_file)

        data = np.vstack(all_data).astype(np.float32)

        criteria = (cv2.TERM_CRITERIA_EPS +
                    cv2.TERM_CRITERIA_MAX_ITER, 20, 1.0)
        _, labels, centers = cv2.kmeans(
            data, self.k, None, criteria, 10, cv2.KMEANS_RANDOM_CENTERS)

        self.centers = centers

        cluster_0_def = np.sum((labels.flatten() == 0) &
                               (np.array(temp_labels) == 1))
        cluster_0_ok = np.sum((labels.flatten() == 0) &
                              (np.array(temp_labels) == 0))

        if cluster_0_def > cluster_0_ok:
            self.def_cluster = 0
            self.ok_cluster = 1
        else:
            self.def_cluster = 1
            self.ok_cluster = 0

        with open('kmeans_model.pkl', 'wb') as f:
            pickle.dump({'centers': self.centers, 'def_cluster': self.def_cluster,
                        'ok_cluster': self.ok_cluster}, f)

    def load_model(self):
        with open('kmeans_model.pkl', 'rb') as f:
            data = pickle.load(f)
            self.centers = data['centers']
            self.def_cluster = data['def_cluster']
            self.ok_cluster = data['ok_cluster']

    def predict_image(self, image_path):
        features = self.extract_features(image_path)
        if features is None:
            return None

        distances = [np.linalg.norm(features - center)
                     for center in self.centers]
        closest_cluster = np.argmin(distances)

        if closest_cluster == self.def_cluster:
            prediction = 'Defectuoso'
        else:
            prediction = 'Normal'

        confidence = 1.0 - (min(distances) / sum(distances))
        return {'prediction': prediction, 'confidence': confidence}


if __name__ == "__main__":
    classifier = DefectClassifierLowMemory(batch_size=20)
    classifier.train("data")
    accuracy = classifier.evaluate_batch("data")
    print(f"Precisión: {accuracy:.3f}")

    result = classifier.predict_image("a.jpeg")
    print(result)
