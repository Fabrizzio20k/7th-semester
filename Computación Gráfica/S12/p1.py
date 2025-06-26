import cv2


def draw_matches(img1_path, img2_path):
    img1 = cv2.imread(img1_path, cv2.IMREAD_GRAYSCALE)
    img2 = cv2.imread(img2_path, cv2.IMREAD_GRAYSCALE)

    sift = cv2.SIFT_create()

    kp1, des1 = sift.detectAndCompute(img1, None)
    kp2, des2 = sift.detectAndCompute(img2, None)

    bf = cv2.BFMatcher()
    matches = bf.knnMatch(des1, des2, k=2)

    matches = sorted(matches, key=lambda x: x[0].distance)

    result = cv2.drawMatchesKnn(
        img1, kp1, img2, kp2, matches[:5], None, flags=cv2.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS)

    cv2.imshow('Matches', result)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


draw_matches('i1.jpg', 'i2.jpg')
