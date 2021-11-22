package me.ztiany.androidav.android.h265.livescreen.receiver;

import android.util.Log;

import org.java_websocket.WebSocket;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.nio.ByteBuffer;

import timber.log.Timber;

public class SocketLiveClient {

    private final SocketCallback socketCallback;
    private DemoWebSocketClient mDemoWebSocketClient;
    private final int port;

    public SocketLiveClient(SocketCallback socketCallback, int port) {
        this.socketCallback = socketCallback;
        this.port = port;
    }

    public void start() {
        try {
            URI url = new URI("ws://192.168.8.100:" + port);
            mDemoWebSocketClient = new DemoWebSocketClient(url);
            mDemoWebSocketClient.connect();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private class DemoWebSocketClient extends WebSocketClient {

        public DemoWebSocketClient(URI serverURI) {
            super(serverURI);
        }

        @Override
        public void onOpen(ServerHandshake serverHandshake) {
            Timber.i("打开 socket");
        }

        @Override
        public void onMessage(String s) {
        }

        @Override
        public void onMessage(ByteBuffer bytes) {
            Timber.i("消息长度：%s", bytes.remaining());
            byte[] buf = new byte[bytes.remaining()];
            bytes.get(buf);
            socketCallback.callBack(buf);
        }

        @Override
        public void onClose(int i, String s, boolean b) {
            Timber.i("onClose");
        }

        @Override
        public void onError(Exception e) {
            Timber.i("onError");
        }
    }

    public interface SocketCallback {
        void callBack(byte[] data);
    }

    public void stop() {
        mDemoWebSocketClient.close();
    }

}
