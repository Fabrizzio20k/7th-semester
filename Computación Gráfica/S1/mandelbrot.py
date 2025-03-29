# import cv2
# import numpy as np


# def sequence(c, z=0):
#     while True:
#         yield z
#         z = z ** 2 + c

# # def mandelbrotGeneration(n, max_iter):
# #     img = np.zeros((n, n, 3), dtype=np.uint8)
# #     for x in range(n):
# #         for y in range(n):


# #     return img


# def mandelbrotSet(n, max_iter):

#     for n, z in enumerate(sequence(c=-1)):
#         print(f'z({n}): {z}')
#         if n >= 11:
#             break

#     # img = mandelbrotGeneration(n, max_iter)
#     # cv2.imshow("Mandelbrot Set", img)
#     # cv2.waitKey(0)
#     # cv2.destroyAllWindows()


# if __name__ == "__main__":
#     n = 800
#     max_iter = 100
#     mandelbrotSet(n, max_iter)
import numpy as np
import matplotlib.pyplot as plt

# setting parameters (these values can be changed)
xDomain, yDomain = np.linspace(-2, 2, 500), np.linspace(-2, 2, 500)
bound = 2
max_iterations = 50  # any positive integer value
colormap = "nipy_spectral"  # set to any matplotlib valid colormap


def func(z, p, c): return z**p + c


# computing 2-d array to represent the mandelbrot-set
iterationArray = []
for y in yDomain:
    row = []
    for x in xDomain:
        z = 0
        p = 2
        c = complex(x, y)
        for iterationNumber in range(max_iterations):
            if abs(z) >= bound:
                row.append(iterationNumber)
                break
            else:
                try:
                    z = func(z, p, c)
                except (ValueError, ZeroDivisionError):
                    z = c
        else:
            row.append(0)

    iterationArray.append(row)

# plotting the data
ax = plt.axes()
ax.set_aspect("equal")
graph = ax.pcolormesh(xDomain, yDomain, iterationArray, cmap=colormap)
plt.colorbar(graph)
plt.xlabel("Real-Axis")
plt.ylabel("Imaginary-Axis")
plt.show()
