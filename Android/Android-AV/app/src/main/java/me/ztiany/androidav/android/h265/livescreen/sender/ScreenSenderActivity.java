package me.ztiany.androidav.android.h265.livescreen.sender;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Bundle;

public class ScreenSenderActivity extends AppCompatActivity {

    private MediaProjectionManager mediaProjectionManager;
    private SocketLiveServer mSocketLiveServer;
    private static final int REQUEST_CODE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mediaProjectionManager = (MediaProjectionManager) getSystemService(MEDIA_PROJECTION_SERVICE);
        Intent captureIntent = mediaProjectionManager.createScreenCaptureIntent();
        startActivityForResult(captureIntent, REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode != RESULT_OK || requestCode != REQUEST_CODE) {
            return;
        }

        MediaProjection mediaProjection = mediaProjectionManager.getMediaProjection(resultCode, data);
        if (mediaProjection == null) {
            return;
        }

        mSocketLiveServer = new SocketLiveServer(12001);
        mSocketLiveServer.start(mediaProjection);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mSocketLiveServer.close();
    }

}