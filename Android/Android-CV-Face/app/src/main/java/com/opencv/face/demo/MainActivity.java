package com.opencv.face.demo;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class MainActivity extends Activity implements SurfaceHolder.Callback, Camera.PreviewCallback {

    static {
        System.loadLibrary("native-lib");
    }

    private CameraHelper cameraHelper;
    int cameraId = Camera.CameraInfo.CAMERA_FACING_FRONT;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        checkPermission();
        surfaceView.getHolder().addCallback(this);
        cameraHelper = new CameraHelper(cameraId);
        cameraHelper.setPreviewCallback(this);
        Utils.copyAssets(this, "lbpcascade_frontalface.xml");
    }

    public void checkPermission() {
        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.CAMERA
            }, 1);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        //初始化跟踪器
        init("/sdcard/lbpcascade_frontalface.xml");
        cameraHelper.startPreview();
    }

    @Override
    protected void onStop() {
        super.onStop();
        //释放跟踪器
        release();
        cameraHelper.stopPreview();
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        //设置surface 用于显示
        setSurface(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        //传输数据
        postData(data, CameraHelper.WIDTH, CameraHelper.HEIGHT, cameraId);
    }

    /**
     * 初始化 追踪器
     */
    native void init(String model);

    /**
     * 设置画布 ANativeWindow
     */
    native void setSurface(Surface surface);

    /**
     * 处理摄像头数据
     */
    native void postData(byte[] data, int w, int h, int cameraId);

    /**
     * 释放
     */
    native void release();

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_UP) {
            cameraHelper.switchCamera();
            cameraId = cameraHelper.getCameraId();
        }
        return super.onTouchEvent(event);
    }

}
