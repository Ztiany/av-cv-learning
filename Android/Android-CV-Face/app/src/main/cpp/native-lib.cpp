#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/native_window_jni.h>

using namespace cv;

ANativeWindow *window = nullptr;
pthread_mutex_t mutex;
DetectionBasedTracker *tracker = nullptr;

class CascadeDetectorAdapter : public DetectionBasedTracker::IDetector {
public:
    CascadeDetectorAdapter(cv::Ptr<cv::CascadeClassifier> detector) : IDetector(), Detector(detector) {

    }

    //适配器检测到了很多物体，交给适配器，告诉我这些形状是不是属于这个分类   类似于 RecyclerView  产生了滑动，通知 Adapter 要拿数据过来了。
    void detect(const cv::Mat &image, std::vector<cv::Rect> &objects) override {
        Detector->detectMultiScale(image, objects, scaleFactor, minNeighbours, 0, minObjSize, maxObjSize);
    }

private:
    CascadeDetectorAdapter() = delete;

    cv::Ptr<cv::CascadeClassifier> Detector;
};

extern "C"
JNIEXPORT void JNICALL
Java_com_opencv_face_demo_MainActivity_init(JNIEnv *env, jobject thiz, jstring model_) {
    pthread_mutex_init(&mutex, nullptr);
    const char *model = env->GetStringUTFChars(model_, nullptr);

    //OpenCV
    if (tracker) {
        tracker->stop();
        delete tracker;
        tracker = nullptr;
    }

    Ptr<CascadeClassifier> classifier = makePtr<CascadeClassifier>(model);
    Ptr<CascadeDetectorAdapter> mainDetector = makePtr<CascadeDetectorAdapter>(classifier);

    Ptr<CascadeClassifier> trackingClassifier = makePtr<CascadeClassifier>(model);
    Ptr<CascadeDetectorAdapter> trackingDetector = makePtr<CascadeDetectorAdapter>(trackingClassifier);

    DetectionBasedTracker::Parameters detectorParams;
    tracker = new DetectionBasedTracker(mainDetector, trackingDetector, detectorParams);

    //开启跟踪器
    tracker->run();

    env->ReleaseStringUTFChars(model_, model);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_opencv_face_demo_MainActivity_setSurface(JNIEnv *env, jobject thiz, jobject surface) {
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    window = ANativeWindow_fromSurface(env, surface);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_opencv_face_demo_MainActivity_postData(JNIEnv *env, jobject thiz, jbyteArray data_, jint w, jint h, jint cameraId) {
    //nv21 的数据
    jbyte *data = env->GetByteArrayElements(data_, nullptr);

    //yuv 复制到 mat
    Mat src(h + h / 2, w, CV_8UC1, data);
    //颜色格式的转换：nv21->RGBA
    cvtColor(src, src, COLOR_YUV2RGBA_NV21);

    //输出到本地，用于测试
    //imwrite("/sdcard/src.jpg",src);

    if (cameraId == 1) {
        //前置摄像头，需要逆时针旋转90度
        rotate(src, src, ROTATE_90_COUNTERCLOCKWISE);
        //水平翻转 镜像
        flip(src, src, 1);
    } else {
        //顺时针旋转90度
        rotate(src, src, ROTATE_90_CLOCKWISE);
    }

    Mat gray;
    //灰色
    cvtColor(src, gray, COLOR_RGBA2GRAY);
    //增强对比度 (直方图均衡)
    equalizeHist(gray, gray);
    std::vector<Rect> faces;
    //定位人脸 N个
    tracker->process(gray);
    tracker->getObjects(faces);
    for (const Rect &face : faces) {
        //画矩形
        rectangle(src, face, Scalar(255, 0, 0));
    }

    //显示
    if (window) {
        //设置windows的属性
        // 因为旋转了，所以宽、高需要交换
        //这里使用 cols 和 rows 代表 宽、高，就不用关心上面是否旋转了
        ANativeWindow_setBuffersGeometry(window, src.cols, src.rows, WINDOW_FORMAT_RGBA_8888);
        do {
            if (!window) {
                break;
            }
            ANativeWindow_setBuffersGeometry(window, src.cols, src.rows, WINDOW_FORMAT_RGBA_8888);
            ANativeWindow_Buffer buffer;
            if (ANativeWindow_lock(window, &buffer, nullptr)) {
                ANativeWindow_release(window);
                window = nullptr;
                break;
            }

            auto *dstData = static_cast<uint8_t *>(buffer.bits);
            int dstLineSize = buffer.stride * 4;
            uint8_t *srcData = src.data;
            int srcLineSize = src.cols * 4;
            for (int i = 0; i < buffer.height; ++i) {
                memcpy(dstData + i * dstLineSize, srcData + i * srcLineSize, srcLineSize);
            }
            ANativeWindow_unlockAndPost(window);
        } while (false);
    }

    //释放Mat
    src.release();
    gray.release();
    env->ReleaseByteArrayElements(data_, data, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_opencv_face_demo_MainActivity_release(JNIEnv *env, jobject thiz) {
    if (tracker) {
        tracker->stop();
        delete tracker;
        tracker = nullptr;
    }
}