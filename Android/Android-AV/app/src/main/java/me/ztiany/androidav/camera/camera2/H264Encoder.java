package me.ztiany.androidav.camera.camera2;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.util.Size;

import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingDeque;

import me.ztiany.androidav.common.Directory;
import me.ztiany.androidav.common.FileUtils;
import me.ztiany.androidav.common.IOUtils;
import me.ztiany.androidav.common.YUVUtils;
import timber.log.Timber;

class H264Encoder {

    private final LinkedBlockingDeque<byte[]> mLinkedBlockingDeque = new LinkedBlockingDeque<>();

    private volatile MediaCodec mediaCodec;
    private volatile boolean stopped = false;

    private byte[] nv21_rotated;
    private byte[] nv12;

    private int mFrameIndex;

    private FileOutputStream mFileOutputStream;
    private FileWriter mFileWriter;

    void initCodec(int width, int height) {
        try {
            mFileOutputStream = new FileOutputStream(Directory.createDCIMPicturePath(Directory.VIDEO_FORMAT_H264));
            mFileWriter = new FileWriter(Directory.createDCIMPicturePath(Directory.VIDEO_FORMAT_TXT));
        } catch (IOException e) {
            Timber.e(e);
            return;
        }

        try {
            mediaCodec = MediaCodec.createEncoderByType("video/avc");
            final MediaFormat format = MediaFormat.createVideoFormat("video/avc", width, height);
            //COLOR_FormatYUV420SemiPlanar
            format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar);
            format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
            format.setInteger(MediaFormat.KEY_FRAME_RATE, 15);
            format.setInteger(MediaFormat.KEY_BIT_RATE, width * height * 3);//可以为 1 3 5 等来控制质量
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2);
            mediaCodec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            mediaCodec.start();
        } catch (IOException e) {
            Timber.e(e);
            return;
        }

        startEncoder();
    }

    void processCamaraData(byte[] nv21, Size previewSize, int stride, int displayOrientation, boolean isMirrorPreview, String openedCameraId) {
        if (nv21_rotated == null) {
            nv21_rotated = new byte[previewSize.getWidth() * previewSize.getHeight() * 3 / 2];
            nv12 = new byte[previewSize.getWidth() * previewSize.getHeight() * 3 / 2];
        }

        YUVUtils.nv21RotateCW(nv21, nv21_rotated, previewSize.getWidth(), previewSize.getHeight(), 90);
        YUVUtils.nv12FromNV21(nv21_rotated, nv12);

        while (mLinkedBlockingDeque.size() > 3) {
            mLinkedBlockingDeque.removeFirst();
            Timber.w("drop a frame");
        }

        byte[] bytes = new byte[nv12.length];
        System.arraycopy(nv12, 0, bytes, 0, nv12.length);
        mLinkedBlockingDeque.add(bytes);
    }

    void stop() {
        mLinkedBlockingDeque.clear();
        stopped = true;
        if (mediaCodec != null) {
            mediaCodec.stop();
            mediaCodec.release();
            mediaCodec = null;
        }
        IOUtils.closeAllSafely(mFileOutputStream, mFileWriter);
    }

    private void startEncoder() {
        new Thread(() -> {
            while (!stopped) {
                try {
                    byte[] rawData = mLinkedBlockingDeque.takeFirst();
                    if (mediaCodec != null) {
                        encode(rawData);
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }).start();

        new Thread(this::receiveDecoded).start();
    }

    private void receiveDecoded() {
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        int outIndex;
        ByteBuffer byteBuffer;

        while (!stopped && mediaCodec != null) {
            try {
                outIndex = mediaCodec.dequeueOutputBuffer(info, 100000);
            } catch (IllegalStateException e) {
                Timber.e(e);
                return;
            }

            if (outIndex < 0) {
                Timber.w("outIndex < 0");
                continue;
            }

            try {
                byteBuffer = mediaCodec.getOutputBuffer(outIndex);
            } catch (IllegalStateException e) {
                Timber.e(e);
                return;
            }
            byte[] encoded = new byte[byteBuffer.remaining()];
            byteBuffer.get(encoded);
            FileUtils.writeBytes(mFileOutputStream, encoded);
            FileUtils.writeContent(mFileWriter, encoded);
            mediaCodec.releaseOutputBuffer(outIndex, false);
        }
    }

    void encode(byte[] rawData) {
        int inIndex = mediaCodec.dequeueInputBuffer(100000);
        if (inIndex < 0) {
            Timber.w("inIndex < 0");
        }
        if (inIndex >= 0) {
            ByteBuffer byteBuffer = mediaCodec.getInputBuffer(inIndex);
            byteBuffer.clear();
            byteBuffer.put(rawData, 0, rawData.length);
            mediaCodec.queueInputBuffer(inIndex, 0, rawData.length, computePresentationTime(mFrameIndex++), 0);
        }

    }

    private long computePresentationTime(long frameIndex) {
        return 240 + frameIndex * 1000000 / 15;
    }

}