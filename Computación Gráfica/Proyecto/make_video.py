import cv2
import os
import glob

# Tamaño de salida (720p)
output_size = (1280, 720)

# FPS deseado (puedes cambiarlo)
fps = 15.0

# Carga los frames
frames = sorted(glob.glob("output_frames/*.jpg"))
frame = cv2.imread(frames[0])

# Configura el video final
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter("output_video.mp4", fourcc, fps, output_size)

# Genera el video con resize suave
for file in frames:
    img = cv2.imread(file)
    img = cv2.resize(img, output_size, interpolation=cv2.INTER_CUBIC)
    out.write(img)

out.release()
print("✅ Video generado a 720p, {} fps: output_video.mp4".format(int(fps)))
