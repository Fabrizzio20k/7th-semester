from tensorflow.keras.callbacks import EarlyStopping
import tensorflow as tf
import numpy as np
import os
from tensorflow.keras.layers import Input, Conv2D, Conv2DTranspose, LeakyReLU
from tensorflow.keras.models import Model
from tensorflow.keras.preprocessing.image import load_img, img_to_array


def load_dataset(path):
    images = []
    for file in os.listdir(path):
        img = load_img(os.path.join(path, file), target_size=(512, 512))
        img = img_to_array(img).astype(np.float32) / 255.0
        images.append(img)
    return np.array(images)


def build_encoder():
    inp = Input(shape=(512, 512, 3))
    x = Conv2D(64, 4, strides=2, padding='same')(inp)
    x = LeakyReLU()(x)
    x = Conv2D(128, 4, strides=2, padding='same')(x)
    x = LeakyReLU()(x)
    x = Conv2D(256, 4, strides=2, padding='same')(x)
    x = LeakyReLU()(x)
    x = Conv2D(512, 4, strides=2, padding='same')(x)
    x = LeakyReLU()(x)
    return Model(inp, x)


def build_decoder():
    inp = Input(shape=(16, 16, 512))
    x = Conv2DTranspose(256, 4, strides=2, padding='same')(inp)
    x = LeakyReLU()(x)
    x = Conv2DTranspose(128, 4, strides=2, padding='same')(x)
    x = LeakyReLU()(x)
    x = Conv2DTranspose(64, 4, strides=2, padding='same')(x)
    x = LeakyReLU()(x)
    x = Conv2DTranspose(3, 4, strides=2, padding='same',
                        activation='sigmoid')(x)
    return Model(inp, x)


# Cargar datasets
faces_A = load_dataset("data/A")
faces_B = load_dataset("data/B")

# Modelos
encoder = build_encoder()
decoder_A = build_decoder()
decoder_B = build_decoder()

model_A = Model(encoder.input, decoder_A(encoder.output))
model_B = Model(encoder.input, decoder_B(encoder.output))

model_A.compile(optimizer='adam', loss='mse')
model_B.compile(optimizer='adam', loss='mse')


early_stop = EarlyStopping(
    monitor='loss',
    patience=5,
    restore_best_weights=True
)

model_A.fit(faces_A, faces_A, epochs=10, batch_size=8, callbacks=[early_stop])
model_B.fit(faces_B, faces_B, epochs=10, batch_size=8, callbacks=[early_stop])

encoder.save("encoder.h5")
decoder_B.save("decoder_B.h5")
