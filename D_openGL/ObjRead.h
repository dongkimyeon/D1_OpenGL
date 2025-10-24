#pragma once  // 이미 있으면 유지
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE_LENGTH 256

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    unsigned int v1, v2, v3;
} Face;

typedef struct {
    //Vertex* vertices;
    size_t vertex_count;
    Face* faces;
    size_t face_count;
} Model;

// 함수 선언만 (정의는 ObjRead.cpp로 이동)
void read_newline(char* str);
void read_obj_file(const char* filename, Model* model);