// 顶点坐标
attribute vec4 aPosition;

//顶点颜色【不一定用到】
attribute vec4 aColor;
//用于传递 aColor 的变量
varying vec4 vColor;

//纹理坐标【不一定用到】
attribute vec4 aTextureCoordinate;
//用于传递 aTextureCoordinate 的变量
varying vec4 vTextureCoordinate;

void main(){
    gl_Position = aPosition;
    vColor = aColor;
    vTextureCoordinate = aTextureCoordinate;
}