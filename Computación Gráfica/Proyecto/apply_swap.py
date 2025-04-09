import cv2
import tensorflow as tf
import os
import numpy as np
from tensorflow.keras.models import load_model
from tqdm import tqdm


def apply_blend(original, face_swap):
    # Crear máscara circular difuminada
    h, w = face_swap.shape[:2]
    mask = np.zeros((h, w), dtype=np.float32)
    cv2.circle(mask, (w // 2, h // 2), int(w * 0.45), 1, -1)  # círculo central
    mask = cv2.GaussianBlur(mask, (51, 51), 0)  # bordes suaves

    # Convertir máscara a 3 canales
    mask_3ch = np.stack([mask]*3, axis=-1)

    # Combinar suavemente usando la máscara
    blended = (face_swap * mask_3ch + original *
               (1 - mask_3ch)).astype(np.uint8)
    return blended


encoder = load_model("encoder.h5")
decoder_B = load_model("decoder_B.h5")

os.makedirs("output_frames", exist_ok=True)
video = cv2.VideoCapture("videos/target.mp4")

batch_size = 32
frames = []
original_sizes = []

frame_id = 0
while True:
    ret, frame = video.read()
    if not ret:
        break
    resized = cv2.resize(frame, (512, 512))
    norm = resized.astype(np.float32) / 255.0
    frames.append(norm)
    original_sizes.append((frame.shape[1], frame.shape[0]))  # width, height
    frame_id += 1
video.release()

frames = np.array(frames)
print(f"Procesando {len(frames)} frames...")

# Inference por batch
for i in tqdm(range(0, len(frames), batch_size)):
    batch = frames[i:i+batch_size]
    codes = encoder.predict(batch)
    swapped_batch = decoder_B.predict(codes)

    for j, swapped in enumerate(swapped_batch):
        out_img = (swapped * 255).astype(np.uint8)
        target_size = original_sizes[i + j]
        resized_swap = cv2.resize(
            out_img, target_size, interpolation=cv2.INTER_CUBIC)
        original_frame = cv2.resize(
            (frames[i + j] * 255).astype(np.uint8), target_size)
        blended = apply_blend(original_frame, resized_swap)
        cv2.imwrite(f"output_frames/frame_{i + j:04d}.jpg", blended)

print("✅ Face swap terminado.")
