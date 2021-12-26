#ifndef ANDROID_AV_LIBRTMP_HPP
#define ANDROID_AV_LIBRTMP_HPP

#include <pthread.h>
#include <rtmp.h>
#include "../utils/SafeQueue.h"
#include "../utils/JavaCaller.h"
#include "../utils/Log.h"
#include "../utils/Config.h"
#include "../codec/X264Parser.h"
#include "../codec/AACParser.h"
#include "../codec/X264Codec.h"
#include "../codec/AACCodec.h"

class RtmpPusher {

private:
    RTMP *_rtmp = nullptr;
    char *_url = nullptr;

    uint32_t _start_time = 0;

    bool _stopped = true;

    SafeQueue<RTMPPacket *> _queue;
    pthread_t _thread_id = 0;

    X264Parser x264Parser;
    AACParser aacParser;

    X264Codec x264Codec;

    JavaCaller *_javaCaller = nullptr;

private:
    static void codecCallback(void *attachment, RTMPPacket *rtmpPacket) {
        auto *pusher = static_cast<RtmpPusher *>(attachment);
        if (pusher->_queue.size() > 80) {//防止内存溢出
            LOGE("clear queue, sizer more than 80");
            pusher->_queue.clear();
        }
        rtmpPacket->m_nTimeStamp = RTMP_GetTime() - pusher->_start_time;
        LOGI("rtmp pusher codecCallback called. type = %d, TimeStamp = %d", rtmpPacket->m_packetType, rtmpPacket->m_nTimeStamp);
        pusher->_queue.push(rtmpPacket);
    }

public:
    RtmpPusher() = default;

    ~RtmpPusher() {
        delete _url;
        delete _javaCaller;
    }

    void processData(int8_t *buf, int len, long tms, int type) {
        if (type == X264) {
            x264Parser.parsePacket(buf, len, tms, &_queue);
        } else if (type == AAC_INFO || type == AAC_DATA) {
            aacParser.parsePacket(buf, len, type, tms, &_queue);
        } else if (YUV == type) {
            x264Codec.encodeData(buf, len, this);
        }
    }

    void start(const char *url) {
        if (isPushing()) {
            LOGE("不要重复调用 start(const char*)");
            return;
        }

        this->_url = (char *) url;
        //start a thread to connect and send packets.
        pthread_create(&_thread_id, nullptr, &RtmpPusher::start_internal, this);
    }

    void stop() {
        _stopped = true;
        _queue.setWork(0);
    }

    bool isPushing() {
        return !_stopped;
    }

    void initJavaCaller(JavaCaller *javaCaller) {
        this->_javaCaller = javaCaller;
    }

    void initVideoCodec(jint width, jint height, jint fps, jint bitrate, jint format) {
        x264Codec.setVideoInfo(width, height, fps, bitrate, format);
        x264Codec.setCodecCallback(RtmpPusher::codecCallback);
    }

private:

    static void releasePackets(RTMPPacket *&packet) {
        if (packet) {
            RTMPPacket_Free(packet);
            delete packet;
            packet = nullptr;
        }
    }

    void startSender() {
        //record the start time.
        _start_time = RTMP_GetTime();
        //allow the queue to accept packet.
        _queue.setWork(1);
        //change the flag.
        _stopped = false;

        int ret;
        RTMPPacket *packet = nullptr;
        //循环从队列取包，然后发送。
        while (!_stopped) {
            _queue.pop(packet);
            if (_stopped) {
                break;
            }
            if (!packet) {
                LOGE("packet = null");
                continue;
            }

            // 给rtmp的流id
            packet->m_nInfoField2 = _rtmp->m_stream_id;
            //发送包 1:加入队列发送
            ret = RTMP_SendPacket(_rtmp, packet, 1);
            if (!ret) {
                LOGE("发送数据失败");
                stop();
                break;
            }
            //用完就释放
            releasePackets(packet);
        }

        //end of loop
        releasePackets(packet);
    }

    static void *start_internal(void *args) {
        auto *libRtmp = static_cast<RtmpPusher *>(args);

        libRtmp->_rtmp = RTMP_Alloc();
        if (!libRtmp->_rtmp) {
            LOGE("rtmp 创建失败");
            libRtmp->stop();
            libRtmp->_javaCaller->notifyInitResult(false, Child);
            return nullptr;
        }
        RTMP_Init(libRtmp->_rtmp);

        //设置超时时间 5s
        libRtmp->_rtmp->Link.timeout = 5;
        int ret = RTMP_SetupURL(libRtmp->_rtmp, libRtmp->_url);
        if (!ret) {
            LOGE("rtmp 设置地址失败:%s", libRtmp->_url);
            libRtmp->stop();
            libRtmp->_javaCaller->notifyInitResult(false, Child);
            return nullptr;
        }

        //开启输出模式
        RTMP_EnableWrite(libRtmp->_rtmp);

        ret = RTMP_Connect(libRtmp->_rtmp, nullptr);
        if (!ret) {
            LOGE("rtmp 连接地址失败:%s", libRtmp->_url);
            libRtmp->stop();
            libRtmp->_javaCaller->notifyInitResult(false, Child);
            return nullptr;
        }

        ret = RTMP_ConnectStream(libRtmp->_rtmp, 0);
        if (!ret) {
            LOGE("rtmp 连接流失败:%s", libRtmp->_url);
            libRtmp->stop();
            libRtmp->_javaCaller->notifyInitResult(false, Child);
            return nullptr;
        }
        LOGE("rtmp 连接成功----------->:%s", libRtmp->_url);
        libRtmp->_javaCaller->notifyInitResult(true, Child);

        libRtmp->startSender();
        return nullptr;
    }

};


#endif //ANDROID_AV_LIBRTMP_HPP
