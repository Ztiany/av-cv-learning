package me.ztiany.androidav.opengl.jwopengl.recorder

import android.media.*
import android.opengl.EGLContext
import android.opengl.GLES20
import android.opengl.Matrix
import android.view.Surface
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.androidav.common.Directory
import me.ztiany.androidav.opengl.jwopengl.common.GLRenderer
import me.ztiany.androidav.opengl.jwopengl.common.RenderMode
import me.ztiany.androidav.opengl.jwopengl.common.SurfaceProvider
import me.ztiany.androidav.opengl.jwopengl.common.SurfaceProviderCallback
import me.ztiany.androidav.opengl.jwopengl.egl14.*
import me.ztiany.androidav.opengl.jwopengl.gles2.*
import timber.log.Timber
import java.io.File
import kotlin.concurrent.thread

/**渲染的是 FBO 中的纹理，使用标准的坐标系*/
class RecorderEncodeRenderer(private val context: AppCompatActivity) : GLRenderer {

    private lateinit var glProgram: GLProgram

    private val glMVPMatrix = GLMVPMatrix()

    /**矩形的坐标*/
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /**纹理坐标*/
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateStandard())

    private var eglEnvironment: EGLEnvironment? = null
    private var mediaCodec: MediaCodec? = null
    private var mediaMuxer: MediaMuxer? = null
    private var mediaCodecSurfaceProvider: MediaCodecSurfaceProvider? = null

    private var lastTimeStamp: Long = 0
    private var track = 0
    private val speed = 1F

    override fun onSurfaceCreated() {
        glProgram = GLProgram.fromAssets(
            "shader/vertex_mvp.glsl",
            "shader/fragment_texture.glsl"
        )

        //vertex
        glProgram.activeAttribute("aPosition")
        glProgram.activeAttribute("aTextureCoordinate")
        glProgram.activeUniform("uMVPModelMatrix")

        //fragment
        glProgram.activeUniform("uTexture")
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
    }

    override fun onDrawFrame(attachment: Any?) {
        val textureWithTime = attachment as? TextureWithTime ?: return
        glProgram.startDraw {
            clearColorBuffer()
            //vertex
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            uniformMatrix4fv("uMVPModelMatrix", glMVPMatrix.mvpMatrix)
            //fragment
            //texture【将分享过来的纹理绘制到 FBO 的纹理上】
            textureWithTime.glTexture.activeTexture(uniformHandle("uTexture"))
            //draw
            drawArraysStrip(4/*4 个顶点*/)
            //setTime
            eglEnvironment?.setPresentationTime(textureWithTime.timestamp)
        }
    }

    override fun onSurfaceDestroy() {

    }

    fun setVideoAttribute(attribute: TextureAttribute) {
        if ((attribute.orientation / 90).mod(2) == 1) {//竖屏
            glMVPMatrix.setWorldSize(attribute.height, attribute.width)
            glMVPMatrix.setModelSize(attribute.height, attribute.width)
        } else {//横屏
            glMVPMatrix.setWorldSize(attribute.height, attribute.width)
            glMVPMatrix.setModelSize(attribute.width, attribute.height)
        }

        glMVPMatrix.resetToIdentity(glMVPMatrix.mvpMatrix)
        glMVPMatrix.lookAtNormally()
        glMVPMatrix.adjustToOrthogonal()
        glMVPMatrix.combineMVP()
        //绕着 Z 轴旋转
        if (!attribute.isFront) {
            //后摄，一般情况下相机的画面被逆时针转了 90 度，这是这里也将顶点坐标转同样的角度。
            //注意【顶点是先插值，然后我们利用矩阵再将顶点修正到正确的采样进行位置】。
            Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, -attribute.orientation.toFloat(), 0F, 0F, 1F)
        } else {
            Matrix.scaleM(glMVPMatrix.mvpMatrix, 0, -1F, 1F, 1F)
            Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, attribute.orientation.toFloat(), 0F, 0F, 1F)
        }
    }


    fun start(sharedEGLContext: EGLContext) {
        endOfStream = false
        val surface = initMediaCodec()
        val mediaCodecSurfaceProvider = MediaCodecSurfaceProvider(surface)
        this.mediaCodecSurfaceProvider = mediaCodecSurfaceProvider

        thread {
            codec()
        }

        eglEnvironment = EGLEnvironment(mediaCodecSurfaceProvider, EGLAttribute(sharedEGLContext)).apply {
            renderMode = RenderMode.WhenDirty
            start(this@RecorderEncodeRenderer)
        }
    }

    private fun initMediaCodec(): Surface {
        val format = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, glMVPMatrix.getModelWidth(), glMVPMatrix.getModelHeight())
        //从 surface 当中获得
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface)
        //码率
        format.setInteger(MediaFormat.KEY_BIT_RATE, glMVPMatrix.getModelWidth() * glMVPMatrix.getModelHeight() * 3)
        //帧率
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 25)
        //关键帧间隔
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 10)
        //创建编码器
        val mediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC)
        //配置编码器
        mediaCodec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE)
        //输入数据
        val surface = mediaCodec.createInputSurface()
        this.mediaCodec = mediaCodec
        //开始编码
        mediaCodec.start()

        //混合器 (复用器) 将编码的 h.264 封装为 mp4
        val output = File(Directory.getSDCardRootPath(), Directory.createTempFileName(Directory.VIDEO_FORMAT_MP4)).absolutePath
        Timber.d("output = $output")
        mediaMuxer = MediaMuxer(output, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4)
        return surface
    }

    fun stop() {
        endOfStream = true
        eglEnvironment?.release()
    }

    fun onFrame(glTexture: GLTexture, timestamp: Long) {
        eglEnvironment?.requestRender(TextureWithTime(glTexture, timestamp))
    }

    private inner class MediaCodecSurfaceProvider(private val surface: Surface) : SurfaceProvider {

        override fun start(surfaceProviderCallback: SurfaceProviderCallback) {
            surfaceProviderCallback.onSurfaceAvailable(surface)
            surfaceProviderCallback.onSurfaceChanged(surface, glMVPMatrix.getModelWidth(), glMVPMatrix.getModelHeight())
        }

        override fun stop() = Unit
    }

    class TextureWithTime(
        val glTexture: GLTexture,
        val timestamp: Long
    )

    @Volatile var endOfStream = false

    private fun codec() {
        val mediaCodec = this.mediaCodec ?: return
        val mediaMuxer = this.mediaMuxer ?: return

        while (true) {
            if (endOfStream) {
                mediaCodec.signalEndOfInputStream()
                mediaCodec.stop()
                mediaCodec.release()
                mediaMuxer.stop()
                mediaMuxer.release()
                Timber.d("stop recording 1.")
            }

            val bufferInfo = MediaCodec.BufferInfo()
            val index: Int = mediaCodec.dequeueOutputBuffer(bufferInfo, 10000)
            if (index == MediaCodec.INFO_TRY_AGAIN_LATER) {
                if (!endOfStream) {
                    continue
                } else {
                    mediaCodec.signalEndOfInputStream()
                    mediaCodec.stop()
                    mediaCodec.release()
                    mediaMuxer.stop()
                    mediaMuxer.release()
                    Timber.d("stop recording 2.")
                    break
                }
            } else if (index == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                //输出格式发生改变 ，第一次总会调用所以在这里开启混合器
                val newFormat: MediaFormat = mediaCodec.outputFormat
                track = mediaMuxer.addTrack(newFormat)
                mediaMuxer.start()
            } else if (index == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                //可以忽略
            } else {
                //调整时间戳
                bufferInfo.presentationTimeUs = (bufferInfo.presentationTimeUs / speed).toLong()
                //有时候会出现异常 ： timestampUs xxx < lastTimestampUs yyy for Video track
                if (bufferInfo.presentationTimeUs <= lastTimeStamp) {
                    bufferInfo.presentationTimeUs = (lastTimeStamp + 1000000 / 25 / speed).toLong()
                }
                lastTimeStamp = bufferInfo.presentationTimeUs

                //正常则 index 获得缓冲区下标
                val encodedData = mediaCodec.getOutputBuffer(index)!!
                //如果当前的 buffer是配置信息，不管它
                if (bufferInfo.flags and MediaCodec.BUFFER_FLAG_CODEC_CONFIG != 0) {
                    bufferInfo.size = 0
                }
                if (bufferInfo.size != 0) {
                    //设置从哪里开始读数据(读出来就是编码后的数据)
                    encodedData.position(bufferInfo.offset)
                    //设置能读数据的总长度
                    encodedData.limit(bufferInfo.offset + bufferInfo.size)
                    //写出为mp4
                    mediaMuxer.writeSampleData(track, encodedData, bufferInfo)
                }
                // 释放这个缓冲区，后续可以存放新的编码后的数据啦
                mediaCodec.releaseOutputBuffer(index, false)
                // 如果给了结束信号 signalEndOfInputStream
                if (bufferInfo.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM != 0) {
                    break
                }
            }
        }
    }

}