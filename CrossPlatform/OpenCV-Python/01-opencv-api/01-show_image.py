import cv2

IMAGE_READ_PATH = "../resources/image/"
IMAGE_SAVE_PATH = "../resources/processed/"

cv2.namedWindow('image', cv2.WINDOW_NORMAL)
cv2.resizeWindow('image', 320, 240)

img = cv2.imread(f"{IMAGE_READ_PATH}perspective.jpeg")
img2 = img.copy()

while True:
    cv2.imshow('image', img2)
    key = cv2.waitKey(0)
    if key & 0xFF == ord('q'):
        print("q clicked")
        break
    elif key & 0xFF == ord('s'):
        print("s clicked")
        cv2.imwrite(f"{IMAGE_SAVE_PATH}perspective.png", img)
    else:
        print(f"{key} clicked")

cv2.destroyAllWindows()
