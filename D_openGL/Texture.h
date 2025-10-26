#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include "image_loader/stb_image.h"
#include "image_loader/SOIL.h"

class Texture
{
public:
	Texture(const char* fileLoc);

	bool LoadTexture(); // 디스크에 저장된 이미지를 메모리에 텍스쳐로 로드
	void UseTexture(); // 렌더링할 때 이 텍스쳐를 쓰겠다고 설정
	void ClearTexture(); // 메모리에서 텍스쳐 내리기

	~Texture();

private:
	GLuint textureID;
	int width, height, bitDepth;

	char fileLocation[128]; // 파일 경로
};