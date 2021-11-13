#include "Texture.h"
#include "utils.h"

unsigned char* DecodeBMP(unsigned char* bmpFileData, int& width, int& height);

void Texture::Init(const char* imagePath) {
	//load image file from disk to memory
	unsigned char* imageFileContent = LoadFileContent(imagePath);
	//decode image
	int width = 0, height = 0;
	unsigned char* pixelData = DecodeBMP(imageFileContent, width, height);
	//generate an opengl texture
	glGenTextures(1, &mTextureID);//申请一个纹理
	glBindTexture(GL_TEXTURE_2D, mTextureID);//设置当前纹理【OpenGL 是状态机，总是有当前的...状态。】
	//operation on current texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//线性过滤
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//纹理坐标超过范围[0, 1]后，怎么处理，可以设置为 GL_REPEAT/GL_CLAMP 两种模式。
	//GL_CLAMP：超过纹理的范围，比如大于 1 时，则取 1，小于 0 则取 0。
	//GL_REPEAT：超过纹理的范围，比如 1.1，则映射到 0.1。
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//生成纹理：内存-->显存
	glTexImage2D(
		GL_TEXTURE_2D,
		0, //没有 level 
		GL_RGB, //显卡上用什么格式存储纹理
		width, height, //宽高
		0, //border 是一个老的数据，一般都是 0
		GL_RGB, //数据源中的数据是什么格式的，即内存中的数据
		GL_UNSIGNED_BYTE, //RGB 每个分量用什么存储的
		pixelData //内存中的数据
	);

	glBindTexture(GL_TEXTURE_2D, 0);//取消当前纹理

	delete imageFileContent;
	//generate an opengl texture
}

unsigned char* DecodeBMP(unsigned char* bmpFileData, int& width, int& height) {
	//0x4D42 是 BMP 的魔数。
	if (0x4D42 == *((unsigned short*)bmpFileData)) {
		int pixelDataOffset = *((int*)(bmpFileData + 10));
		width = *((int*)(bmpFileData + 18));
		height = *((int*)(bmpFileData + 22));
		unsigned char* pixelData = bmpFileData + pixelDataOffset;
		//原始数据：bgr bgr bgr ....
		//目标数据：rgb rgb rgb ....
		for (int i = 0; i < width * height * 3; i += 3) {
			//交换每个颜色单位 b 和 r 的位置。
			unsigned char temp = pixelData[i];
			pixelData[i] = pixelData[i + 2];
			pixelData[i + 2] = temp;
		}
		return pixelData;
	}
	else {
		return nullptr;
	}
}