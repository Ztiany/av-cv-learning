package me.ztiany.androidav.player.mediacodec

import android.media.MediaCodec
import android.media.MediaCodecList
import android.media.MediaFormat
import timber.log.Timber
import java.nio.ByteBuffer

class VideoDataDecoder(
    stateHolder: CodecPlayerStateHolder,
    mediaFormat: MediaFormat,
    provider: MediaDataProvider,
    renderer: MediaDataRenderer
) : BaseDataDecoder(stateHolder, mediaFormat, provider, renderer, syncToPts = true) {

    private val isSurfaceMode = renderer is SurfaceRenderer

    override fun initDecoder(mediaFormat: MediaFormat, renderer: MediaDataRenderer): MediaCodec {
        val name = MediaCodecList(MediaCodecList.ALL_CODECS).findDecoderForFormat(mediaFormat)
        Timber.d("initDecoder.findDecoderForFormat: $name")
        return MediaCodec.createByCodecName(name).apply {
            if (renderer is SurfaceRenderer) {
                Timber.d("config surface mode")
                configure(mediaFormat, renderer.provideSurface(), null, 0)
                Timber.d("config non-surface mode")
            } else {
                configure(mediaFormat, null, null, 0)
            }
            Timber.d("initDecoder success")
        }
    }

    override fun deliverFrame(decoder: MediaCodec, renderer: MediaDataRenderer, data: ByteBuffer, outputBufferInfo: MediaCodec.BufferInfo, index: Int) {
        renderer.render(data, outputBufferInfo)
        decoder.releaseOutputBuffer(index, isSurfaceMode)
    }

}