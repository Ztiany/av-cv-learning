package me.ztiany.androidav.android.camera;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.os.Environment;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class DemoSurfaceView extends SurfaceView implements SurfaceHolder.Callback, Camera.PreviewCallback {

    private Camera.Size size;
    private Camera mCamera;

    public DemoSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

        startPreview();

    }
    byte[] buffer;
    private void startPreview() {
        mCamera = Camera.open(Camera.CameraInfo.CAMERA_FACING_BACK);
//        流程
        Camera.Parameters parameters = mCamera.getParameters();
//尺寸
        size = parameters.getPreviewSize();
        try {
            mCamera.setPreviewDisplay(getHolder());
//            横着
            mCamera.setDisplayOrientation(90);
            buffer = new byte[size.width * size.height * 3 / 2];
            mCamera.addCallbackBuffer(buffer);

            mCamera.setPreviewCallbackWithBuffer(this);
//            输出数据怎么办
            mCamera.startPreview();
        } catch (IOException e) {
            e.printStackTrace();
        }


    }
    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }
//接受到
   private volatile boolean isCaptrue;
    public void startCaptrue(){
        isCaptrue = true;
    }
//    画面    原始数据   编码  视频
    @Override
    public void onPreviewFrame(byte[] bytes, Camera camera) {
        if (isCaptrue) {
            portraitData2Raw(bytes);
            isCaptrue = false;
            captrue(buffer);
        }
        mCamera.addCallbackBuffer(bytes);
    }
//    画面旋转90度  rtmp 推流  软解推流
    private void portraitData2Raw(byte[] data) {
        int width = size.width;
        int height =size.height;
//        旋转y
        int y_len = width * height;
//u   y/4   v  y/4
        int uvHeight = height/2;

        int k = 0;
        for (int j = 0; j < width; j++) {
            for (int i = height - 1; i >= 0; i--) {
//                存值  k++  0          取值  width * i + j
                buffer[k++] = data[width * i + j];
            }
        }
//        旋转uv

        for (int j = 0; j < width; j += 2) {
            for (int i = uvHeight - 1; i >= 0; i--) {
                buffer[k++] = data[y_len + width * i + j];
                buffer[k++] = data[y_len + width * i + j + 1];
            }
        }
    }
    int index = 0;

    public void captrue2(byte[] temp){
        mCamera.takePicture(null, null, (data, camera) -> {

        });
    }

    public void captrue(byte[] temp){

        //保存一张照片
        String fileName = "IMG_" + String.valueOf(index++) + ".jpg";  //jpeg文件名定义
        File sdRoot = Environment.getExternalStorageDirectory();    //系统路径

        File pictureFile = new File(sdRoot,   fileName);
        if (!pictureFile.exists()) {
            try {
                pictureFile.createNewFile();

                FileOutputStream filecon = new FileOutputStream(pictureFile);
//ImageFormat.NV21 and ImageFormat.YUY2 for now
                YuvImage image = new YuvImage(temp, ImageFormat.NV21,size.height, size.width,null);   //将NV21 data保存成YuvImage
                //图像压缩
                image.compressToJpeg(
                        new Rect(0, 0, image.getWidth(), image.getHeight()),
                        100, filecon);   // 将NV21格式图片，以质量70压缩成Jpeg，并得到JPEG数据流

            }catch (IOException e)
            {
                e.printStackTrace();
            }
        }
    }


}
