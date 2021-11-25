package me.ztiany.androidav.camera.camera2;

import android.content.pm.ActivityInfo;
import android.graphics.Point;
import android.hardware.camera2.CameraDevice;
import android.os.Bundle;
import android.util.Size;
import android.view.TextureView;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatActivity;

import me.ztiany.androidav.R;
import me.ztiany.androidav.common.YUVUtils;
import timber.log.Timber;

public class Camera2EncodingActivity extends AppCompatActivity implements ViewTreeObserver.OnGlobalLayoutListener, Camera2Listener {

    private Camera2Helper camera2Helper;
    private TextureView textureView;

    // 默认打开的 CAMERA
    private static final String CAMERA_ID = Camera2Helper.CAMERA_ID_BACK;

    // 图像帧数据，全局变量避免反复创建，降低gc频率
    private byte[] nv21;

    // 显示的旋转角度
    private int displayOrientation;

    // 是否手动镜像预览
    private boolean isMirrorPreview;

    // 实际打开的cameraId
    private String openedCameraId;

    // 当前获取的帧数
    private int currentIndex = 0;

    private final H265Encoder mH265Encoder = new H265Encoder();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.camera_activity_api2_);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LOCKED);
        initView();
    }

    private void initView() {
        textureView = findViewById(R.id.texture_preview);
        textureView.getViewTreeObserver().addOnGlobalLayoutListener(this);
    }

    void initCamera() {
        camera2Helper = new Camera2Helper.Builder()
                .cameraListener(this)
                .maxPreviewSize(new Point(1920, 1080))
                .minPreviewSize(new Point(1280, 720))
                .specificCameraId(CAMERA_ID)
                .context(getApplicationContext())
                .previewOn(textureView)
                .previewViewSize(new Point(textureView.getWidth(), textureView.getHeight()))
                .rotation(getWindowManager().getDefaultDisplay().getRotation())
                .build();
        camera2Helper.start();
    }

    @Override
    public void onGlobalLayout() {
        textureView.getViewTreeObserver().removeOnGlobalLayoutListener(this);
        initCamera();
    }

    @Override
    protected void onPause() {
        if (camera2Helper != null) {
            camera2Helper.stop();
        }
        if (mH265Encoder != null) {
            mH265Encoder.stop();
        }
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (camera2Helper != null) {
            camera2Helper.start();
        }
    }

    @Override
    public void onCameraOpened(CameraDevice cameraDevice, String cameraId, final Size previewSize, final int displayOrientation, boolean isMirror) {
        Timber.i("onCameraOpened:  previewSize = " + previewSize.getWidth() + "x" + previewSize.getHeight());
        this.displayOrientation = displayOrientation;
        this.isMirrorPreview = isMirror;
        this.openedCameraId = cameraId;
        mH265Encoder.initCodec(previewSize.getHeight(), previewSize.getWidth());
    }

    @Override
    public void onPreview(final byte[] y, final byte[] u, final byte[] v, final Size previewSize, final int stride) {
        if (nv21 == null) {
            nv21 = new byte[stride * previewSize.getHeight() * 3 / 2];
        }
        YUVUtils.yuvToNV21(y, u, v, nv21, stride, previewSize.getHeight());
        mH265Encoder.processCamaraData(nv21, previewSize, stride, displayOrientation, isMirrorPreview, openedCameraId, currentIndex++);
    }

    @Override
    public void onCameraClosed() {
        Timber.i("onCameraClosed: ");
    }

    @Override
    public void onCameraError(Exception e) {
        e.printStackTrace();
    }

    @Override
    protected void onDestroy() {
        if (camera2Helper != null) {
            camera2Helper.release();
        }
        super.onDestroy();
    }

    public void switchCamera(View view) {
        if (camera2Helper != null) {
            camera2Helper.switchCamera();
        }
    }

}
