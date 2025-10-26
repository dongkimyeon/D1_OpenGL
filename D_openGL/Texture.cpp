#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION  
#include "image_loader/stb_image.h"

Texture::Texture(const char* fileLoc)
{
    textureID = 0;
    width = 0;
    height = 0;
    bitDepth = 0;

    strcpy(fileLocation, fileLoc);
}

bool Texture::LoadTexture()
{
    // 디스크에서 이미지 로드, 포맷은 unsigned char[]
    unsigned char* texData = stbi_load(fileLocation, &width, &height, &bitDepth, 0);
    if (!texData)
    {
        printf("텍스쳐 로드 실패 : %s\n", fileLocation);
        return false;  // *** 이미 있음 ***
    }
    else
    {
        printf("텍스쳐 로드 성공\n");
    }

    // VRAM 내부에 텍스쳐를 담는 공간 생성 
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 파라미터들 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // VRAM으로 텍스쳐 쏴주기 (bitDepth에 따라 포맷 동적 설정)
    if (bitDepth == 4) {  // *** 알파 채널 지원 (RGBA) ***
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    }
    else {  // RGB
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
    }

    // 밉맵 생성
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(texData);
    return true;  // *** 추가: 성공 시 return true (경고 해결) ***
}

void Texture::UseTexture()
{
    glActiveTexture(GL_TEXTURE0); //0번 텍스쳐 유닛 활성화
    glBindTexture(GL_TEXTURE_2D, textureID); // VRAM 내에 있는 이 텍스쳐를 0번 텍스쳐 유닛에 bind
}

void Texture::ClearTexture()
{
    glDeleteTextures(1, &textureID);
    textureID = 0;
    width = 0;
    height = 0;
    bitDepth = 0;

    fileLocation[0] = '\0';
}

Texture::~Texture()
{
    ClearTexture();
}