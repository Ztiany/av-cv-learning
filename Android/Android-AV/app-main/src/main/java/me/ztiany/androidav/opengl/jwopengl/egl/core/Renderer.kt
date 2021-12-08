package me.ztiany.androidav.opengl.jwopengl.egl.core

/**All the methods in this class will be called in a OpenGL Marked Thread.*/
interface Renderer {

    /**
     * Called when the surface is created or recreated.
     */
    fun onSurfaceCreated()

    /**
     * Called when the surface changed size.
     */
    fun onSurfaceChanged(width: Int, height: Int)

    /**
     * Called to draw the current frame.
     */
    fun onDrawFrame()

    /**
     * Called when egl context release.
     */
    fun onSurfaceDestroy()

}