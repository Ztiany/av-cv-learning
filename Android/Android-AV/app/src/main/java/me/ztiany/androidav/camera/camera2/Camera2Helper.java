package me.ztiany.androidav.camera.camera2;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

public class Camera2Helper {

    /**
     * 屏幕的长宽，在选择最佳相机比例时用到
     */
    private Point previewViewSize;
    private Size mPreviewSize;

    private Camera2Listener camera2Listener;
    private Handler mBackgroundHandler;
    private HandlerThread mBackgroundThread;

    private ImageReader mImageReader;
    private TextureView mTextureView;
    private CameraDevice mCameraDevice;
    private CameraCaptureSession mCaptureSession;
    CaptureRequest.Builder mPreviewRequestBuilder;
    private final Context context;

    public Camera2Helper(Context context) {
        this.context = context;
        this.camera2Listener = (Camera2Listener) context;
    }

    public synchronized void start(TextureView textureView) {
        this.mTextureView = textureView;
        startBackgroundThread();
        openCamera();
    }

    /**
     * Starts a background thread and its {@link Handler}.
     */
    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("CameraBackground");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    public synchronized void openCamera() {
        CameraManager cameraManager = (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
        try {
            CameraCharacteristics characteristics = cameraManager.getCameraCharacteristics("0");
            //获取图片输出的尺寸和预览画面输出的尺寸
            StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            assert map != null;
            mPreviewSize = getBestSupportedSize(new ArrayList<>(Arrays.asList(map.getOutputSizes(SurfaceTexture.class))));
            // 获取读取流数据
            mImageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(), ImageFormat.YUV_420_888, 2);
            mImageReader.setOnImageAvailableListener(new OnImageAvailableListenerImpl(), mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        try {
            if (ActivityCompat.checkSelfPermission(context, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            //打开相机
            cameraManager.openCamera("0", mDeviceStateCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private final CameraDevice.StateCallback mDeviceStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            mCameraDevice = cameraDevice;
            createCameraPreviewSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int error) {
            cameraDevice.close();
            mCameraDevice = null;
        }
    };

    private void createCameraPreviewSession() {
        SurfaceTexture texture = mTextureView.getSurfaceTexture();
        //设置texture 预览大小 与摄像头输出对其
        texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
        //创建surface
        Surface surface = new Surface(texture);
        try {
            // 设置预览输出画面
            mPreviewRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            //添加输出surface
            mPreviewRequestBuilder.addTarget(surface);
            //再输出到ImagerReader
            mPreviewRequestBuilder.addTarget(mImageReader.getSurface());
            //创建详解会话
            mCameraDevice.createCaptureSession(Arrays.asList(surface, mImageReader.getSurface()), mCaptureStateCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private final CameraCaptureSession.StateCallback mCaptureStateCallback = new CameraCaptureSession.StateCallback() {

        @Override
        public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
            // The camera is already closed
            if (null == mCameraDevice) {
                return;
            }
            // When the session is ready, we start displaying the preview.
            mCaptureSession = cameraCaptureSession;
            try {
                mCaptureSession.setRepeatingRequest(mPreviewRequestBuilder.build(), new CameraCaptureSession.CaptureCallback() {

                }, mBackgroundHandler);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession cameraCaptureSession) {
        }

    };

    public interface Camera2Listener {

        /**
         * 预览数据回调
         *
         * @param y           预览数据，Y分量
         * @param u           预览数据，U分量
         * @param v           预览数据，V分量
         * @param previewSize 预览尺寸
         * @param stride      步长
         */
        void onPreview(byte[] y, byte[] u, byte[] v, Size previewSize, int stride);
    }

    private class OnImageAvailableListenerImpl implements ImageReader.OnImageAvailableListener {

        private byte[] y;
        private byte[] u;
        private byte[] v;

        private final ReentrantLock lock = new ReentrantLock();

        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireNextImage();
            // Y:U:V == 4:2:2
            if (camera2Listener != null && image.getFormat() == ImageFormat.YUV_420_888) {
                Image.Plane[] planes = image.getPlanes();
                // 加锁确保y、u、v来源于同一个Image
                lock.lock();
                // 重复使用同一批byte数组，减少gc频率
                if (y == null) {
                    y = new byte[planes[0].getBuffer().limit() - planes[0].getBuffer().position()];
                    u = new byte[planes[1].getBuffer().limit() - planes[1].getBuffer().position()];
                    v = new byte[planes[2].getBuffer().limit() - planes[2].getBuffer().position()];
                }
                if (image.getPlanes()[0].getBuffer().remaining() == y.length) {
                    planes[0].getBuffer().get(y);
                    planes[1].getBuffer().get(u);
                    planes[2].getBuffer().get(v);
                    camera2Listener.onPreview(y, u, v, mPreviewSize, planes[0].getRowStride());
                }
                lock.unlock();
            }
            image.close();
        }
    }

    private Size getBestSupportedSize(List<Size> sizes) {
        Point maxPreviewSize = new Point(1920, 1080);
        Point minPreviewSize = new Point(1280, 720);

        Size defaultSize = sizes.get(0);
        Size[] tempSizes = sizes.toArray(new Size[0]);

        Arrays.sort(tempSizes, (o1, o2) -> {
            if (o1.getWidth() > o2.getWidth()) {
                return -1;
            } else if (o1.getWidth() == o2.getWidth()) {
                return o1.getHeight() > o2.getHeight() ? -1 : 1;
            } else {
                return 1;
            }
        });

        sizes = new ArrayList<>(Arrays.asList(tempSizes));

        for (int i = sizes.size() - 1; i >= 0; i--) {
            if (sizes.get(i).getWidth() > maxPreviewSize.x || sizes.get(i).getHeight() > maxPreviewSize.y) {
                sizes.remove(i);
                continue;
            }
            if (sizes.get(i).getWidth() < minPreviewSize.x || sizes.get(i).getHeight() < minPreviewSize.y) {
                sizes.remove(i);
            }
        }

        if (sizes.size() == 0) {
            return defaultSize;
        }

        Size bestSize = sizes.get(0);
        float previewViewRatio;

        if (previewViewSize != null) {
            previewViewRatio = (float) previewViewSize.x / (float) previewViewSize.y;
        } else {
            previewViewRatio = (float) bestSize.getWidth() / (float) bestSize.getHeight();
        }

        if (previewViewRatio > 1) {
            previewViewRatio = 1 / previewViewRatio;
        }

        for (Size s : sizes) {
            if (Math.abs((s.getHeight() / (float) s.getWidth()) - previewViewRatio) < Math.abs(bestSize.getHeight() / (float) bestSize.getWidth() - previewViewRatio)) {
                bestSize = s;
            }
        }
        return bestSize;
    }

}