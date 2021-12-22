#ifndef ANDROID_AV_CONFIG_H
#define ANDROID_AV_CONFIG_H

enum VideoType {
    YUV,
    X264
};

enum VideoFormat {
    I420,
    NV21,
    NV12
};

enum AudioType {
    PCM = 2, AAC_INFO, AAC_DATA
};

#endif //ANDROID_AV_CONFIG_H
