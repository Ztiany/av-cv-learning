#ifndef ANDROID_AV_X264CODEC_H
#define ANDROID_AV_X264CODEC_H

#include <x264/x264.h>
#include <rtmp.h>
#include "utils/Config.h"

typedef void (*VideoCodecCallback)(RTMPPacket *rtmpPacket);

class X264Codec {

private:
    int _width = 0;
    int _height = 0;
    int _fps = 0;
    int _bitrate = 0;

    x264_picture_t *_pic_in = nullptr;
    int _ySize = 0;
    int _uvSize = 0;

    x264_t *_videoCodec = nullptr;

    VideoCodecCallback _videoCodecCallback = nullptr;

private:
    int calcUVSize(int format) {
        if(I420 == format){
            return _width * _height * 3 / 2;
        }
        //TODO：兼容其他格式
        return _width * _height * 3 / 2;
    }

public:
    void setVideoInfo(int width, int height, int fps, int bitrate, jint format) {
        //save video info
        _width = width;
        _height = height;
        _fps = fps;
        _bitrate = bitrate;
        //calc size
        _ySize = _width * _height;
        _uvSize = calcUVSize(format);

        //设置编码器参数
        x264_param_t param;
        x264_param_default_preset(&param, "ultrafast", "zerolatency");
        param.i_bframe = 0;
    }

    void encodeData(int8_t *data, int len) {

    }

    void setCodecCallback(VideoCodecCallback videoCodecCallback) {
        this->_videoCodecCallback = videoCodecCallback;
    }

};

#endif //ANDROID_AV_X264CODEC_H
