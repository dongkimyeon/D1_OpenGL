#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" // filetobuf 함수가 포함된 헤더 파일
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
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
void UpdateSpirals();
void Timer(int value);
GLint width = 800, height = 800;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO;
float bg_r = 0.0f, bg_g = 0.0f, bg_b = 0.0f; // 배경색
int drawType = 0; // 0: 점, 1: 선
struct Spiral {
    std::vector<GLfloat> vertices; // x, y, r, g, b
    float center_x, center_y; // 스파이럴 중심
    float theta = 0.0f; // 현재 각도 (라디안)
    float radius = 0.05f; // 초기 반지름
    float angularSpeed; // 각속도 (시계/반시계 포함)
    float radiusSpeed = 0.001f; // 반지름 증가 속도
    bool isOutward = true; // true: 밖으로, false: 안으로
    int turns; // 목표 바퀴 수
    int currentTurn = 0; // 현재 바퀴 수
    float end_x, end_y; // 종료 지점 (픽셀 좌표)
    float r, g, b; // 색상
    bool isActive = true; // 스파이럴이 업데이트되는지 여부
};
std::vector<Spiral> spirals;
int main(int argc, char** argv) {
    srand(time(nullptr));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Spiral Animation");
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
    glClearColor(bg_r, bg_g, bg_b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO);
    // 스파이럴 렌더링
    for (const auto& spiral : spirals) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, spiral.vertices.size() * sizeof(GLfloat), spiral.vertices.data(), GL_DYNAMIC_DRAW);
        if (drawType == 0) {
            glPointSize(3.0f);
            glDrawArrays(GL_POINTS, 0, spiral.vertices.size() / 5);
        }
        else {
            glLineWidth(2.0f);
            glDrawArrays(GL_LINE_STRIP, 0, spiral.vertices.size() / 5);
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
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // 배경색 랜덤 변경
        bg_r = (rand() % 100) / 100.0f;
        bg_g = (rand() % 100) / 100.0f;
        bg_b = (rand() % 100) / 100.0f;
        Spiral newSpiral;
        newSpiral.center_x = (2.0f * x / width) - 1.0f;
        newSpiral.center_y = 1.0f - (2.0f * y / height);
        std::cout << "New spiral at (" << newSpiral.center_x << ", " << newSpiral.center_y << ")\n";
        newSpiral.theta = 0.0f;
        // 랜덤하게 시계/반시계 방향 설정
        newSpiral.angularSpeed = (rand() % 2 == 0 ? 0.05f : -0.05f); // 50% 확률로 시계(-) 또는 반시계(+)
        newSpiral.radiusSpeed = 0.001f; // 반지름 증가 속도
        newSpiral.isOutward = true; // 밖으로 감기도록 설정
        newSpiral.turns = rand() % 2 + 2; // 2~3 바퀴
        newSpiral.radius = 0.0f;
        newSpiral.currentTurn = 0;
        newSpiral.r = (rand() % 100) / 100.0f;
        newSpiral.g = (rand() % 100) / 100.0f;
        newSpiral.b = (rand() % 100) / 100.0f;
        newSpiral.isActive = true;
        spirals.push_back(newSpiral);
        glutPostRedisplay();
    }
}
void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'p':
        drawType = 0; // 점
        break;
    case 'l':
        drawType = 1; // 선
        break;
    case '1': case '2': case '3': case '4': case '5': {
        int numSpirals = key - '0'; // '1'->1, '2'->2, ..., '5'->5
        for (int i = 0; i < numSpirals; ++i) {
            Spiral newSpiral;
            float rand_x = rand() % width;
            float rand_y = rand() % height;
            newSpiral.center_x = (2.0f * rand_x / width) - 1.0f;
            newSpiral.center_y = 1.0f - (2.0f * rand_y / height);
            std::cout << "New spiral at (" << newSpiral.center_x << ", " << newSpiral.center_y << ")\n";
            newSpiral.theta = 0.0f;
            newSpiral.angularSpeed = (rand() % 2 == 0 ? 0.05f : -0.05f); // 50% 확률로 시계(-) 또는 반시계(+)
            newSpiral.radiusSpeed = 0.001f;
            newSpiral.isOutward = true;
            newSpiral.turns = rand() % 2 + 2; // 2~3 바퀴
            newSpiral.radius = 0.0f;
            newSpiral.currentTurn = 0;
            newSpiral.r = (rand() % 100) / 100.0f;
            newSpiral.g = (rand() % 100) / 100.0f;
            newSpiral.b = (rand() % 100) / 100.0f;
            newSpiral.isActive = true;
            spirals.push_back(newSpiral);
        }
        break;
    }
    case 'c':
        spirals.clear();
        bg_r = bg_g = bg_b = 0.0f; // 배경색 초기화
        break;
    case 'q':
        exit(0);
        break;
    default:
        break;
    }
    glutPostRedisplay();
}
void UpdateSpirals() {
    for (auto& spiral : spirals) {
        if (!spiral.isActive) continue; // 비활성 스파이럴은 업데이트 안 함
        // 각도와 반지름 업데이트
        if (spiral.isOutward) {
            // 밖으로 감김
            spiral.radius += spiral.radiusSpeed;
            spiral.theta += spiral.angularSpeed;
        }
        else {
            // 안으로 감김
            spiral.radius += spiral.radiusSpeed; // radiusSpeed는 음수 (-0.001f)
            spiral.theta += spiral.angularSpeed;
        }
        // 안으로 감길 때 종료 조건 체크 (점 추가 전에)
        if (!spiral.isOutward) {
            if (spiral.radius <= 0.0f) {
                spiral.isActive = false; // 스파이럴 비활성화
                std::cout << "Spiral reached center, deactivated.\n";
                continue;
            }
        }
        // 새 점 추가
        float x = spiral.center_x + spiral.radius * cos(spiral.theta) / 2;
        float y = spiral.center_y + spiral.radius * sin(spiral.theta) / 2;
        spiral.vertices.push_back(x);
        spiral.vertices.push_back(y);
        spiral.vertices.push_back(spiral.r);
        spiral.vertices.push_back(spiral.g);
        spiral.vertices.push_back(spiral.b);
        // 종료 조건 (outward)
        if (spiral.isOutward) {
            // 바퀴 수 체크
            float turnProgress = fabs(spiral.theta) / (2 * M_PI);
            if (turnProgress >= spiral.turns) {
                spiral.end_x = x;
                spiral.end_y = y;
                float phys_r = spiral.radius / 2.0f;
                float offset_x = phys_r * cos(spiral.theta);
                float offset_y = phys_r * sin(spiral.theta);
                spiral.center_x = x + offset_x;
                spiral.center_y = y + offset_y;
                float dx = x - spiral.center_x;
                float dy = y - spiral.center_y;
                spiral.radius = 2.0f * sqrt(dx * dx + dy * dy);
                spiral.theta = atan2(dy, dx);
                // 방향 반전 및 속도 설정
                spiral.angularSpeed = -spiral.angularSpeed;
                spiral.radiusSpeed = -spiral.radiusSpeed;
                spiral.isOutward = false; // 안으로 감기 시작
                std::cout << "Spiral reached target turns, now spiraling inward from new center (" << spiral.center_x << ", " << spiral.center_y << ").\n";
            }
        }
    }
}
void Timer(int value) {
    UpdateSpirals();
    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0); // 다음 16ms 후에 호출
}