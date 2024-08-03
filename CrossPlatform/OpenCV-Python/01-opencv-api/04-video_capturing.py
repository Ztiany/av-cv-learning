import cv2
import os

VIDEO_SAVE_PATH = "../resources/video/"
if not os.path.exists(VIDEO_SAVE_PATH):
    os.makedirs(VIDEO_SAVE_PATH)

# 不同的操作系统可能需要不同的 fourcc，不同的 fourcc 需要对应正确的文件后缀名。
fourcc = cv2.VideoWriter_fourcc(*'XVID')
cap = cv2.VideoCapture(0)

# 检查摄像头是否正常工作
if not cap.isOpened():
    print("Error: Unable to access camera")
    exit()

# 获取摄像头的实际分辨率，保证保存的视频分辨率与摄像头分辨率一致。
ret, frame = cap.read()
if not ret:
    print("Error: Unable to read frame from camera")
    cap.release()
    cv2.destroyAllWindows()
    exit()

height, width = frame.shape[:2]
vw = cv2.VideoWriter(f"{VIDEO_SAVE_PATH}out.avi", fourcc, 25, (width, height))

cv2.namedWindow('video', cv2.WINDOW_NORMAL)
cv2.resizeWindow('video', width, height)

while cap.isOpened():
    ret, frame = cap.read()
    if ret:
        vw.write(frame)
        cv2.imshow('video', frame)
        if cv2.waitKey(1) == ord('q'):
            break
    else:
        print("Error: Failed to read frame")
        break

cap.release()
vw.release()
cv2.destroyAllWindows()
