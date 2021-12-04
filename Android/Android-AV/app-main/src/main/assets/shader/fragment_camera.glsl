#ifdef GL_ES
precision mediump float;
#endif

//直接承载 YUV 的纹理
#extension GL_OES_EGL_image_external : require

varying vec2 vTextureCoordinate;

uniform samplerExternalOES uTexture;

void main(){
    vec4 rgba = texture2D(uTexture, vTextureCoordinate);
    gl_FragColor=vec4(rgba.r, rgba.g, rgba.b, rgba.a);
}