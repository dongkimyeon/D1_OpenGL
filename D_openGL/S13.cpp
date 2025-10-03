#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h"
#define M_PI 3.14159265358979323846
#include <algorithm> // std::clamp
#include <vector>
#include <iostream>
#include <cmath> // sqrt, cos, sin

// 도형 타입 정의
enum ShapeType { Point, Line, Triangle, Rect, Pentagon };

enum State {
    ONE, TWO, NONE
    // ONE: 튕기기
    // TWO: 지그재그 이동
};

enum MoveType {
    LEFT, RIGHT, UP, DOWN,
    LEFTUP, LEFTDOWN, RIGHTUP, RIGHTDOWN,
    None
};

struct Shape {
    ShapeType type;
    float centerX, centerY; // 도형의 중심 좌표
    float r, g, b; // 색상
    std::vector<GLfloat> vertices; // x, y, r, g, b
    std::vector<GLfloat> offsets; // 중심 기준 오프셋 (x, y per vertex)
    State state = NONE;
    MoveType moveType = None;
    int Speed = 0;
    bool reverseflag = false; // 수직 이동 방향 플래그
    float moveDistance = 0.0f; // 이동 거리
};

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void Motion(int x, int y);
void UpdateMovement();
void Timer(int value);
void InitShapes();
void RecreateShape(Shape& shape);
// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;



std::vector<Shape> shapes;
bool isDragging = false;
int selectedShapeIndex = -1;
bool animationRunning = true;

int main(int argc, char** argv) {
    srand(static_cast<unsigned>(time(0))); // 랜덤 시드 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Prac");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        return -1;
    }
    InitShapes();

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
    glutMotionFunc(Motion);
    glutMouseFunc(Mouse);

    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, Timer, 0); // 약 60 FPS
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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);  // EBO 바인딩 추가
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // 실제 데이터 크기: 2(pos) + 3(color) = 5 floats
    const GLsizei stride = 5 * sizeof(GLfloat);
    // 위치 속성 (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 속성 (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(GLfloat)));  // 오프셋: 위치 다음
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);  // 언바인드 추가
}

void DrawScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO);

    for (const auto& shape : shapes) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(GLfloat), shape.vertices.data(), GL_DYNAMIC_DRAW);
        int numVertices = shape.vertices.size() / 5;
        switch (shape.type) {
        case Point:
            glPointSize(10.0f);
            glDrawArrays(GL_POINTS, 0, 1);
            break;
        case Line:
            glLineWidth(4.0f);
            glDrawArrays(GL_LINES, 0, numVertices);
            break;
        case Triangle:
            glDrawArrays(GL_TRIANGLES, 0, numVertices);
            break;
        case Rect:
            glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
            break;
        case Pentagon:
            glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
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

void Mouse(int button, int state, int x, int y)
{

    float ndc_x = (2.0f * x / glutGet(GLUT_WINDOW_WIDTH)) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / glutGet(GLUT_WINDOW_HEIGHT));

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        for (auto& shape : shapes)
        {
            float dx = ndc_x - shape.centerX;
            float dy = ndc_y - shape.centerY;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance < 0.1f) // 클릭한 위치가 도형 중심에서 가까운 경우
            {
                selectedShapeIndex = &shape - &shapes[0]; // 선택된 도형의 인덱스 저장
				shape.moveType = None; // 이동 타입 초기화
                isDragging = true;
                break;
            }
        }
    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        isDragging = false;

        //도형끼리 충돌 처리
        if (selectedShapeIndex != -1)
        {
            Shape& movedShape = shapes[selectedShapeIndex];
            bool collided = false;
            for (size_t i = 0; i < shapes.size(); ++i)
            {
                if (i == static_cast<size_t>(selectedShapeIndex)) continue; // 자기 자신은 건너뜀
                Shape& otherShape = shapes[i];
                float dx = movedShape.centerX - otherShape.centerX;
                float dy = movedShape.centerY - otherShape.centerY;
                float distance = sqrt(dx * dx + dy * dy);
                if (distance < 0.1f) {

                    otherShape.type = static_cast<ShapeType>((otherShape.type + movedShape.vertices.size() / 5) % 5); // 도형 타입 변경
                    RecreateShape(otherShape); // 새 타입으로 vertices와 offsets 재생성

                    // 랜덤 상태 설정 (튕기기 또는 지그재그)
                    int randState = rand() % 2;
                    if (randState == 0) {
                        otherShape.state = ONE;
                        otherShape.moveType = static_cast<MoveType>(4 + (rand() % 4)); // LEFTUP 등
                    }
                    else {
                        otherShape.state = TWO;
                        otherShape.moveType = static_cast<MoveType>(rand() % 2); // LEFT or RIGHT
                    }
                    otherShape.Speed = 10 + (rand() % 20);
                    otherShape.reverseflag = false;
                    otherShape.moveDistance = 0.0f;
                    collided = true;
                    break;
                }
            }
            if (collided) {
                shapes.erase(shapes.begin() + selectedShapeIndex);
                selectedShapeIndex = -1;
            }
        }
    }
}

void Motion(int x, int y)
{
    float ndc_x = (2.0f * x / width) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / height);
    if (isDragging && selectedShapeIndex != -1)
    {

        Shape& shape = shapes[selectedShapeIndex];
        float dx = ndc_x - shape.centerX;
        float dy = ndc_y - shape.centerY;
        shape.centerX = ndc_x;
        shape.centerY = ndc_y;
        // 오프셋 사용해 vertices 업데이트
        int numVertices = shape.vertices.size() / 5;
        for (size_t i = 0; i < numVertices; ++i) {
            shape.vertices[i * 5] += dx;     // x 좌표 업데이트
            shape.vertices[i * 5 + 1] += dy; // y 좌표 업데이트
        }
        glutPostRedisplay();
    }
}

void RecreateShape(Shape& shape) {
    shape.vertices.clear();
    shape.offsets.clear();
    switch (shape.type) {
    case Point: {
        shape.vertices = { shape.centerX, shape.centerY, shape.r, shape.g, shape.b };
        shape.offsets = { 0.0f, 0.0f };
        break;
    }
    case Line: {
        float offsetX = 0.1f;
        float offsetY = 0.1f;
        shape.vertices = { shape.centerX - offsetX, shape.centerY - offsetY, shape.r, shape.g, shape.b,
                           shape.centerX + offsetX, shape.centerY + offsetY, shape.r, shape.g, shape.b };
        shape.offsets = { -offsetX, -offsetY, offsetX, offsetY };
        break;
    }
    case Triangle: {
        float size = 0.1f;
        shape.vertices = { shape.centerX, shape.centerY + size, shape.r, shape.g, shape.b,
                           shape.centerX - size, shape.centerY - size, shape.r, shape.g, shape.b,
                           shape.centerX + size, shape.centerY - size, shape.r, shape.g, shape.b };
        shape.offsets = { 0.0f, size, -size, -size, size, -size };
        break;
    }
    case Rect: {
        float w = 0.1f;
        float h = 0.1f;
        shape.vertices = { shape.centerX - w, shape.centerY - h, shape.r, shape.g, shape.b,
                           shape.centerX + w, shape.centerY - h, shape.r, shape.g, shape.b,
                           shape.centerX + w, shape.centerY + h, shape.r, shape.g, shape.b,
                           shape.centerX - w, shape.centerY + h, shape.r, shape.g, shape.b };
        shape.offsets = { -w, -h, w, -h, w, h, -w, h };
        break;
    }
    case Pentagon: {
        float psize = 0.1f;
        for (int j = 0; j < 5; j++) {
            float angle = j * (2.0f * M_PI / 5.0f) + M_PI / 2.0f;
            float x = shape.centerX + psize * cos(angle);
            float y = shape.centerY + psize * sin(angle);
            shape.vertices.push_back(x);
            shape.vertices.push_back(y);
            shape.vertices.push_back(shape.r);
            shape.vertices.push_back(shape.g);
            shape.vertices.push_back(shape.b);
            shape.offsets.push_back(x - shape.centerX);
            shape.offsets.push_back(y - shape.centerY);
        }
        break;
    }
    }
}

void UpdateMovement()
{
    for (auto& shape : shapes)
    {
        int numVertices = shape.offsets.size() / 2;
        if (shape.state == ONE) { // 튕기기
            {
                float delta = shape.Speed / 1000.0f;

                // 중심점 이동
                if (shape.moveType == LEFTUP) {
                    shape.centerX -= delta;
                    shape.centerY += delta;
                }
                else if (shape.moveType == LEFTDOWN) {
                    shape.centerX -= delta;
                    shape.centerY -= delta;
                }
                else if (shape.moveType == RIGHTUP) {
                    shape.centerX += delta;
                    shape.centerY += delta;
                }
                else if (shape.moveType == RIGHTDOWN) {
                    shape.centerX += delta;
                    shape.centerY -= delta;
                }

                // 경계 체크 및 반사
                if (shape.centerX < -1.0f) {
                    shape.centerX = -1.0f;
                    if (shape.moveType == LEFTUP) shape.moveType = RIGHTUP;
                    else if (shape.moveType == LEFTDOWN) shape.moveType = RIGHTDOWN;
                }
                else if (shape.centerX > 1.0f) {
                    shape.centerX = 1.0f;
                    if (shape.moveType == RIGHTUP) shape.moveType = LEFTUP;
                    else if (shape.moveType == RIGHTDOWN) shape.moveType = LEFTDOWN;
                }
                if (shape.centerY < -1.0f) {
                    shape.centerY = -1.0f;
                    if (shape.moveType == LEFTDOWN) shape.moveType = LEFTUP;
                    else if (shape.moveType == RIGHTDOWN) shape.moveType = RIGHTUP;
                }
                else if (shape.centerY > 1.0f) {
                    shape.centerY = 1.0f;
                    if (shape.moveType == LEFTUP) shape.moveType = LEFTDOWN;
                    else if (shape.moveType == RIGHTUP) shape.moveType = RIGHTDOWN;
                }

                // 정점 위치 갱신
                for (size_t i = 0; i < numVertices; ++i) {
                    float x_offset = shape.offsets[i * 2];
                    float y_offset = shape.offsets[i * 2 + 1];
                    shape.vertices[i * 5] = shape.centerX + x_offset;
                    shape.vertices[i * 5 + 1] = shape.centerY + y_offset;
                }
            }
        }
        else if (shape.state == TWO) { // 지그재그 이동
            {
                float delta = shape.Speed / 1000.0f;

                // 이동 조건에 따른 이동
                if (shape.moveType == LEFT) {
                    shape.centerX -= delta;
                }
                else if (shape.moveType == RIGHT) {
                    shape.centerX += delta;
                }
                else if (shape.moveType == UP) {
                    shape.centerY += delta;
                }
                else if (shape.moveType == DOWN) {
                    shape.centerY -= delta;
                }

                if (shape.centerY < -1.0f) {
                    shape.reverseflag = true; // 다음에는 위로 이동
                }
                else if (shape.centerY > 1.0f) {
                    shape.reverseflag = false; // 다음에는 아래로 이동
                }

                // 경계 닿으면 방향 전환
                if (shape.centerX < -1.0f) {
                    if (shape.reverseflag == false) {
                        shape.moveType = DOWN;
                        shape.moveDistance += delta;
                        if (shape.moveDistance >= 0.2f) {
                            shape.moveType = RIGHT;
                            shape.moveDistance = 0.0f;
                        }
                    }
                    else {
                        shape.moveType = UP;
                        shape.moveDistance += delta;
                        if (shape.moveDistance >= 0.2f) {
                            shape.moveType = RIGHT;
                            shape.moveDistance = 0.0f;
                        }
                    }
                }
                else if (shape.centerX > 1.0f) {
                    if (shape.reverseflag == false) {
                        shape.moveType = DOWN;
                        shape.moveDistance += delta;
                        if (shape.moveDistance >= 0.2f) {
                            shape.moveType = LEFT;
                            shape.moveDistance = 0.0f;
                        }
                    }
                    else {
                        shape.moveType = UP;
                        shape.moveDistance += delta;
                        if (shape.moveDistance >= 0.2f) {
                            shape.moveType = LEFT;
                            shape.moveDistance = 0.0f;
                        }
                    }
                }

                // 정점 위치 갱신
                for (size_t i = 0; i < numVertices; ++i) {
                    float x_offset = shape.offsets[i * 2];
                    float y_offset = shape.offsets[i * 2 + 1];
                    shape.vertices[i * 5] = shape.centerX + x_offset;
                    shape.vertices[i * 5 + 1] = shape.centerY + y_offset;
                }
            }
        }
        // NONE 상태는 이동하지 않음
    }
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key)
    {
    case 'c': {
        InitShapes();
        break;
    }
    case 's': {
        animationRunning = !animationRunning;
        break;
    }
    case '1': {
        for (auto& shape : shapes) {
            shape.state = ONE;
            shape.moveType = static_cast<MoveType>(4 + (rand() % 4)); // LEFTUP 등
            shape.Speed = 10 + (rand() % 20);
            shape.reverseflag = false;
            shape.moveDistance = 0.0f;
            std::cout << "Shape MoveType: " << shape.moveType << std::endl;
        }
        break;
    }
    case '2': {
        for (auto& shape : shapes) {
            shape.state = TWO;
            shape.moveType = static_cast<MoveType>(rand() % 2); // LEFT or RIGHT
            shape.Speed = 10 + (rand() % 20);
            shape.reverseflag = false;
            shape.moveDistance = 0.0f;
            std::cout << "Shape MoveType: " << shape.moveType << std::endl;
        }
        break;
    }
    default:
        break;
    }
    glutPostRedisplay();
}

void Timer(int value)
{
    if (animationRunning) {
        UpdateMovement();
    }
    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0); // 다음 16ms 후에 다시 호출
}

void InitShapes()
{
    shapes.clear();
    //점, 선, 삼각형, 사각형, 오각형을 각각 3개씩 저장
    for (int i = 0; i < 15; i++)
    {
        float centerX = ((rand() % 200) / 100.0f) - 1.0f; // -1.0 ~ 1.0
        float centerY = ((rand() % 200) / 100.0f) - 1.0f; // -1.0 ~ 1.0
        Shape shape;
        shape.centerX = centerX;
        shape.centerY = centerY;
        shape.r = (rand() % 100) / 100.0f;
        shape.g = (rand() % 100) / 100.0f;
        shape.b = (rand() % 100) / 100.0f;
        shape.state = NONE;
        shape.moveType = None;
        shape.Speed = 0;
        if (i < 3) //점
        {
            shape.type = Point;
            shape.vertices = { centerX, centerY, shape.r, shape.g, shape.b };
            shape.offsets = { 0.0f, 0.0f };
        }
        else if (i < 6) //선
        {
            shape.type = Line;
            float offsetX = 0.1f;
            float offsetY = 0.1f;
            shape.vertices = { centerX - offsetX, centerY - offsetY, shape.r, shape.g, shape.b,
                               centerX + offsetX, centerY + offsetY, shape.r, shape.g, shape.b };
            shape.offsets = { -offsetX, -offsetY, offsetX, offsetY };
        }
        else if (i < 9) //삼각형
        {
            shape.type = Triangle;
            float size = 0.1f;
            shape.vertices = { centerX, centerY + size, shape.r, shape.g, shape.b,
                               centerX - size, centerY - size, shape.r, shape.g, shape.b,
                               centerX + size, centerY - size, shape.r, shape.g, shape.b };
            shape.offsets = { 0.0f, size, -size, -size, size, -size };
        }
        else if (i < 12)
        {
            shape.type = Rect;
            float w = 0.1f;
            float h = 0.1f;
            shape.vertices = { centerX - w, centerY - h, shape.r, shape.g, shape.b,
                               centerX + w, centerY - h, shape.r, shape.g, shape.b,
                               centerX + w, centerY + h, shape.r, shape.g, shape.b,
                               centerX - w, centerY + h, shape.r, shape.g, shape.b };
            shape.offsets = { -w, -h, w, -h, w, h, -w, h };
        }
        else //오각형
        {
            shape.type = Pentagon;
            float psize = 0.1f;
            for (int j = 0; j < 5; j++)
            {
                float angle = j * (2.0f * M_PI / 5.0f) + M_PI / 2.0f;
                float x = centerX + psize * cos(angle);
                float y = centerY + psize * sin(angle);
                shape.vertices.push_back(x);
                shape.vertices.push_back(y);
                shape.vertices.push_back(shape.r);
                shape.vertices.push_back(shape.g);
                shape.vertices.push_back(shape.b);
                shape.offsets.push_back(x - centerX);
                shape.offsets.push_back(y - centerY);
            }
        }
        shapes.push_back(shape);
    }
}