import cv2

# 创建窗口
cv2.namedWindow('video', cv2.WINDOW_NORMAL)
cv2.resizeWindow('video', 1280, 720)

# 创建 VideoCapture
cap = cv2.VideoCapture(0)

# 判断摄像头是否为打开关态
while cap.isOpened():
    # 从摄像头读视频帧
    ret, frame = cap.read()

    if ret:
        # 将视频帧在窗口中显示
        cv2.imshow('video', frame)

        # 等待键盘事件，如果为 q，退出
        key = cv2.waitKey(5)  # 等待 5 毫秒
        if key & 0xFF == ord('q'):
            break
    else:
        break

cap.release()
cv2.destroyAllWindows()
