# 小码哥音视频教程【代码与笔记】

## 1 运行环境说明

`02_HelloFFmpeg` 到 `15_record_audio_wav`：

- ffmpeg 版本：`4.3.2-2021-02-27-full_build-www.gyan.dev`
- sdl 版本：`2.0.14-mingw`

`16_aac_encode` 到最后

- ffmpeg 版本是自己编译的版本，具体参考 [【秒懂音视频开发】14_编译FFmpeg](https://www.cnblogs.com/mjios/p/14633516.html)。

## 2 代码说明

### QT 基础编程、集成 FFmpeg

1. `01-HelloQT`：QT 项目示例。
2. `02_HelloFFmpeg`：QT 中集成 FFmpeg。
3. `03_QtBase`：QT 基础编程。
4. `05_signal_and_slot`：QT 信号与槽。
5. `06_diy_signal_and_slot`：QT 自定义信号与槽。
6. `07_signal_and_slots_ui`：QT 在 UI 上定义信号与槽。

### 使用 FFmpeg 录制 PCM 音频数据

1. `08_record_audio`：使用 FFmpeg 录制 PCM 音频数据。
2. `09_record_audio_thread`：子线程中使用 FFmpeg 录制 PCM 音频数据。
3. `09_record_audio_thread_correct`：基于 `09_record_audio_thread`，正确地处理录音过程中可能存在的错误。

### 使用 sdl 库播放 pcm 数据

1. `10_sdl_play_pcm`：使用 sdl 库播放 pcm 数据。
2. `10_sdl_play_pcm_optimized`：基于 `10_sdl_play_pcm` 的优化，去掉读取音频数据时的循环等待。
3. `11_sdl_play_pcm_userdata`：基于 `10_sdl_play_pcm_optimized`：基于的优化，等待播放器消费完所有数据才退出线程。

### 使用 FFmpeg 将 PCM 转换为 WAV

`12_pcm_to_wav`：使用 FFmpeg 将 PCM 转换为 WAV。

### 使用 FFmpeg 对 PCM 进行重采样

`13_audio_resample`：使用 FFmpeg 对 PCM 进行重采样。

### 使用 SDL 播放 WAV

`14_sdl_play_wav`：使用 SDL 播放 WAV

### 使用 FFmpeg 直接录制 WAV

`15_record_audio_wav`：使用 FFmpeg 直接录制 WAV

### 使用 FFmpeg 将 PCM 编码为 AAC

`16_aac_encode`：使用 FFmpeg 将 PCM 编码为 AAC。

### 使用 FFmpeg 将 AAC 解编码为 PCM

`17_aac_decode`：使用 FFmpeg 将  AAC 解编码为 PCM。

### 使用 FFmpeg 将录制 YUV 数据

`18_record_video`：使用 FFmpeg 将录制 YUV 数据。

### 使用 SDL 展示图片

- `19_sdl_show_bmp`：使用 SDL 展示 BMP 图片。
- `20_sdl_draw`：使用 SDL 绘制图形。
- `21_sdl_show_yuv`：使用 SDL 展示 YUV 图片。
- `22_sdl_show_embed`：使用 SDL 在 Qt 窗口展示 YUV 图片。

### 使用 SDL 播放 YUV 视频

`24_yuv_player_sdl`：使用 SDL 播放 YUV 视频。

- 好处是：SDL 默认就支持 YUV 视频。
- 缺点是：SDL 窗口与 QT 窗口渲染机制不一致，有兼容性问题。

### 使用 FFmpeg +Qt 播放 YUV

`25_yuv_player_ffmpeg`：使用 FFmpeg + Qt 播放 YUV 视频。存在的问题：

1. 视频没有居中。
2. 没有维护视频播放的状态。

`25_yuv_player_ffmpeg_center`：使用 FFmpeg + Qt 播放 YUV 视频。基于 `25_yuv_player_ffmpeg` 优化如下：

1. 动态调整视频，始终居中播放。
2. 维护视频状态。
3. 支持一次性设置多个视频，轮流播放。

### 使用 FFmpeg 将 YUV 编码为 H264

`26_h264_encode`：使用 FFmpeg 将 YUV 编码为 H264。

### 使用 FFmpeg 将 H264 解编码为 YUV

`27_h264_decode`：使用 FFmpeg 将 H264 解编码为 YUV。

### 使用 FFmpeg 对 Mp4 进行解封装和解码

`28_demuxer`：使用 FFmpeg 对 Mp4 进行解封装和解码。

### 本地播放器

`29_video_player`：使用 FFmpeg + QT 实现本地播放器。

### 多线程互斥锁演示

1. `30_mutex_cond_01`
2. `31_mutex_cond_02`
