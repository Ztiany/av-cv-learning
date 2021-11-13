package me.ztiany.androidav.opengl.nwglsurv;

public class OGLESInterface {

    public native void Init();

    public native void OnViewportChanged(int width,int height);

    public native void Render();

}
