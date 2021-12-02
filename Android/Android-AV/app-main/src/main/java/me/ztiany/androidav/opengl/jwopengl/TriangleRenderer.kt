package me.ztiany.androidav.opengl.jwopengl

import android.opengl.GLES20
import android.opengl.GLSurfaceView
import me.ztiany.androidav.common.FileUtils
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class TriangleRenderer : GLSurfaceView.Renderer {

    private var program = 0
    private var vPositionHandle = 0
    private var colorHandle = 0

    private lateinit var vertexBuffer: FloatBuffer

    private val triangleCoordination = floatArrayOf(
        0.0F, 0.5F, 0.0F,  // top
        -0.5F, -0.5F, 0.0F,  // bottom left
        0.5F, -0.5F, 0.0F // bottom right
    )

    //设置颜色，依次为红绿蓝和透明通道
    private val color = floatArrayOf(1.0f, 0f, 0f, 1.0f)

    private fun loadShader(type: Int, shaderCode: String): Int {
        val shader = GLES20.glCreateShader(type)
        GLES20.glShaderSource(shader, shaderCode)
        GLES20.glCompileShader(shader)
        return shader
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        //申请 native 空间
        vertexBuffer = ByteBuffer.allocateDirect(triangleCoordination.size * 4 /*one float has four bytes.*/).run {
            order(ByteOrder.nativeOrder())
            asFloatBuffer()
        }

        //将坐标数据转换为 FloatBuffer
        vertexBuffer.put(triangleCoordination)
        vertexBuffer.position(0)

        //生成 Shader
        val vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, FileUtils.loadAssets("shader/base_vert.glsl"))
        val fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, FileUtils.loadAssets("shader/color_fragment.glsl"))

        //创建一个空的OpenGLES程序
        program = GLES20.glCreateProgram()
        //将顶点着色器加入到程序
        GLES20.glAttachShader(program, vertexShader)
        //将片元着色器加入到程序中
        GLES20.glAttachShader(program, fragmentShader)
        //连接到着色器程序
        GLES20.glLinkProgram(program)

        //获取顶点着色器的vPosition成员句柄
        vPositionHandle = GLES20.glGetAttribLocation(program, "vPosition")
        //获取片元着色器的vColor成员的句柄
        colorHandle = GLES20.glGetUniformLocation(program, "vColor")
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
    }

    override fun onDrawFrame(gl: GL10?) {
        //将程序加入到OpenGLES2.0环境
        GLES20.glUseProgram(program)
        //启用三角形顶点的句柄
        GLES20.glEnableVertexAttribArray(vPositionHandle)

        //准备三角形的坐标数据：CPU -> GPU
        GLES20.glVertexAttribPointer(vPositionHandle, 3, GLES20.GL_FLOAT, false, 12/*3 * 4*/, vertexBuffer)

        //设置绘制三角形的颜色
        GLES20.glUniform4fv(colorHandle, 1/*4fv，所以是 1*/, color, 0)
        //绘制三角形
        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 3/*3 个点*/)

        //禁止顶点数组的句柄
        GLES20.glDisableVertexAttribArray(program)
        GLES20.glUseProgram(0);
    }

}