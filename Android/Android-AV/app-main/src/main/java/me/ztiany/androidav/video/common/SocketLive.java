package me.ztiany.androidav.video.common;

public interface SocketLive {

    void start();

    void close();

    void sendData(byte[] bytes);

    interface SocketCallback {
        void callBack(byte[] data);
    }

}
