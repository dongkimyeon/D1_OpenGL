#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "CommonInclude.h" // filetobuf 함수가 포함된 헤더 파일

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

// 도형 타입 정의
enum ShapeType { point, LINE, TRIANGLE, RECTANGLE };

struct Shape {
    ShapeType type;
    std::vector<GLfloat> vertices; // x, y, r, g, b, border_r, border_g, border_b
    std::vector<GLuint> indices;   // 인덱스 데이터 (사각형용)
    int vertexCount;
};

// 도형 저장 (최대 10개)
std::vector<Shape> shapes;
int selectedShapeIndex = -1; // 선택된 도형 인덱스 (-1: 선택 없음)
char currentShape = 'p'; // 기본적으로 점 그리기

int main(int argc, char** argv) {
    srand(static_cast<unsigned>(time(0))); // 랜덤 시드 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("S8");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        return -1;
    }

    MakeVertexShaders();
    MakeFragmentShaders();
    shaderProgramID = MakeShaderProgram();
    if (shaderProgramID == 0) {
        std::cerr << "셰이더 프로그램 생성 실패" << std::endl;
        return -1;
    }
    InitBuffers();

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutKeyboardFunc(Keyboard);

    glutMainLoop();
    return 0;
}

void MakeVertexShaders() {
    GLchar* vertexSource = filetobuf("vertex.glsl");
    if (!vertexSource) {
        std::cerr << "ERROR: vertex.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(vertexSource);
}

void MakeFragmentShaders() {
    GLchar* fragmentSource = filetobuf("fragment.glsl");
    if (!fragmentSource) {
        std::cerr << "ERROR: fragment.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(fragmentSource);
}

GLuint MakeShaderProgram() {
    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);

    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderID);
    return shaderID;
}

void InitBuffers() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // 위치 속성 (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // 내부 색상 속성 (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // 테두리 색상 속성 (border_r, border_g, border_b)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
}

void DrawScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO);

    for (const auto& shape : shapes) {
        if (shape.vertices.empty()) continue; // 빈 도형 스킵
        // 정점 데이터 전송
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(GLfloat), shape.vertices.data(), GL_DYNAMIC_DRAW);

        //셰이더 내에 있는 변수의 위치 가져오기
        GLint isBorderLoc = glGetUniformLocation(shaderProgramID, "isBorder");

        switch (shape.type) {
        case point:
            glPointSize(10.0f);
			glUniform1i(isBorderLoc, 0); //위치에 값 설정 (0: 내부 색상, 1: 테두리 색상)
            glDrawArrays(GL_POINTS, 0, shape.vertexCount);
            break;
        case LINE:
            glLineWidth(3.0f);
            glUniform1i(isBorderLoc, 0); 
            glDrawArrays(GL_LINES, 0, shape.vertexCount);
            break;
        case TRIANGLE:
            glUniform1i(isBorderLoc, 0); 
            glDrawArrays(GL_TRIANGLES, 0, shape.vertexCount);
            break;
        case RECTANGLE:
            // 인덱스 데이터 전송
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.indices.size() * sizeof(GLuint), shape.indices.data(), GL_DYNAMIC_DRAW);
            // 내부 채우기
            glUniform1i(isBorderLoc, 0); // 내부 색상 사용
            glDrawElements(GL_TRIANGLES, shape.indices.size(), GL_UNSIGNED_INT, 0);
            // 테두리 그리기
            glLineWidth(3.0f);
            glUniform1i(isBorderLoc, 1); // 테두리 색상 사용
            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0); // 4개의 정점으로 테두리
            break;
        }
    }

    glBindVertexArray(0);
    glutSwapBuffers();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

void Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && shapes.size() < 10) {
        float ndc_x = (2.0f * x / width) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / height);

        Shape newShape;
        if (currentShape == 'p') {
            newShape.type = point;
        }
        else if (currentShape == 'n') {
            newShape.type = LINE;
        }
        else if (currentShape == 't') {
            newShape.type = TRIANGLE;
        }
        else {
            newShape.type = RECTANGLE;
        }

        // 색상: 랜덤 (내부 색상)
        float r = (rand() % 100) / 100.0f;
        float g = (rand() % 100) / 100.0f;
        float b = (rand() % 100) / 100.0f;
        // 테두리 색상: 내부 색상과 다르게 설정 (예: 반전 색상)
        float border_r = 1.0f - r;
        float border_g = 1.0f - g;
        float border_b = 1.0f - b;

        // 도형 크기
        float size = 0.1f;
        switch (newShape.type) {
        case point:
            newShape.vertices = { ndc_x, ndc_y, r, g, b, r, g, b }; // 테두리 색상 동일
            newShape.vertexCount = 1;
            break;
        case LINE:
            newShape.vertices = { ndc_x, ndc_y, r, g, b, r, g, b,
                                 ndc_x + size, ndc_y + size, r, g, b, r, g, b };
            newShape.vertexCount = 2;
            break;
        case TRIANGLE:
            newShape.vertices = {
                ndc_x, ndc_y, r, g, b, r, g, b,
                ndc_x + size, ndc_y, r, g, b, r, g, b,
                ndc_x, ndc_y + size, r, g, b, r, g, b
            };
            newShape.vertexCount = 3;
            break;
        case RECTANGLE:
            newShape.vertices = {
                ndc_x, ndc_y, r, g, b, border_r, border_g, border_b,                    // 좌하 (0)
                ndc_x + size, ndc_y, r, g, b, border_r, border_g, border_b,             // 우하 (1)
                ndc_x + size, ndc_y + size, r, g, b, border_r, border_g, border_b,      // 우상 (2)
                ndc_x, ndc_y + size, r, g, b, border_r, border_g, border_b              // 좌상 (3)
            };
            newShape.indices = { 0, 1, 3, 1, 2, 3 }; // 두 삼각형: (0,1,3), (1,2,3)
            newShape.vertexCount = 4;
            break;
        }

        shapes.push_back(newShape);
        glutPostRedisplay();
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        float ndc_x = (2.0f * x / width) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / height);
        selectedShapeIndex = -1;
        for (size_t i = 0; i < shapes.size(); ++i) {
            auto& shape = shapes[i];
            for (size_t j = 0; j < shape.vertices.size(); j += 8) {
                float vx = shape.vertices[j];
                float vy = shape.vertices[j + 1];
                if (fabs(vx - ndc_x) < 0.05f && fabs(vy - ndc_y) < 0.05f) {
                    selectedShapeIndex = static_cast<int>(i);
                    break;
                }
            }
            if (selectedShapeIndex != -1) break;
        }
    }
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'p': // 점
    case 'n': // 선
    case 't': // 삼각형
    case 'r': // 사각형
        currentShape = key;
        break;
    case 'c': // 모든 도형 삭제
        shapes.clear();
        selectedShapeIndex = -1;
        glutPostRedisplay();
        break;
    case 'w': // 위로 이동
        if (selectedShapeIndex != -1) {
            for (size_t i = 1; i < shapes[selectedShapeIndex].vertices.size(); i += 8) {
                shapes[selectedShapeIndex].vertices[i] += 0.05f;
            }
            glutPostRedisplay();
        }
        break;
    case 's': // 아래로 이동
        if (selectedShapeIndex != -1) {
            for (size_t i = 1; i < shapes[selectedShapeIndex].vertices.size(); i += 8) {
                shapes[selectedShapeIndex].vertices[i] -= 0.05f;
            }
            glutPostRedisplay();
        }
        break;
    case 'a': // 왼쪽으로 이동
        if (selectedShapeIndex != -1) {
            for (size_t i = 0; i < shapes[selectedShapeIndex].vertices.size(); i += 8) {
                shapes[selectedShapeIndex].vertices[i] -= 0.05f;
            }
            glutPostRedisplay();
        }
        break;
    case 'd': // 오른쪽으로 이동
        if (selectedShapeIndex != -1) {
            for (size_t i = 0; i < shapes[selectedShapeIndex].vertices.size(); i += 8) {
                shapes[selectedShapeIndex].vertices[i] += 0.05f;
            }
            glutPostRedisplay();
        }
        break;
    case 'i': // 좌상 이동
        if (selectedShapeIndex != -1) {
            for (size_t i = 0; i < shapes[selectedShapeIndex].vertices.size(); i += 8) {
                shapes[selectedShapeIndex].vertices[i] -= 0.05f;
                shapes[selectedShapeIndex].vertices[i + 1] += 0.05f;
            }
            glutPostRedisplay();
        }
        break;
    case 'j': // 우상 이동
        if (selectedShapeIndex != -1) {
            for (size_t i = 0; i < shapes[selectedShapeIndex].vertices.size(); i += 8) {
                shapes[selectedShapeIndex].vertices[i] += 0.05f;
                shapes[selectedShapeIndex].vertices[i + 1] += 0.05f;
            }
            glutPostRedisplay();
        }
        break;
    case 'k': // 좌하 이동
        if (selectedShapeIndex != -1) {
            for (size_t i = 0; i < shapes[selectedShapeIndex].vertices.size(); i += 8) {
                shapes[selectedShapeIndex].vertices[i] -= 0.05f;
                shapes[selectedShapeIndex].vertices[i + 1] -= 0.05f;
            }
            glutPostRedisplay();
        }
        break;
    case 'l': // 우하 이동
        if (selectedShapeIndex != -1) {
            for (size_t i = 0; i < shapes[selectedShapeIndex].vertices.size(); i += 8) {
                shapes[selectedShapeIndex].vertices[i] += 0.05f;
                shapes[selectedShapeIndex].vertices[i + 1] -= 0.05f;
            }
            glutPostRedisplay();
        }
        break;
    }
}