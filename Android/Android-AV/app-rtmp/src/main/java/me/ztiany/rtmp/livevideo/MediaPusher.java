package me.ztiany.rtmp.livevideo;

public class MediaPusher {

    static {
        System.loadLibrary("android_rtmp");
    }

    native void initPusher(String url);

    native void releasePusher();
}
