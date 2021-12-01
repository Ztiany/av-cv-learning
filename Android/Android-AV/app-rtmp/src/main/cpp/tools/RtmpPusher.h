#ifndef ANDROID_AV_LIBRTMP_HPP
#define ANDROID_AV_LIBRTMP_HPP

#include <SafeQueue.h>
#include <pthread.h>
#include <JavaCaller.h>
#include <Log.h>

extern "C" {
#include "../librtmp/rtmp.h"
}

class RtmpPusher {

private:
    SafeQueue<RTMPPacket *> queue;
    static pthread_t thread_id;
    RTMP *rtmp = nullptr;
    const char *_url = nullptr;
    uint32_t start_time;
    JavaCaller *javaCaller = nullptr;
    bool ready_pushing = false;
    bool stopped = false;

public:
    RtmpPusher() {

    }

    ~RtmpPusher() {
        delete _url;
    }

    void addPacket(RTMPPacket *packet) {
        queue.push(packet);
    }

    void start(const char *url) {
        this->_url = url;
        //start a thread to connect
        pthread_create(&thread_id, nullptr, &RtmpPusher::start_internal, this);
    }

    void stop() {

    }

private:

    void startSender() {
        start_time = RTMP_GetTime();
        ready_pushing = true;
        start_time = RTMP_GetTime();
        queue.setWork(1);

        RTMPPacket *packet = nullprt;
        //循环从队列取包，然后发送。
        while (!stopped) {
            queue.pop(packet);
            if (stopped) {
                break;
            }
            if (!packet) {
                continue;
            }
            // 给rtmp的流id
            packet->m_nInfoField2 = rtmp->m_stream_id;
            //发送包 1:加入队列发送
            ret = RTMP_SendPacket(rtmp, packet, 1);
            releasePackets(packet);
            if (!ret) {
                LOGE("发送数据失败");
                break;
            }
        }
        releasePackets(packet);
    }

    static void *start_internal(void *args) {
        RtmpPusher *libRtmp = static_cast<RtmpPusher *>(args);

        rtmp = RTMP_Alloc();
        if (!rtmp) {
            LOGE("rtmp创建失败");
            break;
        }
        RTMP_Init(rtmp);

        //设置超时时间 5s
        rtmp->Link.timeout = 5;
        int ret = RTMP_SetupURL(rtmp, url);
        if (!ret) {
            LOGE("rtmp设置地址失败:%s", url);
            break;
        }

        //开启输出模式
        RTMP_EnableWrite(rtmp);

        ret = RTMP_Connect(rtmp, 0);
        if (!ret) {
            LOGE("rtmp连接地址失败:%s", url);
            break;
        }

        ret = RTMP_ConnectStream(rtmp, 0);
        LOGE("rtmp连接成功----------->:%s", url);
        if (!ret) {
            LOGE("rtmp连接流失败:%s", url);
            break;
        }

        libRtmp->startSender();
    }

};


#endif //ANDROID_AV_LIBRTMP_HPP
