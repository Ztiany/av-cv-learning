package me.ztiany.androidav.camera.camera2;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.util.Size;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;

import me.ztiany.androidav.common.FileUtils;
import me.ztiany.androidav.common.YUVUtils;

class H265Encoder {

    private MediaCodec mediaCodec;

    // 图像帧数据，全局变量避免反复创建，降低gc频率
    private byte[] nv21;
    private byte[] nv21_rotated;
    private byte[] nv12;

    void initCodec(Size size) {
        try {
            mediaCodec = MediaCodec.createEncoderByType("video/avc");
            final MediaFormat format = MediaFormat.createVideoFormat("video/avc", size.getHeight(), size.getWidth());
            //设置帧率
            format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
            format.setInteger(MediaFormat.KEY_FRAME_RATE, 15);
            format.setInteger(MediaFormat.KEY_BIT_RATE, 4000_000);
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2);//2s一个I帧
            mediaCodec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            mediaCodec.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    void processCamaraData(byte[] nv21, Size previewSize, int stride) {
        if (nv21 == null) {
            nv21 = new byte[stride * previewSize.getHeight() * 3 / 2];
            nv21_rotated = new byte[stride * previewSize.getHeight() * 3 / 2];
            nv12 = new byte[stride * previewSize.getHeight() * 3 / 2];
        }
        if (mediaCodec == null) {
            initCodec(previewSize);
        }

        YUVUtils.rotateNV21CW(nv21, nv21_rotated, previewSize.getWidth(), previewSize.getHeight(), 90);
        YUVUtils.nv21toNV12(nv21_rotated, nv12);

        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        int inIndex = mediaCodec.dequeueInputBuffer(100000);
        if (inIndex >= 0) {
            ByteBuffer byteBuffer = mediaCodec.getInputBuffer(inIndex);
            byteBuffer.clear();
            byteBuffer.put(nv12, 0, nv12.length);
            mediaCodec.queueInputBuffer(inIndex, 0, nv12.length, 0, 0);
        }
        int outIndex = mediaCodec.dequeueOutputBuffer(info, 100000);
        if (outIndex >= 0) {
            ByteBuffer byteBuffer = mediaCodec.getOutputBuffer(outIndex);
            byte[] ba = new byte[byteBuffer.remaining()];
            byteBuffer.get(ba);
            //TODO
            FileUtils.writeContent(ba, new File(""), true);
            FileUtils.writeBytes(ba, new File(""), true);
            mediaCodec.releaseOutputBuffer(outIndex, false);
        }
    }

    void stop() {
        if (mediaCodec != null) {
            mediaCodec.stop();
            mediaCodec.release();
        }
    }

}
