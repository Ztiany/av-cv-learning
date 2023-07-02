#include "TextureKit.h"
#include <IOKit.h>

unsigned char *DecodeBMP(unsigned char *bmpFileData, int &width, int &height) {
    if (0x4D42/*BMP MagicCode*/ == *((unsigned short *) bmpFileData)) {
        int pixelDataOffset = *((int *) (bmpFileData + 10/*Header*/));
        width = *((int *) (bmpFileData + 18));
        height = *((int *) (bmpFileData + 22));
        unsigned char *pixelData = bmpFileData + pixelDataOffset;
        // 转换：bgr 布局 --> rgb 布局
        for (int i = 0; i < width * height * 3; i += 3) {
            unsigned char temp = pixelData[i];
            pixelData[i] = pixelData[i + 2];
            pixelData[i + 2] = temp;
        }
        return pixelData;
    } else {
        return nullptr;
    }
}

void Texture::Init(const char *imagePath) {
    // load image file from disk to memory
    unsigned char *imageFileContent = LoadFileContent(imagePath);
    // decode image
    int width = 0, height = 0;
    unsigned char *pixelData = DecodeBMP(imageFileContent, width, height);

    /*
     * 在 OpenGL 中，glGenTextures 函数用于生成指定数量的纹理对象，并将它们的标识符存储在指定的 GLuint 数组中，以便在后续的纹理操作中使用。
     *
     *  - n 表示要生成的纹理对象数量;
     *  - textures 是一个 GLuint 类型的数组，用于存储生成的纹理对象的标识符。
     *
     * 调用 glGenTextures 函数后，OpenGL 会生成 n 个纹理对象，并将它们的标识符存储在 textures 数组中。这些标识符是无符号整数类型的，可以用于后续的纹理操作，如绑定纹理、设置纹理参数、加载纹理数据等。
     */
    glGenTextures(1, &mTextureID);

    /*
     * 在 OpenGL 中，glBindTexture 函数用于将指定的纹理对象绑定到当前的纹理单元上，以便后续的纹理操作能够作用于该纹理对象。
     *
     *  - target 表示纹理对象的类型，如 GL_TEXTURE_2D 表示二维纹理，GL_TEXTURE_CUBE_MAP 表示立方体贴图等。
     *  - texture 表示要绑定的纹理对象的标识符。
     *
     * 调用 glBindTexture 函数后，OpenGL 会将指定的纹理对象绑定到当前的纹理单元上。纹理单元是一个抽象的概念，用于表示当前正在被渲染器使用的纹理对象。
     */
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    /*
     * 使用 OpenGL 中的纹理参数函数 glTexParameteri，用于设置当前绑定的 2D 纹理对象的各种参数。具体来说，这里设置了四个纹理参数：
     *
     * - GL_TEXTURE_MAG_FILTER：用于设置纹理放大过滤方式。这里设置为 GL_LINEAR，表示使用双线性过滤来获得更加平滑的纹理放大效果。
     * - GL_TEXTURE_MIN_FILTER：用于设置纹理缩小过滤方式。这里同样设置为 GL_LINEAR，表示使用双线性过滤来获得更加平滑的纹理缩小效果。
     * - GL_TEXTURE_WRAP_S：用于设置纹理在 S 方向上的包裹方式。这里设置为 GL_REPEAT，表示通过重复纹理图像来填充纹理坐标超出 [0, 1] 范围的部分。
     * - GL_TEXTURE_WRAP_T：用于设置纹理在 T 方向上的包裹方式。这里同样设置为 GL_REPEAT。
     *
     * 这些纹理参数可以影响纹理的渲染效果，例如，通过设置放大和缩小过滤方式来控制纹理在不同距离处的细节程度，或者通过设置包裹方式来控制纹理坐标超出范围的行为。
     *
     * 需要注意的是，这些纹理参数只对当前绑定的纹理对象有效，如果需要对不同的纹理对象设置不同的纹理参数，需要先绑定对应的纹理对象再进行设置。
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /*
     * glTexImage2D 函数用于将纹理图像的数据加载到当前绑定的 2D 纹理对象中（转换到 GPU 中），以便后续使用。
     *
     * 其中，各个参数的含义如下：
     *
     *    -  target：纹理类型，这里应该为 GL_TEXTURE_2D。
     *    -  level：指定要加载的纹理图像的级别。0 表示最基本的级别，随着级别的逐渐增加，纹理图像的尺寸会等比例缩小。
     *    -  internalFormat：指定纹理内部的存储格式，如 GL_RGB、GL_RGBA 等。
     *    -  width/height：指定纹理图像的宽度和高度，以像素为单位。
     *    -  border：指定边框的宽度，一般应该设置为 0。
     *    -  format：指定纹理图像的格式，如 GL_RGB、GL_RGBA 等（即 pixelData 数据的格式）。
     *    -  type：指定纹理图像数据的类型，如 GL_UNSIGNED_BYTE、GL_FLOAT 等。
     *    -  data：指向纹理图像数据的指针。
     *
     */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);

    // 取消绑定
    glBindTexture(GL_TEXTURE_2D, 0);

    delete imageFileContent;
}