package me.ztiany.androidav.player.mediacodec;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioTrack;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaSync;
import android.media.PlaybackParams;
import android.media.SyncParams;
import android.util.Log;
import android.view.Surface;
import android.view.View;

import androidx.annotation.NonNull;

import java.io.IOException;
import java.nio.ByteBuffer;

public class MyMedia {

    static final String TAG = "MyMedia";

    private String mContentUri = "https://ia600603.us.archive.org/30/items/Tears-of-Steel/tears_of_steel_1080p.mp4";
//    private String mContentUri = "https://scontent-nrt1-1.cdninstagram.com/v/t50.2886-16/41638619_239560346735367_5701419805668761311_n.mp4?_nc_ht=scontent-nrt1-1.cdninstagram.com&_nc_cat=103&_nc_ohc=NN0ddOIY3fUAX81uvbP&oe=5E5992B3&oh=f3f3097a6daa345e82fc5c0f12c7cb24";

    private boolean mRunning;
    private View mView;
    private Surface mGivenSurface, mSurface;
    private AudioTrack mAudioTrack;
    private MediaSync mSync;
    private MediaExtractor mVideoExtractor, mAudioExtractor;
    private MediaCodec mVideoCodec, mAudioCodec;


    static private MediaExtractor createExtractor(String uri){
        MediaExtractor extractor = new MediaExtractor();
        try {
            extractor.setDataSource(uri);
        } catch (IOException e) {
            e.printStackTrace();
            extractor.release();
            return null;
        }

        return extractor;
    }


    static private int[] findTrackAVIndex(MediaExtractor extractor) {
        int[] ii = new int[2];
        ii[0] = -1; // video
        ii[1] = -1; // audio

        for (int i = 0, n = extractor.getTrackCount(); i < n; ++i) {
            MediaFormat format = extractor.getTrackFormat(i);
            String mime = format.getString(MediaFormat.KEY_MIME);
            Log.d(TAG, "track " + i + " : key_mime = " + mime);
            if (mime == null) continue;
            if (mime.contains("video"))
                ii[0] = i;
            else if (mime.contains("audio"))
                ii[1] = i;
        }
        return ii;
    }


    static private MediaCodec createMediaCodec(MediaFormat format){
        assert format != null;
        String codecName = new MediaCodecList(MediaCodecList.ALL_CODECS).findDecoderForFormat(format);
        MediaCodec codec = null;
        try {
            String mime = format.getString(MediaFormat.KEY_MIME);
            Log.d(TAG, "try to create a codec mime="+mime+" codecName="+codecName);
            if (codecName != null)
                codec = MediaCodec.createByCodecName(codecName);
            else if (mime != null)
                codec = MediaCodec.createDecoderByType(mime);  // may be throw IllegalArgumentException
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return codec;
    }


    static private MediaCodec createVideoDecoder(MediaFormat format, Surface surface) {
        MediaCodec codec = createMediaCodec(format);
        if (codec == null)
            return null;
        codec.configure(format, surface, null, 0);
        return codec;
    }


    static private MediaCodec createAudioDecoder(MediaFormat format) {
        MediaCodec codec = createMediaCodec(format);
        if (codec == null)
            return null;
        codec.configure(format, null, null, 0);
        return codec;
    }


    static private AudioFormat createAudioFormatFromMediaFormat(MediaFormat mediaFormat) {
        AudioFormat.Builder b = new AudioFormat.Builder();
        if (mediaFormat.containsKey(MediaFormat.KEY_SAMPLE_RATE)) {
            b.setSampleRate(mediaFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE));
        }
        else {
            Log.w(TAG, "createAudioFormatFormMediaFormat: not found KEY_SAMPLE_RATE");
            b.setSampleRate(44100);
        }
        if (mediaFormat.containsKey(MediaFormat.KEY_PCM_ENCODING)) {
            b.setEncoding(mediaFormat.getInteger(MediaFormat.KEY_PCM_ENCODING));
        }
        else {
            Log.w(TAG, "createAudioFormatFormMediaFormat: not found KEY_PCM_ENCODING");
            b.setEncoding(AudioFormat.ENCODING_PCM_16BIT);
        }
        if (mediaFormat.containsKey(MediaFormat.KEY_CHANNEL_MASK)) {
            b.setChannelMask(mediaFormat.getInteger(MediaFormat.KEY_CHANNEL_MASK));
        }
        else {
            Log.w(TAG, "createAudioFormatFormMediaFormat: not found KEY_CHANNEL_MASK");
            b.setChannelMask(AudioFormat.CHANNEL_OUT_STEREO);
        }

        return b.build();
    }


    static private AudioTrack createAudioTrack(AudioFormat audioFormat) {
        int size = AudioTrack.getMinBufferSize(audioFormat.getSampleRate(),
                audioFormat.getChannelMask(), audioFormat.getEncoding());
        return new AudioTrack.Builder()
                .setAudioAttributes(new AudioAttributes.Builder()
                        .setUsage(AudioAttributes.USAGE_MEDIA)
                        .setContentType(AudioAttributes.CONTENT_TYPE_MOVIE)
                        .build())
                .setAudioFormat(audioFormat)
                .setTransferMode(AudioTrack.MODE_STREAM)
                .setBufferSizeInBytes(size)
                .build();
    }


    static private void dumpMediaCodecList() {
        MediaCodecList mcl = new MediaCodecList(MediaCodecList.ALL_CODECS);
        for (MediaCodecInfo mci : mcl.getCodecInfos()) {
            for (String type : mci.getSupportedTypes()) {
                Log.d(TAG, mci.getName() + " supports " + type);
            }
        }
    }


    public MyMedia(Surface surface, View view) {
        assert surface != null;
        mRunning = false;
        mView = view;
        mGivenSurface = surface;
        dumpMediaCodecList();
    }


    public boolean initialize() {
        Log.d(TAG, "[1]initialize");
        mRunning = false;

        MediaExtractor extractor = createExtractor(mContentUri);
        if (extractor == null) {
            Log.w(TAG, "createExtractor failed.");
            return false;
        }

        mSync = new MediaSync();
        mSync.setPlaybackParams(new PlaybackParams().setSpeed(0.f));
        mSync.setSurface(mGivenSurface);
        mSurface = mSync.createInputSurface();

        int videoTrackIndex, audioTrackIndex;
        {
            int[] ii = findTrackAVIndex(extractor);
            videoTrackIndex = ii[0];
            audioTrackIndex = ii[1];
            if (videoTrackIndex < 0) {
                Log.w(TAG, "videoTrack not found");
                return false;
            }
            if (audioTrackIndex < 0) {
                Log.w(TAG, "audioTrack not found");
                return false;
            }
            Log.d(TAG, "videoTrackIndex=" + videoTrackIndex + " audioTrackIndex="+audioTrackIndex);
        }
        MediaFormat audioMediaFormat = extractor.getTrackFormat(audioTrackIndex);

        mVideoExtractor = createExtractor(mContentUri);
        assert mVideoExtractor != null;
        mVideoExtractor.selectTrack(videoTrackIndex);
        mVideoCodec = createVideoDecoder(extractor.getTrackFormat(videoTrackIndex), mSurface);

        mAudioExtractor = createExtractor(mContentUri);
        assert mAudioExtractor != null;
        mAudioExtractor.selectTrack(audioTrackIndex);
        mAudioCodec = createAudioDecoder(extractor.getTrackFormat(audioTrackIndex));

        mAudioTrack = createAudioTrack(createAudioFormatFromMediaFormat(audioMediaFormat));
        mSync.setAudioTrack(mAudioTrack);

        mSync.setSyncParams(new SyncParams()
                .setSyncSource(SyncParams.SYNC_SOURCE_SYSTEM_CLOCK));

        Log.d(TAG, "[2]initialize");
        return true;
    }


    public void release() {
        Log.d(TAG, "[1]release");
        mRunning = false;
        mVideoExtractor.release();
        mVideoCodec.release();
        mAudioExtractor.release();
        mAudioCodec.release();
        mAudioTrack.release();
        mSync.release();
        Log.d(TAG, "[2]release");
    }


    void run() {
        Log.d(TAG, "[1]run");

        mRunning = true;

        mVideoCodec.setCallback(new CodecCallback(mVideoExtractor, mSync, false));
        mAudioCodec.setCallback(new CodecCallback(mAudioExtractor, mSync, true));

        mSync.setCallback(new MediaSync.Callback() {
            @Override
            public void onAudioBufferConsumed(MediaSync sync, ByteBuffer audioBuffer, int bufferId) {
                Log.d(TAG, "onAudioBufferConsumed " + bufferId);
                mAudioCodec.releaseOutputBuffer(bufferId, true);
            }
        }, null);// This needs to be done since sync is paused on creation.

        mVideoCodec.start();
        mAudioCodec.start();

        new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "ready...");
                try {
                    Thread.sleep(2000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                mSync.setPlaybackParams(new PlaybackParams().setSpeed(1.f));
                Log.d(TAG, "play !!!!!!");
                ;
            }
        }).start();

        Log.d(TAG, "[2]run");
    }


    class CodecCallback extends MediaCodec.Callback {
        MediaExtractor mExtractor;
        MediaSync mSync;
        boolean mIsAudio;

        CodecCallback(MediaExtractor extractor, MediaSync sync, boolean isAudio) {
            mExtractor = extractor;
            mSync = sync;
            mIsAudio = isAudio;
        }

        private String getTag() {
            return mIsAudio ? "Audio" : "Video";
        }

        @Override
        public void onInputBufferAvailable(@NonNull MediaCodec mediaCodec, int index) {
            Log.d(TAG, "onInputBufferAvailable " + getTag() + " i=" + index);
            ByteBuffer buffer = mediaCodec.getInputBuffer(index);
            if (buffer == null) {
                Log.w(TAG, "codec buffer is null");
                return;
            }
            int size = mExtractor.readSampleData(buffer, 0);
            if (size < 0) {
                Log.w(TAG, "track empty");
                return;
            }
            long time = mExtractor.getSampleTime();
            Log.d(TAG, "sampleTime=" + time);
            mediaCodec.queueInputBuffer(
                    index, 0, size,
                    time, 0);

            mExtractor.advance();
        }

        @Override
        public void onOutputBufferAvailable(@NonNull MediaCodec mediaCodec, int index, @NonNull MediaCodec.BufferInfo bufferInfo) {
            Log.d(TAG, "onOutputBufferAvailable " + getTag() + " i=" + index + " timeMs="+bufferInfo.presentationTimeUs/1000);
            if (mIsAudio) {
                ByteBuffer buffer = mediaCodec.getOutputBuffer(index);
                if (buffer == null) {
                    Log.w(TAG, "buffer is null");
                    return;
                }
                mSync.queueAudio(buffer, index, bufferInfo.presentationTimeUs);
            }
            else {
                mediaCodec.releaseOutputBuffer(index, bufferInfo.presentationTimeUs*1000); // renderTimestampNs
            }
        }

        @Override
        public void onError(@NonNull MediaCodec mediaCodec, @NonNull MediaCodec.CodecException e) {
            Log.e(TAG, "codec: onError " + getTag());
            e.printStackTrace();
        }

        @Override
        public void onOutputFormatChanged(@NonNull MediaCodec mediaCodec, @NonNull MediaFormat mediaFormat) {
            Log.w(TAG, "codec: onOutputFormatChanged " + getTag());
        }
    }

}