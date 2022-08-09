#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "condmutex.h"
#include <QFile>
#include <QImage>
#include <QObject>
#include <QRect>
#include <list>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

//===========================================================================
// 宏定义：工具
//===========================================================================

/** 根据返回值 ret 获取 ffmpeg 错误信息，并定义为 errorBuffer */
#define ERROR_BUFFER_FROM_RET                                                                                                                                                                          \
  char errorBuffer[1024];                                                                                                                                                                              \
  av_strerror(ret, errorBuffer, sizeof(errorBuffer));

/** 输出根据 ret 生成的错误，并执行指定代码 */
#define PRINT_ERROR_AND_EXECUTE(func, code)                                                                                                                                                            \
  if (ret < 0) {                                                                                                                                                                                       \
    ERROR_BUFFER_FROM_RET;                                                                                                                                                                             \
    qDebug() << #func << "error" << errorBuffer;                                                                                                                                                       \
    code;                                                                                                                                                                                              \
  }

/** 输出根据 ret 生成的错误，并终止程序 */
#define PUT_ERROR_AND_TERMINATE(func) PRINT_ERROR_AND_EXECUTE(func, fataError(); return;)

/** 输出根据 ret 生成的错误，并返回 ret */
#define PUT_ERROR_AND_RETURN_RET(func) PRINT_ERROR_AND_EXECUTE(func, return ret;)

/** 输出根据 ret 生成的错误，并继续循环 */
#define PUT_ERROR_AND_CONTINUE(func) PRINT_ERROR_AND_EXECUTE(func, continue;)

/** 输出根据 ret 生成的错误，并跳出循环 */
#define PUT_ERROR_AND_BREAK(func) PRINT_ERROR_AND_EXECUTE(func, break;)

//===========================================================================
// VideoPlayer 定义
//===========================================================================

/**
 * @brief 预处理视频
 */
class VideoPlayer : public QObject {
  Q_OBJECT
public:
  explicit VideoPlayer(QObject *parent = nullptr);
  ~VideoPlayer();

  //===========================================================================
  // 通用的结构体与枚举定义
  //===========================================================================
public:
  /**播放状态*/
  typedef enum {
    /**停止*/
    Stopped = 0,
    /**播放中*/
    Playing,
    /**暂停*/
    Paused
  } State;

  /**视频规格*/
  typedef struct {
    int width;            //视频宽
    int height;           //视频高
    AVPixelFormat pixFmt; //像素格式（FFmpeg 中定义的枚举）
    int size;             //一帧图片的字节数
  } VideoSwsSpec;

  /**音频规格*/
  typedef struct {
    int sampleRate;           //采样率
    AVSampleFormat sampleFmt; //采样格式
    int chLayout;             //通道（FFmpeg 定义的标识）
    int chs;                  //通道数
    int bytesPerSampleFrame;  //每个采样帧多少字节（通道数 * 采样格式）
  } AudioSwsSpec;

  /**音量*/
  typedef enum { MIN = 0, MAX = 100 } Volume;

  //===========================================================================
  // 播放状态与播放控制
  //===========================================================================
public:
  void setFilename(QString ilename); //设置播放源

  void play();  //播放
  void pause(); //暂停
  void stop();  //停止

  void setVolumn(int volume); //设置音量
  int getVolumn();            //获取音量
  bool isMute();              //是否静音
  void setMute(bool mute);    //设置是否静音

  void setTime(int time); //跳到指定时间
  int getTime();          //获取当前播放到了的时间
  int getDuration();      //获取总时长（单位是秒）

  bool isPlaying(); //是否正在播放
  State getState(); //获取当前状态

  //===========================================================================
  // 信号
  //===========================================================================
signals:

  void stateChanged(VideoPlayer *player);                                    //信号函数，通知状态变更。
  void timeChanged(VideoPlayer *player);                                     //信号函数，通知播放进度变更。
  void initFinished(VideoPlayer *player);                                    //信号函数，通知设置播放源后，初始化完毕。
  void playFailed(VideoPlayer *player);                                      //信号函数，通知播放出错。
  void frameDecoded(VideoPlayer *player, uint8_t *data, VideoSwsSpec &spec); //信号函数，通知解码出了一帧，需要进行渲染。

  //===========================================================================
  // 音频相关
  //===========================================================================
private:
  AVCodecContext *_aDecodeCtx = nullptr; //音频解码上下文
  AVStream *_aStream = nullptr;          //音频流
  std::list<AVPacket> _aPacketList;      //待解码音频包队列
  CondMutex _aMutex;                     //音频队列锁

  SwrContext *_aSwrCtx = nullptr;   //音频重采样上下文
  AudioSwsSpec _aSwrInSpec;         //音频重采样原规格
  AudioSwsSpec _aSwrOutSpec;        //音频重采样输出规格
  AVFrame *_aSwrInFrame = nullptr;  //音频重采样原帧
  AVFrame *_aSwrOutFrame = nullptr; //音频重采样输出帧

  //下面两个变量用于记录音频解码时，缓冲区的中间状态
  int _aSwrOutIndex = 0; //音频重采样输出PCM的索引（从哪个位置开始取出PCM数据填充到SDL的音频缓冲区）
  int _aSwrOutSize = 0;  //音频重采样输出PCM的大小

  double _aTime = 0;      //音频时钟，当前音频包对应的时间值
  int _aSeekTime = -1;    //外面设置的当前播放时刻（用于完成seek功能）
  bool _hasAudio = false; //是否有音频流
  bool _aCanFree = false; //音频资源是否可以释放了

  int initAudioInfo();                     //初始化音频信息
  int initAudioSwr();                      //初始化音频重采样组件
  int initSDL();                           //初始化SDL，用于音频播放
  void addAudioPacket(AVPacket &avPacket); //添加待解码的音频包
  void clearAudioPacketList();             //清空音频包
  int decodeAudio();                       //解码音频包

  void sdlAudioCallabck(Uint8 *stream, int len);                            // SDL填充缓冲区的回调函数
  static void sdlAudioCallbackFunc(void *userData, Uint8 *stream, int len); // SDL填充缓冲区的回调函数

  //===========================================================================
  // 视频相关
  //===========================================================================
private:
  AVCodecContext *_vDecodeCtx = nullptr; //视频解码上下文
  AVStream *_vStream = nullptr;          //视频流
  std::list<AVPacket> _vPacketList;      //待解码视频包队列
  CondMutex _vMutex;                     //视频队列锁

  SwsContext *_vSwsCtx = nullptr;   //像素格式转换的上下文
  AVFrame *_vSwsInFrame = nullptr;  //视频格式转换原帧
  AVFrame *_vSwsOutFrame = nullptr; //视频格式转换输出帧
  VideoSwsSpec _vSwsOutSpec;        //视频格式输出规格

  double _vTime = 0;      //视频时钟，当前视频包对应的时间值
  bool _vCanFree = false; //视频资源是否可以释放
  int _vSeekTime = -1;    //外面设置的当前播放时刻（用于完成seek功能）
  bool _hasVideo = false; //是否有视频流

  int initVideoInfo();                   //初始化视频信息
  int initVideoSws();                    //初始化视频格式转换组件
  void addVideoPacket(AVPacket &packet); //添加待解码的音频包到队列
  void clearVideoPacketList();           //清空音频包
  void decodeVideo();                    //解码音频包

  //===========================================================================
  // 通用
  //===========================================================================
  AVFormatContext *_fmtCtx = nullptr; //解封装上下文
  bool _fmtCtxCanFree;                //解封装上下文是否可以释放了

  int _volumn = MAX;  //音量
  bool _mute = false; //是否静音

  State _state = Stopped; //当前状态

  char _filename[512]; //文件路径

  int _seekTime = -1; //面设置的当前播放时刻（用于完成seek功能）

  int initDecoder(AVCodecContext **decodeCtx, AVStream **stream, AVMediaType type); //初始化解码器和解码上下文

  void readFile(); //读取文件中的数据

  void setState(State state); //更新播放状态

  void free();      //总的释放函数
  void freeAudio(); //释放音频相关组件（由 free 调用）
  void freeVideo(); //释放视频相关组件（由 free 调用）

  void fataError(); //发生严重错误时调用，用于终止播放
};

#endif // VIDEOPLAYER_H
