import cv2
import numpy as np


def trackbar_callback(x):
    print(x)


# 创建窗口
cv2.namedWindow('trackbar', cv2.WINDOW_NORMAL)

# 创建 trackbar：trackbar 名字，窗口名，最小值，最大值，回调函数。
cv2.createTrackbar('R', 'trackbar', 0, 255, trackbar_callback)
cv2.createTrackbar('G', 'trackbar', 0, 255, trackbar_callback)
cv2.createTrackbar('B', 'trackbar', 0, 255, trackbar_callback)

# 创建一个背景图片
img = np.zeros((480, 640, 3), np.uint8)

while True:

    # 获取当前 trackbar 的值
    r = cv2.getTrackbarPos('R', 'trackbar')
    g = cv2.getTrackbarPos('G', 'trackbar')
    b = cv2.getTrackbarPos('B', 'trackbar')

    # 改变背景图片颜色
    img[:] = [b, g, r]
    cv2.imshow('trackbar', img)

    key = cv2.waitKey(10)
    if key & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
