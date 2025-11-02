#include "ObjRead.h"

void read_newline(char* str) {
    char* pos;
    if ((pos = strchr(str, '\n')) != NULL)
        *pos = '\0';
}

void read_obj_file(const char* filename, Model* model) {
    FILE* file;
    errno_t err = fopen_s(&file, filename, "r");
    if (err != 0 || !file) {
        perror("Error opening file");
        model->vertex_count = 0;
        model->face_count = 0;
        return;
    }
    char line[MAX_LINE_LENGTH];
    model->vertex_count = 0;
    model->face_count = 0;
    while (fgets(line, sizeof(line), file)) {
        read_newline(line);
        if (line[0] == 'v' && line[1] == ' ')
            model->vertex_count++;
        else if (line[0] == 'f' && line[1] == ' ')
            model->face_count++;
    }
    fseek(file, 0, SEEK_SET);
    model->vertices = (Vertex*)malloc(model->vertex_count * sizeof(Vertex));
    if (!model->vertices) {
        fclose(file);
        return;
    }
    model->faces = (Face*)malloc(model->face_count * sizeof(Face));
    if (!model->faces) {
        free(model->vertices);
        fclose(file);
        return;
    }
    size_t vertex_index = 0; size_t face_index = 0;
    while (fgets(line, sizeof(line), file)) {
        read_newline(line);
        if (line[0] == 'v' && line[1] == ' ') {
            int result = sscanf_s(line + 2, "%f %f %f", &model->vertices[vertex_index].x,
                &model->vertices[vertex_index].y,
                &model->vertices[vertex_index].z);
            if (result == 3) {
                vertex_index++;
            }
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            unsigned int v1, v2, v3;
            // //n 형식 지원: %u//%*u (숫자//숫자 스킵)
            int result = sscanf_s(line + 2, "%u//%*u %u//%*u %u//%*u", &v1, &v2, &v3);
            if (result == 3) {
                model->faces[face_index].v1 = v1 - 1; // OBJ indices start at 1
                model->faces[face_index].v2 = v2 - 1;
                model->faces[face_index].v3 = v3 - 1;
                face_index++;
            }
            else {
                // 대안: 단순 %u %u %u 시도 (no /)
                result = sscanf_s(line + 2, "%u %u %u", &v1, &v2, &v3);
                if (result == 3) {
                    model->faces[face_index].v1 = v1 - 1;
                    model->faces[face_index].v2 = v2 - 1;
                    model->faces[face_index].v3 = v3 - 1;
                    face_index++;
                }
            }
        }
    }
    // 실제 카운트 업데이트 (파싱 실패 시)
    model->vertex_count = vertex_index;
    model->face_count = face_index;
    fclose(file);
}