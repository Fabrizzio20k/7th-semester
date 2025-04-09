import numpy as np
import matplotlib.pyplot as plt
import cv2
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Input, Conv2D, MaxPooling2D, Conv2DTranspose, BatchNormalization, Activation
from tensorflow.keras.optimizers import Adam
from sklearn.model_selection import train_test_split

# ========== CONFIG ==========
IMG_PATH = "foto_original.jpg"
IMG_SIZE = 256
N_SAMPLES = 1000
PATCHES = 10
EPOCHS = 10
BATCH = 16

# ========== 1. CARGAR Y NORMALIZAR ==========


def load_image(path, size=128):
    img = cv2.imread(path)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img = cv2.resize(img, (size, size))
    img = img.astype('float32') / 255.0
    return img


img_original = load_image(IMG_PATH, size=IMG_SIZE)

# ========== 2. DAÑAR LA IMAGEN ==========


def mask_random_patches(img, n_patches=10, patch_size=16):
    img = img.copy()
    h, w, _ = img.shape
    for _ in range(n_patches):
        x = np.random.randint(0, h - patch_size)
        y = np.random.randint(0, w - patch_size)
        img[x:x+patch_size, y:y+patch_size, :] = 0
    return img


def create_dataset_from_image(img, n=500):
    data_in, data_out = [], []
    for _ in range(n):
        noisy = mask_random_patches(img, n_patches=PATCHES)
        data_in.append(noisy)
        data_out.append(img)
    return np.array(data_in), np.array(data_out)


x_data, y_data = create_dataset_from_image(img_original, n=N_SAMPLES)

# ========== 3. DIVIDIR TRAIN / VALIDATION ==========
x_train, x_val, y_train, y_val = train_test_split(
    x_data, y_data, test_size=0.1, random_state=42)

# ========== 4. AUTOENCODER CONVOLUCIONAL ==========


def build_autoencoder(input_shape=(128, 128, 3)):
    inp = Input(shape=input_shape)

    # Encoder
    x = Conv2D(64, 3, padding='same')(inp)
    x = BatchNormalization()(x)
    x = Activation('relu')(x)
    x = MaxPooling2D(2)(x)

    x = Conv2D(128, 3, padding='same')(x)
    x = BatchNormalization()(x)
    x = Activation('relu')(x)
    x = MaxPooling2D(2)(x)

    # Decoder
    x = Conv2DTranspose(128, 3, strides=2, padding='same')(x)
    x = BatchNormalization()(x)
    x = Activation('relu')(x)

    x = Conv2DTranspose(64, 3, strides=2, padding='same')(x)
    x = BatchNormalization()(x)
    x = Activation('relu')(x)

    out = Conv2D(3, 3, activation='sigmoid', padding='same')(x)

    return Model(inp, out)


model = build_autoencoder((IMG_SIZE, IMG_SIZE, 3))
model.compile(optimizer=Adam(learning_rate=0.001), loss='mse')

# ========== 5. ENTRENAMIENTO ==========
model.fit(x_train, y_train, validation_data=(x_val, y_val),
          epochs=EPOCHS, batch_size=BATCH, verbose=1)

# ========== 6. PRUEBA Y VISUALIZACIÓN ==========
img_test_damaged = mask_random_patches(img_original, n_patches=PATCHES)
x_test = np.expand_dims(img_test_damaged, axis=0)
reconstructed = model.predict(x_test)[0]

# ========== 7. PLOT ==========
plt.figure(figsize=(12, 4))

plt.subplot(1, 3, 1)
plt.imshow(img_test_damaged)
plt.title("Dañada")
plt.axis('off')

plt.subplot(1, 3, 2)
plt.imshow(reconstructed)
plt.title("Reconstruida")
plt.axis('off')

plt.subplot(1, 3, 3)
plt.imshow(img_original)
plt.title("Original")
plt.axis('off')

plt.tight_layout()
plt.show()
