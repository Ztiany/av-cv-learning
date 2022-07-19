package me.ztiany.androidav.player.mediacodec

import android.content.Context
import android.net.Uri


internal fun usage(context: Context, source: Uri) {
    val extractor = MediaDataExtractorImpl(context)
    extractor.setTrackSelector(AUDIO_SELECTOR)
    extractor.setSource(source)
    extractor.initExtractor()

    val pcmAudioRenderer = PCMAudioDataRenderer()
    pcmAudioRenderer.initRenderer(extractor.getMediaFormat())

    val audioDataDecoder = AudioDataDecoder()
    audioDataDecoder.setMediaDataProvider(extractor)
    audioDataDecoder.setMediaDataRenderer(pcmAudioRenderer)
    audioDataDecoder.initMediaDecoder()
    audioDataDecoder.start()
}