#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" // filetobuf 함수가 포함된 헤더 파일
#include <algorithm> // std::clamp
#include <vector>
#include <iostream>
#define M_PI 3.14159265358979323846

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void UpdateTriangles();
void Timer(int value);

GLint width = 800, height = 800;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

enum State {
    ONE, TWO, THREE, FOUR, NONE
    // ONE: 튕기기
    // TWO: 지그재그 이동
    // THREE: 사각 스파이럴
    // FOUR: 나선형
};

enum MoveType {
    LEFT, RIGHT, UP, DOWN,
    LEFTUP, LEFTDOWN, RIGHTUP, RIGHTDOWN,
    None
};

struct Triangle {
    std::vector<GLfloat> vertices; // x, y, r, g, b
    std::vector<GLfloat> offsets; // 중심점 기준 오프셋 (x, y)
    float center_x, center_y; // 삼각형 중심점
    MoveType moveType;
    int Speed;
    bool reverseflag = false; // 수직 이동 방향 플래그
    float moveDistance = 0.0f; // 이동 거리
    // 사각 스파이럴용 변수
    float spiralStepSize = 0.1f; // 초기 변 길이
    float currentStepRemaining = 0.1f; // 현재 변에서 남은 이동 거리
    int spiralTurns = 0; // 회전 횟수
    // 나선형용 변수
    float theta = 0.0f; // 현재 각도 (라디안)
    float radius = 0.1f; // 초기 반지름
    float angularSpeed = 0.05f; // 각속도 (라디안/프레임)
};

State globalState = NONE; // 전역 상태 변수
std::vector<Triangle> triangles;
int drawType = 0; // 0: 선, 1: 면

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("S9");

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
    glutTimerFunc(16, Timer, 0); // 16ms마다 Timer 호출 (약 60fps)

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

    // 위치 속성 (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    // 색상 속성 (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void DrawScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO);

    // 화면을 4등분하는 선
    std::vector<GLfloat> lines = {
        // 수직선
        0.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
        // 수평선
        -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         1.0f, 0.0f, 1.0f, 0.0f, 0.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(GLfloat), lines.data(), GL_DYNAMIC_DRAW);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 4);

    // 삼각형 렌더링
    for (const auto& triangle : triangles) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, triangle.vertices.size() * sizeof(GLfloat), triangle.vertices.data(), GL_DYNAMIC_DRAW);
        if (drawType == 0)
            glDrawArrays(GL_LINE_LOOP, 0, 3);
        else
            glDrawArrays(GL_TRIANGLES, 0, 3);
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
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float ndc_x = (2.0f * x / width) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / height);

        Triangle newTriangle;

        // 이등변 삼각형의 크기
        float base = 0.1f;
        float height_val = 0.1f;

        float r = (rand() % 100) / 100.0f;
        float g = (rand() % 100) / 100.0f;
        float b = (rand() % 100) / 100.0f;

        // 정점 좌표
        float x1 = ndc_x;
        float y1 = ndc_y;
        float x2 = ndc_x - base / 2.0f;
        float y2 = ndc_y - height_val;
        float x3 = ndc_x + base / 2.0f;
        float y3 = ndc_y - height_val;

        // 중심점 계산 (세 정점의 평균)
        newTriangle.center_x = (x1 + x2 + x3) / 3.0f;
        newTriangle.center_y = (y1 + y2 + y3) / 3.0f;

        // 중심점 기준 오프셋 계산
        newTriangle.offsets = {
            x1 - newTriangle.center_x, y1 - newTriangle.center_y, // 정점 1
            x2 - newTriangle.center_x, y2 - newTriangle.center_y, // 정점 2
            x3 - newTriangle.center_x, y3 - newTriangle.center_y  // 정점 3
        };

        // 정점 데이터 (절대 좌표)
        newTriangle.vertices = {
            x1, y1, r, g, b,
            x2, y2, r, g, b,
            x3, y3, r, g, b
        };

        newTriangle.moveType = static_cast<MoveType>(4 + (rand() % 4)); // LEFTUP, LEFTDOWN, RIGHTUP, RIGHTDOWN 중 랜덤
        newTriangle.Speed = 10 + (rand() % 20);
        newTriangle.reverseflag = false;
        triangles.push_back(newTriangle);
        glutPostRedisplay();
    }
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
        drawType = 1; // 면
        break;
    case 'b':
        drawType = 0; // 선
        break;
    case '1':
        globalState = ONE;
        for (auto& tri : triangles) {
            int moveType = 4 + (rand() % 4);
            tri.moveType = static_cast<MoveType>(moveType);
            std::cout << "MoveType: " << tri.moveType << std::endl;
        }
        break;
    case '2':
        globalState = TWO;
        for (auto& tri : triangles) {
            int moveType = rand() % 2;
            tri.moveType = static_cast<MoveType>(moveType);
            std::cout << "MoveType: " << tri.moveType << std::endl;
        }
        break;
    case '3':
        globalState = THREE;
        for (auto& tri : triangles) {
            tri.moveType = RIGHT; // 초기 방향: 오른쪽
            tri.center_x = 0.0f; // 중심 시작
            tri.center_y = 0.0f;
            tri.spiralStepSize = 0.1f; // 초기 변 길이
            tri.currentStepRemaining = tri.spiralStepSize; // 첫 변 길이
            tri.spiralTurns = 0;
        }
        break;
    case '4':
        globalState = FOUR;
        for (auto& tri : triangles) {
            tri.theta = 0.0f; // 초기 각도
            tri.radius = 0.1f; // 초기 반지름
            tri.angularSpeed = 0.05f + (rand() % 50) / 1000.0f; // 삼각형마다 약간 다른 각속도
            tri.center_x = 0.0f; // 중심 시작
            tri.center_y = 0.0f;
        }
        break;
    case 'c':
        triangles.clear();
        break;
    case 'q':
        exit(0);
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void UpdateTriangles() {
    for (auto& tri : triangles) {
        if (globalState == ONE) { // 튕기기
            float delta = tri.Speed / 1000.0f;

            // 중심점 이동
            if (tri.moveType == LEFTUP) {
                tri.center_x -= delta;
                tri.center_y += delta;
            }
            else if (tri.moveType == LEFTDOWN) {
                tri.center_x -= delta;
                tri.center_y -= delta;
            }
            else if (tri.moveType == RIGHTUP) {
                tri.center_x += delta;
                tri.center_y += delta;
            }
            else if (tri.moveType == RIGHTDOWN) {
                tri.center_x += delta;
                tri.center_y -= delta;
            }

            // 경계 체크 및 반사
            if (tri.center_x < -1.0f) {
                tri.center_x = -1.0f;
                if (tri.moveType == LEFTUP) tri.moveType = RIGHTUP;
                else if (tri.moveType == LEFTDOWN) tri.moveType = RIGHTDOWN;
            }
            else if (tri.center_x > 1.0f) {
                tri.center_x = 1.0f;
                if (tri.moveType == RIGHTUP) tri.moveType = LEFTUP;
                else if (tri.moveType == RIGHTDOWN) tri.moveType = LEFTDOWN;
            }
            if (tri.center_y < -1.0f) {
                tri.center_y = -1.0f;
                if (tri.moveType == LEFTDOWN) tri.moveType = LEFTUP;
                else if (tri.moveType == RIGHTDOWN) tri.moveType = RIGHTUP;
            }
            else if (tri.center_y > 1.0f) {
                tri.center_y = 1.0f;
                if (tri.moveType == LEFTUP) tri.moveType = LEFTDOWN;
                else if (tri.moveType == RIGHTUP) tri.moveType = RIGHTDOWN;
            }

            // 회전 각도 설정
            float angle;
            switch (tri.moveType) {
            case LEFTUP: angle = M_PI / 4.0f; break;
            case LEFTDOWN: angle = 3.0f * M_PI / 4.0f; break;
            case RIGHTUP: angle = 7.0f * M_PI / 4.0f; break;
            case RIGHTDOWN: angle = 5.0f * M_PI / 4.0f; break;
            default: angle = 0.0f; break;
            }

            // 정점 위치 갱신
            for (size_t i = 0; i < 3; ++i) {
                float x_offset = tri.offsets[i * 2];
                float y_offset = tri.offsets[i * 2 + 1];
                float cos_a = cos(angle);
                float sin_a = sin(angle);
                float rotated_x = x_offset * cos_a - y_offset * sin_a;
                float rotated_y = x_offset * sin_a + y_offset * cos_a;
                tri.vertices[i * 5] = tri.center_x + rotated_x;
                tri.vertices[i * 5 + 1] = tri.center_y + rotated_y;
            }
        }
        else if (globalState == TWO) { // 지그재그 이동
            float delta = tri.Speed / 1000.0f;

            // 이동 조건에 따른 이동
            if (tri.moveType == LEFT) {
                tri.center_x -= delta;
            }
            else if (tri.moveType == RIGHT) {
                tri.center_x += delta;
            }
            else if (tri.moveType == UP) {
                tri.center_y += delta;
            }
            else if (tri.moveType == DOWN) {
                tri.center_y -= delta;
            }

            if (tri.center_y < -1.0f) {
                tri.reverseflag = true; // 다음에는 위로 이동
            }
            else if (tri.center_y > 1.0f) {
                tri.reverseflag = false; // 다음에는 아래로 이동
            }

            // 경계 닿으면 방향 전환
            if (tri.center_x < -1.0f) {
                if (tri.reverseflag == false) {
                    tri.moveType = DOWN;
                    tri.moveDistance += delta;
                    if (tri.moveDistance >= 0.2f) {
                        tri.moveType = RIGHT;
                        tri.moveDistance = 0.0f;
                    }
                }
                else {
                    tri.moveType = UP;
                    tri.moveDistance += delta;
                    if (tri.moveDistance >= 0.2f) {
                        tri.moveType = RIGHT;
                        tri.moveDistance = 0.0f;
                    }
                }
            }
            else if (tri.center_x > 1.0f) {
                if (tri.reverseflag == false) {
                    tri.moveType = DOWN;
                    tri.moveDistance += delta;
                    if (tri.moveDistance >= 0.2f) {
                        tri.moveType = LEFT;
                        tri.moveDistance = 0.0f;
                    }
                }
                else {
                    tri.moveType = UP;
                    tri.moveDistance += delta;
                    if (tri.moveDistance >= 0.2f) {
                        tri.moveType = LEFT;
                        tri.moveDistance = 0.0f;
                    }
                }
            }

            float angle = 0.0f;
            if (tri.moveType == LEFT) angle = M_PI / 2;
            else if (tri.moveType == RIGHT) angle = -M_PI / 2;
            else if (tri.moveType == UP) angle = 0.0f;
            else if (tri.moveType == DOWN) angle = M_PI;

            // 정점 위치 갱신
            for (size_t i = 0; i < 3; ++i) {
                float x_offset = tri.offsets[i * 2];
                float y_offset = tri.offsets[i * 2 + 1];
                float cos_a = cos(angle);
                float sin_a = sin(angle);
                float rotated_x = x_offset * cos_a - y_offset * sin_a;
                float rotated_y = x_offset * sin_a + y_offset * cos_a;
                tri.vertices[i * 5] = tri.center_x + rotated_x;
                tri.vertices[i * 5 + 1] = tri.center_y + rotated_y;
            }
        }
        else if (globalState == THREE) { // 사각 스파이럴
            float delta = tri.Speed / 1000.0f;

            // 현재 방향에 따라 중심점 이동
            if (tri.moveType == RIGHT) {
                tri.center_x += delta;
            }
            else if (tri.moveType == DOWN) {
                tri.center_y -= delta;
            }
            else if (tri.moveType == LEFT) {
                tri.center_x -= delta;
            }
            else if (tri.moveType == UP) {
                tri.center_y += delta;
            }

            // 남은 이동 거리 감소
            tri.currentStepRemaining -= delta;

            // 남은 거리가 0 이하 되면 방향 전환
            if (tri.currentStepRemaining <= 0.0f) {
                // 다음 방향으로 전환 (시계 방향: RIGHT -> DOWN -> LEFT -> UP -> RIGHT)
                if (tri.moveType == RIGHT) {
                    tri.moveType = DOWN;
                }
                else if (tri.moveType == DOWN) {
                    tri.moveType = LEFT;
                }
                else if (tri.moveType == LEFT) {
                    tri.moveType = UP;
                }
                else if (tri.moveType == UP) {
                    tri.moveType = RIGHT;
                }

                // 회전 횟수 증가
                tri.spiralTurns++;

                // 1번 회전마다 stepSize 증가 (스파이럴 확대)
                if (tri.spiralTurns % 1 == 0) {
                    tri.spiralStepSize += 0.12f;
                }

                // 다음 변의 남은 거리 설정
                tri.currentStepRemaining = tri.spiralStepSize;
            }

            // 초기 위치로 돌아가기
            if (tri.center_x > 1.0f || tri.center_x < -1.0f || tri.center_y < -1.0f || tri.center_y > 1.0f) {
                tri.center_x = 0.0f;
                tri.center_y = 0.0f;
                tri.moveType = None;
            }

            // 각도 반영
            float angle = 0.0f;
            if (tri.moveType == LEFT) angle = M_PI / 2.0f;
            else if (tri.moveType == RIGHT) angle = -M_PI / 2.0f;
            else if (tri.moveType == UP) angle = 0.0f;
            else if (tri.moveType == DOWN) angle = M_PI;

            // 정점 위치 갱신
            for (size_t i = 0; i < 3; ++i) {
                float x_offset = tri.offsets[i * 2];
                float y_offset = tri.offsets[i * 2 + 1];
                float cos_a = cos(angle);
                float sin_a = sin(angle);
                float rotated_x = x_offset * cos_a - y_offset * sin_a;
                float rotated_y = x_offset * sin_a + y_offset * cos_a;
                tri.vertices[i * 5] = tri.center_x + rotated_x;
                tri.vertices[i * 5 + 1] = tri.center_y + rotated_y;
            }
        }
        else if (globalState == FOUR) { // 나선형
            
            tri.theta += tri.angularSpeed; // 각속도
            tri.radius += 0.001f; // 반지름 증가율 
            // 중심점 계산 (극좌표 -> 직교좌표)
            tri.center_x = tri.radius * cos(tri.theta);
            tri.center_y = tri.radius * sin(tri.theta);

            // 경계 체크 및 리셋
            if (tri.radius > 1.0f || tri.center_x > 1.0f || tri.center_x < -1.0f ||
                tri.center_y > 1.0f || tri.center_y < -1.0f) {
                tri.theta = 0.0f;
                tri.radius = 0.1f; // 초기 반지름으로 리셋
                tri.center_x = 0.0f;
                tri.center_y = 0.0f;
            }

            // 이동 방향(접선 방향)에 따른 삼각형 회전
            // 원의 접선 방향 벡터: (-sin(theta), cos(theta))
            float tangent_angle = tri.theta; // 접선 방향 (90도 회전)

            // 정점 위치 갱신
            for (size_t i = 0; i < 3; ++i) {
                float x_offset = tri.offsets[i * 2];
                float y_offset = tri.offsets[i * 2 + 1];
                float cos_a = cos(tangent_angle);
                float sin_a = sin(tangent_angle);
                float rotated_x = x_offset * cos_a - y_offset * sin_a;
                float rotated_y = x_offset * sin_a + y_offset * cos_a;
                tri.vertices[i * 5] = tri.center_x + rotated_x;
                tri.vertices[i * 5 + 1] = tri.center_y + rotated_y;
            }
        }
        // NONE 상태는 이동하지 않음
    }
}

void Timer(int value) {
    UpdateTriangles();
    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0); // 다음 16ms 후에 다시 호출
}