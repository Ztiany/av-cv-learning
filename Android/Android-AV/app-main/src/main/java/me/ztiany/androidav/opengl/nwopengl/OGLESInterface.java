package me.ztiany.androidav.opengl.nwopengl;

public class OGLESInterface {

    public native void Init();

    public native void OnViewportChanged(int width,int height);

    public native void Render();

}
