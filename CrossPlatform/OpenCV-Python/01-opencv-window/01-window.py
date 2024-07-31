import cv2

cv2.namedWindow('image', cv2.WINDOW_NORMAL)
cv2.resizeWindow('image', 640, 480)
cv2.imshow('image', 1)

# 0 means wait for any key press and no time-out.
key = cv2.waitKey(0)
if key == 'q':
    exit()

cv2.destroyAllWindows()
