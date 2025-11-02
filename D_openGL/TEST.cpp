#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include "CommonInclude.h"

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);
void DrawWalls();

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint wallVAO, wallVBO;

// 마우스 회전 변수
bool isMousePressed = false;
int mouseX, mouseY;
float rotateX = 0.0f, rotateY = 0.0f;

// 카메라 설정
glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 8.0f);
float cameraOffsetZ = 0.0f;
float cameraRotationY = 0.0f;

// 벽 회전 변수 추가
float wallRotationX = 0.0f;
float wallRotationY = 0.0f;
float wallRotationZ = 0.0f;

// 공간 경계
const float ROOM_SIZE = 4.0f;
const float FLOOR_Y = -4.0f;  // 바닥을 더 아래로 이동

// 바닥 열림 상태
bool floorOpen = false;

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("실습 21");

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
    glutMotionFunc(Motion);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, Timer, 0);

    glutMainLoop();
    return 0;
}

void MakeVertexShaders() {
    GLchar* vertexSource = filetobuf("vertex3D.glsl");
    if (!vertexSource) {
        std::cerr << "ERROR: vertex3D.glsl 파일을 읽지 못했습니다." << std::endl;
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
    // 벽면 버퍼 초기화
    glGenVertexArrays(1, &wallVAO);
    glBindVertexArray(wallVAO);

    glGenBuffers(1, &wallVBO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void DrawWalls() {
    std::vector<GLfloat> wallData;

    // 왼쪽 벽 (빨간색) - X = -ROOM_SIZE
    float leftWall[] = {
        -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // 왼쪽 아래 뒤
      -ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // 왼쪽 아래 앞
        -ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // 왼쪽 위 앞
        -ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // 왼쪽 위 앞
        -ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // 왼쪽 위 뒤
        -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 1.0f, 0.0f, 0.0f   // 왼쪽 아래 뒤
    };

    // 오른쪽 벽 (초록색) - X = ROOM_SIZE
    float rightWall[] = {
    ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 1.0f, 0.0f,  // 오른쪽 아래 뒤
      ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 1.0f, 0.0f,  // 오른쪽 위 뒤
        ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 0.0f, 1.0f, 0.0f,  // 오른쪽 위 앞
    ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 0.0f, 1.0f, 0.0f,// 오른쪽 위 앞
        ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.0f, 1.0f, 0.0f,  // 오른쪽 아래 앞
        ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 1.0f, 0.0f   // 오른쪽 아래 뒤
    };

    // 뒷벽 (파란색) - Z = -ROOM_SIZE
    float backWall[] = {
        -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // 왼쪽 아래 뒤
        ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // 오른쪽 아래 뒤
ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // 오른쪽 위 뒤
        ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // 오른쪽 위 뒤
        -ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // 왼쪽 위 뒤
        -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 0.0f, 1.0f   // 왼쪽 아래 뒤
    };

    // 위쪽 벽 (노란색) - Y = ROOM_SIZE
    float topWall[] = {
        -ROOM_SIZE, ROOM_SIZE, -ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // 왼쪽 위 뒤
        -ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // 왼쪽 위 앞
        ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // 오른쪽 위 앞
        ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // 오른쪽 위 앞
  ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // 오른쪽 위 뒤
 -ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 1.0f, 1.0f, 0.0f   // 왼쪽 위 뒤
    };

    // 바닥 (회색, 조건부) - Y = FLOOR_Y (가장 아래)
    if (!floorOpen) {
        float bottomWall[] = {
            -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // 왼쪽 바닥 뒤
   -ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // 왼쪽 바닥 앞
      ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // 오른쪽 바닥 앞
            ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // 오른쪽 바닥 앞
    ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // 오른쪽 바닥 뒤
         -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.5f, 0.5f, 0.5f   // 왼쪽 바닥 뒤
        };

        for (int i = 0; i < 36; i++) {
            wallData.push_back(bottomWall[i]);
        }
    }

    // 모든 벽 데이터 추가 (앞면 제외)
    for (int i = 0; i < 36; i++) {
        wallData.push_back(leftWall[i]);
    }
    for (int i = 0; i < 36; i++) {
        wallData.push_back(rightWall[i]);
    }
    for (int i = 0; i < 36; i++) {
        wallData.push_back(backWall[i]);
    }
    for (int i = 0; i < 36; i++) {
        wallData.push_back(topWall[i]);
    }

    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, wallData.size() * sizeof(GLfloat), wallData.data(), GL_DYNAMIC_DRAW);

    // 벽에 회전 변환 적용
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::rotate(modelMat, glm::radians(wallRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));  // Z축 기준 회전

    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

    // 컬링 비활성화 - 내부가 보이도록
    glDisable(GL_CULL_FACE);

    int wallCount = floorOpen ? 4 : 5;
    glDrawArrays(GL_TRIANGLES, 0, wallCount * 6);

    glBindVertexArray(0);
}

void DrawScene() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    // 카메라 설정
    glm::vec3 adjustedCameraPos = cameraPos + glm::vec3(0.0f, 0.0f, cameraOffsetZ);

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::rotate(view, glm::radians(cameraRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(rotateY), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::lookAt(adjustedCameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    // 벽만 그리기
    DrawWalls();

    glutSwapBuffers();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

void Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isMousePressed = true;
            mouseX = x;
            mouseY = y;
        }
        else if (state == GLUT_UP) {
            isMousePressed = false;
        }
    }
}

void Motion(int x, int y) {
    if (isMousePressed) {
        int deltaX = x - mouseX;
        int deltaY = y - mouseY;

        // 카메라 회전
        rotateY += deltaX * 0.5f;
        rotateX += deltaY * 0.5f;

        mouseX = x;
        mouseY = y;

        glutPostRedisplay();
    }
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'z':
        cameraOffsetZ += 0.5f;
        break;
    case 'Z':
        cameraOffsetZ -= 0.5f;
        break;
    case 'y':
        wallRotationZ += 5.0f;
        break;
    case 'Y':
        wallRotationZ -= 5.0f;
        break;
    case ' ':
        floorOpen = !floorOpen;
        break;
    case 'q':
        exit(0);
        break;
    }

    glutPostRedisplay();
}

void Timer(int value) {
    float deltaTime = 1.0f / 60.0f;

    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}