#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "CommonInclude.h"
#include <string> // 추가: string 사용을 위해
#include <cmath> // 추가: sqrtf 사용을 위해
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint cubeVAO, cubeVBO, cubeEBO;
GLuint pyramidVAO, pyramidVBO, pyramidEBO;
GLuint texIDs[6]; // 추가: 텍스처 ID 배열
struct Texture
{
    int widthImage, heightImage, numberOfChannel;
    unsigned char* data = nullptr;
};
Texture cubeTexture[6];
glm::vec3 cameraPos = glm::vec3(3.0f, 3.0f, 3.0f);
glm::vec3 lightPos = glm::vec3(2.0f, 2.0f, 2.0f);
float rotX = 0.0f;
float rotY = 0.0f;
int currentShape = 0; // 0: cube, 1: pyramid
void MakeVertexShaders() {
    GLchar* vertexSource = filetobuf("lightVertex.glsl");
    if (!vertexSource) { std::cerr << "ERROR: cannot read vertex shader" << std::endl; return; }
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint result; GLchar log[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) { glGetShaderInfoLog(vertexShader, 512, NULL, log); std::cerr << log << std::endl; }
    free(vertexSource);
}
void MakeFragmentShaders() {
    GLchar* fragmentSource = filetobuf("lightFragment.glsl");
    if (!fragmentSource) { std::cerr << "ERROR: cannot read fragment shader" << std::endl; return; }
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    GLint result; GLchar log[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) { glGetShaderInfoLog(fragmentShader, 512, NULL, log); std::cerr << log << std::endl; }
    free(fragmentSource);
}
GLuint MakeShaderProgram() {
    GLuint id = glCreateProgram();
    glAttachShader(id, vertexShader);
    glAttachShader(id, fragmentShader);
    glLinkProgram(id);
    GLint res; GLchar log[512];
    glGetProgramiv(id, GL_LINK_STATUS, &res);
    if (!res) { glGetProgramInfoLog(id, 512, NULL, log); std::cerr << "Program link error:\n" << log << std::endl; return 0; }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(id);
    return id;
}
void InitCubeBuffers() {
    // Cube data: positions, normals, and texcoords
    GLfloat vertices[] = {
        // Back face (z=-0.5): indices 0-3, UV: (0,0),(1,0),(1,1),(0,1)
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
         0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        // Front face (z=0.5): indices 4-7, UV: (0,0),(1,0),(1,1),(0,1)
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        // Left face (x=-0.5): indices 8-11, UV: (1,1),(0,1),(0,0),(1,0) [z/y 기반]
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        // Right face (x=0.5): indices 12-15, UV: (1,1),(0,1),(0,0),(1,0) [z/y 기반]
         0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         // Bottom face (y=-0.5): indices 16-19, UV: (0,0),(1,0),(1,1),(0,1) [x/z 기반]
         -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
          0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
          0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
         -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
         // Top face (y=0.5): indices 20-23, UV: (0,0),(1,0),(1,1),(0,1) [x/z 기반]
         -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
          0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
          0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
         -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };
    GLuint indices[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // 수정: stride 8 * sizeof(GLfloat), UV 속성 추가
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}
void InitPyramidBuffers() {
    float invSqrt5 = 1.0f / sqrtf(5.0f);
    float ny = invSqrt5;
    float nx = 2.0f * invSqrt5;
    GLfloat vertices[] = {
        // Base face (bottom)
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  // 0: base0
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  // 1: base1
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  // 2: base2
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,  // 3: base3
        // Front face (-z)
         0.0f,  0.5f,  0.0f,  0.0f,  ny, -nx,  0.5f, 1.0f,  // 4: apex
        -0.5f, -0.5f, -0.5f,  0.0f,  ny, -nx,  0.0f, 0.0f,  // 5: base0
         0.5f, -0.5f, -0.5f,  0.0f,  ny, -nx,  1.0f, 0.0f,  // 6: base1
         // Right face (+x)
          0.0f,  0.5f,  0.0f,  nx,  ny,  0.0f,  0.5f, 1.0f,  // 7: apex
          0.5f, -0.5f,  0.5f,  nx,  ny,  0.0f,  1.0f, 0.0f,  // 8: base2
          0.5f, -0.5f, -0.5f,  nx,  ny,  0.0f,  0.0f, 0.0f,  // 9: base1
          // Back face (+z)
           0.0f,  0.5f,  0.0f,  0.0f,  ny,  nx,  0.5f, 1.0f,  // 10: apex
          -0.5f, -0.5f,  0.5f,  0.0f,  ny,  nx,  0.0f, 0.0f,  // 11: base3
           0.5f, -0.5f,  0.5f,  0.0f,  ny,  nx,  1.0f, 0.0f,  // 12: base2
           // Left face (-x)
            0.0f,  0.5f,  0.0f, -nx,  ny,  0.0f,  0.5f, 1.0f,  // 13: apex
           -0.5f, -0.5f, -0.5f, -nx,  ny,  0.0f,  0.0f, 0.0f,  // 14: base0
           -0.5f, -0.5f,  0.5f, -nx,  ny,  0.0f,  1.0f, 0.0f   // 15: base3
    };
    GLuint indices[] = {
        // Base
        0, 1, 2, 2, 3, 0,
        // Front: apex, base1, base0
        4, 6, 5,
        // Right: apex, base2, base1
        7, 8, 9,
        // Back: apex, base3, base2
        10, 11, 12,
        // Left: apex, base0, base3
        13, 14, 15
    };
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glGenBuffers(1, &pyramidEBO);
    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}
void DrawScene() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
    GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
    GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
    // objectColorLoc 제거
    GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");
    GLint texLoc = glGetUniformLocation(shaderProgramID, "texture1"); // 추가: 텍스처 유니폼
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
    // glUniform3f(objectColorLoc, 0.8f, 0.5f, 0.2f); 제거
    glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
    // 수정: shape에 따라 VAO와 그리기 방식 변경
    glBindVertexArray(currentShape == 0 ? cubeVAO : pyramidVAO);
    if (currentShape == 0) {
        // Cube: 6 faces, each 6 indices
        for (int face = 0; face < 6; ++face) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texIDs[face]);
            glUniform1i(texLoc, 0);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(face * 6 * sizeof(GLuint)));
        }
    }
    else {
        // Pyramid: 5 faces, base 6 indices, sides 3 each
        for (int face = 0; face < 5; ++face) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texIDs[face]);
            glUniform1i(texLoc, 0);
            int count = (face == 0) ? 6 : 3;
            size_t indexOffset = (face == 0) ? 0 : (6 + (face - 1) * 3);
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(indexOffset * sizeof(GLuint)));
        }
    }
    glBindVertexArray(0);
    glutSwapBuffers();
}
void Reshape(int w, int h) { glViewport(0, 0, w, h); width = w; height = h; }
void Keyboard(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') exit(0);
    if (key == 'c' || key == 'C') { currentShape = 0; glutPostRedisplay(); }
    if (key == 'p' || key == 'P') { currentShape = 1; glutPostRedisplay(); }
    if (key == 'x') { rotX += 5.0f; glutPostRedisplay(); }
    if (key == 'X') { rotX -= 5.0f; glutPostRedisplay(); }
    if (key == 'y') { rotY += 5.0f; glutPostRedisplay(); }
    if (key == 'Y') { rotY -= 5.0f; glutPostRedisplay(); }
    if (key == 's' || key == 'S') { rotX = 0.0f; rotY = 0.0f; glutPostRedisplay(); }
}
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("S29");
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "GLEW init failed" << std::endl; return -1; }

    stbi_set_flip_vertically_on_load(true);
    MakeVertexShaders();
    MakeFragmentShaders();
    shaderProgramID = MakeShaderProgram();
    if (shaderProgramID == 0) { std::cerr << "Shader program failed" << std::endl; return -1; }
    InitCubeBuffers();
    InitPyramidBuffers();
    const char* textureNames[6] = { "a", "b", "c", "d", "e", "f" };
    for (int i = 0; i < 6; ++i)
    {
        std::string textureFile = "Textures/" + std::string(textureNames[i]) + ".png";
        cubeTexture[i].data = stbi_load(textureFile.c_str(), &cubeTexture[i].widthImage, &cubeTexture[i].heightImage, &cubeTexture[i].numberOfChannel, 0);
        if (!cubeTexture[i].data)
        {
            std::cerr << "Failed to load texture: " << textureFile << std::endl;
        }
    }
  
    glGenTextures(6, texIDs);
    for (int i = 0; i < 6; ++i) {
        glBindTexture(GL_TEXTURE_2D, texIDs[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        int channels = cubeTexture[i].numberOfChannel;
        GLenum internalFormat = (channels == 3) ? GL_RGB : GL_RGBA;
        GLenum format = internalFormat;
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, cubeTexture[i].widthImage, cubeTexture[i].heightImage, 0, format, GL_UNSIGNED_BYTE, cubeTexture[i].data);
        stbi_image_free(cubeTexture[i].data);
        cubeTexture[i].data = nullptr;
    }
    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMainLoop();
    return 0;
}