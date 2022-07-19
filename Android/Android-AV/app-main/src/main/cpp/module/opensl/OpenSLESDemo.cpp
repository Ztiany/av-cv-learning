#include <jni.h>
#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <log.h>

//1 创建引擎
static SLObjectItf engineSL = nullptr;

SLEngineItf CreateSL() {
    SLresult re;
    SLEngineItf en;
    re = slCreateEngine(&engineSL, 0, nullptr, 0, nullptr, nullptr);
    if (re != SL_RESULT_SUCCESS) return nullptr;
    re = (*engineSL)->Realize(engineSL, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) return nullptr;
    re = (*engineSL)->GetInterface(engineSL, SL_IID_ENGINE, &en);
    if (re != SL_RESULT_SUCCESS) return nullptr;
    return en;
}

void PcmCallback(SLAndroidSimpleBufferQueueItf bf, void *context) {
    XLOGD("PcmCallback");
    static FILE *fp = nullptr;
    static char *buf = nullptr;
    if (!buf) {
        buf = new char[1024 * 1024];
    }
    if (!fp) {
        fp = fopen("/sdcard/test.pcm", "rb");
    }
    if (!fp)return;
    if (feof(fp) == 0) {
        int len = fread(buf, 1, 1024, fp);
        if (len > 0)
            (*bf)->Enqueue(bf, buf, len);
    }
}

void playPCM() {
    //1 创建引擎
    SLEngineItf eng = CreateSL();
    if (!eng) {
        XLOGD("CreateSL failed!");
        return;
    }
    XLOGD("CreateSL success!");


    //2 创建混音器
    SLObjectItf mix = nullptr;
    SLresult re;
    re = (*eng)->CreateOutputMix(eng, &mix, 0, nullptr, nullptr);
    if (re != SL_RESULT_SUCCESS) {
        XLOGD("SL_RESULT_SUCCESS failed!");
    }
    re = (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) {
        XLOGD("(*mix)->Realize failed!");
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, mix};
    SLDataSink audioSink = {&outputMix, nullptr};


    //3 配置音频信息
    //缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue que = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};
    //音频格式
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            2,//    声道数
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN //字节序，小端
    };
    SLDataSource ds = {&que, &pcm};


    //4 创建播放器
    SLObjectItf player = nullptr;
    SLPlayItf iPlayer = nullptr;
    SLAndroidSimpleBufferQueueItf pcmQue = nullptr;
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    re = (*eng)->CreateAudioPlayer(eng, &player, &ds, &audioSink, sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (re != SL_RESULT_SUCCESS) {
        XLOGD("CreateAudioPlayer failed!");
    } else {
        XLOGD("CreateAudioPlayer success!");
    }
    (*player)->Realize(player, SL_BOOLEAN_FALSE);
    //获取player接口
    re = (*player)->GetInterface(player, SL_IID_PLAY, &iPlayer);
    if (re != SL_RESULT_SUCCESS) {
        XLOGD("GetInterface SL_IID_PLAY failed!");
    }
    re = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, &pcmQue);
    if (re != SL_RESULT_SUCCESS) {
        XLOGD("GetInterface SL_IID_BUFFERQUEUE failed!");
    }

    //设置回调函数，播放队列空调用
    (*pcmQue)->RegisterCallback(pcmQue, PcmCallback, nullptr);

    //设置为播放状态
    (*iPlayer)->SetPlayState(iPlayer, SL_PLAYSTATE_PLAYING);

    //启动队列回调
    (*pcmQue)->Enqueue(pcmQue, "", 1);
}