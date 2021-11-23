package me.ztiany.androidav.video;

public interface SocketLive {

    void start();

    void close();

    void sendData(byte[] bytes);

    interface SocketCallback {
        void callBack(byte[] data);
    }

}
