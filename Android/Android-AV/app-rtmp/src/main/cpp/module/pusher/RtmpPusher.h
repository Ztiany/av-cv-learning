#ifndef ANDROID_AV_LIBRTMP_HPP
#define ANDROID_AV_LIBRTMP_HPP

#include <pthread.h>
#include <rtmp.h>
#include "../utils/SafeQueue.h"
#include "../utils/JavaCaller.h"
#include "../utils/Log.h"

class RtmpPusher {

private:
    RTMP *rtmp = nullptr;
    char *_url = nullptr;
    uint32_t start_time = 0;
    bool stopped = true;

    SafeQueue<RTMPPacket *> queue;
    pthread_t thread_id = 0;

    JavaCaller *javaCaller = nullptr;

public:
    RtmpPusher() = default;

    ~RtmpPusher() {
        delete _url;
    }

    void addPacket(RTMPPacket *packet) {
        queue.push(packet);
    }

    void start(const char *url) {
        this->_url = (char *) url;
        //start a thread to connect and send packets.
        pthread_create(&thread_id, nullptr, &RtmpPusher::start_internal, this);
    }

    void stop() {

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
        start_time = RTMP_GetTime();
        //allow the queue to accept packet.
        queue.setWork(1);
        //change the flag.
        stopped = false;

        int ret;
        RTMPPacket *packet = nullptr;
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
            //用完就释放
            releasePackets(packet);
            if (!ret) {
                LOGE("发送数据失败");
                break;
            }
        }
        releasePackets(packet);
    }

    static void *start_internal(void *args) {
        auto *libRtmp = static_cast<RtmpPusher *>(args);

        libRtmp->rtmp = RTMP_Alloc();
        if (!libRtmp->rtmp) {
            LOGE("rtmp 创建失败");
            return nullptr;
        }
        RTMP_Init(libRtmp->rtmp);

        //设置超时时间 5s
        libRtmp->rtmp->Link.timeout = 5;
        int ret = RTMP_SetupURL(libRtmp->rtmp, libRtmp->_url);
        if (!ret) {
            LOGE("rtmp 设置地址失败:%s", libRtmp->_url);
            return nullptr;
        }

        //开启输出模式
        RTMP_EnableWrite(libRtmp->rtmp);

        ret = RTMP_Connect(libRtmp->rtmp, nullptr);
        if (!ret) {
            LOGE("rtmp 连接地址失败:%s", libRtmp->_url);
            return nullptr;
        }

        ret = RTMP_ConnectStream(libRtmp->rtmp, 0);
        LOGE("rtmp 连接成功----------->:%s", libRtmp->_url);
        if (!ret) {
            LOGE("rtmp 连接流失败:%s", libRtmp->_url);
            return nullptr;
        }

        libRtmp->startSender();
        return nullptr;
    }

};


#endif //ANDROID_AV_LIBRTMP_HPP
