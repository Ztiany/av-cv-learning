import cv2
import numpy as np


# 鼠标回调函数：event 事件类型，x, y 鼠标坐标，flags 鼠标事件标志，userdata 用户数据。
def mouse_callback(event, x, y, flags, userdata):
    print(event, x, y, flags, userdata)


# 创建窗口
cv2.namedWindow('mouse', cv2.WINDOW_NORMAL)
cv2.resizeWindow('mouse', 640, 360)

# 设置鼠标回调：窗口名，回调函数，用户数据
cv2.setMouseCallback('mouse', mouse_callback, "123")

# 显示窗口和背景，numpy 是一个多维数组库，这里创建一个 360x640 的黑色背景。
img = np.zeros((360, 640, 3), np.uint8)
while True:
    cv2.imshow('mouse', img)
    key = cv2.waitKey(1)
    if key & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
