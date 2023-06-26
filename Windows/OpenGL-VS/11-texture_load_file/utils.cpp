#include "utils.h"

unsigned char* LoadFileContent(const char* filePath) {
	unsigned char* fileContent = nullptr;
	//项目属性-->C/C++-->命令行：添加 //D_CRT_SECURE_NO_WARNINGS 内容。
	FILE* pFile = fopen(filePath, "rb");

	if (pFile) {
		//read
		fseek(pFile, 0, SEEK_END);
		int nLen = ftell(pFile);
		if (nLen > 0) {
			rewind(pFile);
			fileContent = new unsigned char[nLen + 1];
			fread(fileContent, sizeof(unsigned char), nLen, pFile);
			fileContent[nLen] = '\0';
		}
		fclose(pFile);
	}

	return fileContent;
}