#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <iostream>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500

struct Rect {
    float x1, y1, x2, y2;
    float r, g, b;
};

std::vector<Rect> boardRects;

void InitBoard();
void DrawScene();
void Reshape(int w, int h);
void DrawBoard();

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("S7 - 속이 빈 사각형 보드판");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "GLEW 초기화 성공\n";
    }

    InitBoard();
    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutMainLoop();
    return 0;
}

void InitBoard() {
    boardRects.clear();

    // 고정된 좌표로 10개의 사각형 정의 (면으로 연결, 겹치지 않음, 위아래 길쭉한 직사각형 위주)
    std::vector<Rect> predefinedRects = {
        // 첫 번째 줄
        {-0.9f, 0.9f, -0.75f, 0.4f, 0.6f, 0.4f, 0.5f}, // 사각형 1 (위아래 길쭉: 가로 0.15, 세로 0.5)
        {-0.75f, 0.9f, -0.55f, 0.5f, 0.5f, 0.6f, 0.4f}, // 사각형 2 (위아래 길쭉: 가로 0.2, 세로 0.4, 오른쪽 변 연결)

        // 두 번째 줄
        {-0.9f, 0.4f, -0.7f, 0.0f, 0.7f, 0.5f, 0.3f},  // 사각형 3 (위아래 길쭉: 가로 0.2, 세로 0.4, 사각형 1 아래쪽 변 연결)
        {-0.7f, 0.4f, -0.4f, 0.15f, 0.3f, 0.7f, 0.5f},  // 사각형 4 (좌우 길쭉: 가로 0.3, 세로 0.25, 사각형 3 오른쪽 변 연결)

        // 세 번째 줄
        {-0.9f, 0.0f, -0.75f, -0.4f, 0.5f, 0.3f, 0.7f}, // 사각형 5 (위아래 길쭉: 가로 0.15, 세로 0.4, 사각형 3 아래쪽 변 연결)
        {-0.75f, 0.0f, -0.45f, -0.25f, 0.6f, 0.7f, 0.4f}, // 사각형 6 (좌우 길쭉: 가로 0.3, 세로 0.25, 사각형 5 오른쪽 변 연결)

        // 네 번째 줄
        {-0.9f, -0.4f, -0.7f, -0.8f, 0.7f, 0.4f, 0.6f},  // 사각형 7 (위아래 길쭉: 가로 0.2, 세로 0.4, 사각형 5 아래쪽 변 연결)
        {-0.7f, -0.25f, -0.5f, -0.65f, 0.4f, 0.6f, 0.5f}, // 사각형 8 (위아래 길쭉: 가로 0.2, 세로 0.4, 사각형 6 오른쪽 변 연결)

        // 다섯 번째 줄
        {-0.9f, -0.8f, -0.65f, -1.0f, 0.5f, 0.6f, 0.7f}, // 사각형 9 (좌우 길쭉: 가로 0.25, 세로 0.2, 사각형 7 아래쪽 변 연결)
        {-0.65f, -0.8f, -0.45f, -1.2f, 0.6f, 0.4f, 0.5f} // 사각형 10 (위아래 길쭉: 가로 0.2, 세로 0.4, 사각형 9 오른쪽 변 연결)
    };

    boardRects = predefinedRects;
}

void DrawScene() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 어두운 배경
    glClear(GL_COLOR_BUFFER_BIT);
    DrawBoard();
    glutSwapBuffers();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void DrawBoard() {
    glLineWidth(2.0f); // 테두리 선 두께 설정
    for (const auto& rect : boardRects) {
        glColor3f(rect.r, rect.g, rect.b);
        glBegin(GL_LINE_LOOP);
        glVertex2f(rect.x1, rect.y1); // 좌상단
        glVertex2f(rect.x2, rect.y1); // 우상단
        glVertex2f(rect.x2, rect.y2); // 우하단
        glVertex2f(rect.x1, rect.y2); // 좌하단
        glEnd();
    }
}