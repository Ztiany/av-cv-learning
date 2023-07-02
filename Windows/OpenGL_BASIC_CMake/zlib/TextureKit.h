#ifndef OPENGL_BASIC_TEXTUREKIT_H
#define OPENGL_BASIC_TEXTUREKIT_H

#include <windows.h>
#include <gl/GL.h>

class Texture {
public:

    /**
     * OpenGL 的 GLuint 表示无符号整数类型的标识符，通常用于标识 OpenGL 对象的唯一性，如纹理、缓冲区对象、帧缓冲区对象和着色器程序对象等。
     *
     * GLuint 是 OpenGL 中定义的数据类型之一，它是一个无符号整数类型，通常是一个 32 位的整数。在 OpenGL 中使用 GLuint 来存储对象的标识符，
     * 每个对象都会被赋予一个唯一的标识符。这个标识符可以用来引用对象，并且可以用于管理和操作这些对象。
     *
     * 在使用 OpenGL 进行编程时，通常需要创建和操作许多不同类型的对象，如纹理、缓冲区、帧缓冲区等。GLuint 类型的标识符可以帮助开发者唯一地标识和
     * 管理这些对象，从而方便地进行对象的创建、修改和删除等操作。
     */
    GLuint mTextureID;

    void Init(const char *imagePath);
};


#endif //OPENGL_BASIC_TEXTUREKIT_H
