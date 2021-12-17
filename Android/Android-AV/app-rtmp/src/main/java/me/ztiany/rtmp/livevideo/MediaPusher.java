package me.ztiany.rtmp.livevideo;

public class MediaPusher {

    static {
        System.loadLibrary("android_rtmp");
    }

    private final MediaPusher mMediaPusher = new MediaPusher();

    private MediaPusher() {

    }

    public MediaPusher getMediaPusher() {
        return mMediaPusher;
    }

    public native void init();

    public native void start(String url);

    public native void initVideoCodec(int width, int height, int fps, int bitrate);

    public native void initAudioCodec(int sampleRate, int channels);

    public native void sendVideoPacket(byte[] data, int videoType);

    public native void sendAudioPacket(byte[] data, int audioType);

    public native void stop();

    public native void release();

}
