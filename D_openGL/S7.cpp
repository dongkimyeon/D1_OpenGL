#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <iostream>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500

struct Rect {
    float x1, y1, x2, y2;
    float r, g, b;
	bool ismatched = false;
};

std::vector<Rect> boardRects;
std::vector<Rect> UserRects;

int selectedRectIndex = -1;
bool isDragging = false;

void InitBoard();
void DrawScene();
void Reshape(int w, int h);
void DrawBoard();
void InitUserRects();
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void Motion(int x, int y);

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("S7");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "GLEW 초기화 성공\n";
    }

    InitBoard();
    InitUserRects();
    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
	glutKeyboardFunc(Keyboard);
    glutMainLoop();
    return 0;
}

void InitBoard() {
    boardRects.clear();

    std::vector<Rect> predefinedRects = {
        // 첫 번째 줄
        {-0.9f, 0.9f, -0.75f, 0.4f, 0.6f, 0.4f, 0.5f}, // 사각형 1
        {-0.75f, 0.9f, -0.55f, 0.5f, 0.5f, 0.6f, 0.4f}, // 사각형 2

        // 두 번째 줄
        {-0.9f, 0.4f, -0.7f, 0.0f, 0.7f, 0.5f, 0.3f},  // 사각형 3
        {-0.7f, 0.4f, -0.4f, 0.15f, 0.3f, 0.7f, 0.5f},  // 사각형 4

        // 세 번째 줄
        {-0.9f, 0.0f, -0.75f, -0.4f, 0.5f, 0.3f, 0.7f}, // 사각형 5
        {-0.75f, 0.0f, -0.45f, -0.25f, 0.6f, 0.7f, 0.4f}, // 사각형 6

        // 네 번째 줄
        {-0.9f, -0.4f, -0.7f, -0.8f, 0.7f, 0.4f, 0.6f},  // 사각형 7
        {-0.7f, -0.25f, -0.5f, -0.65f, 0.4f, 0.6f, 0.5f}, // 사각형 8

        // 다섯 번째 줄
        {-0.9f, -0.8f, -0.65f, -1.0f, 0.5f, 0.6f, 0.7f}, // 사각형 9
        {-0.65f, -0.8f, -0.45f, -1.0f, 0.6f, 0.4f, 0.5f} // 사각형 10
    };

    boardRects = predefinedRects;

    /*for (int i = 0; i < boardRects.size(); i++) {
        std::cout << "Rect " << i + 1 << ": (" << boardRects[i].x1 << ", " << boardRects[i].y1 << ", "
            << boardRects[i].x2 << ", " << boardRects[i].y2 << ")\n";
    }*/
}

void InitUserRects() {
    UserRects.clear();
    for (const auto& r : boardRects) 
    {
        Rect temp = r;
		float offset = 0.85f; // x축으로 0.5만큼 이동
		temp.x1 += offset;
		temp.x2 += offset;

        UserRects.push_back(temp); // boardRects와 동일한 좌표로 초기화
    }
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
    for (const auto& rect : UserRects) {
        glColor3f(rect.r, rect.g, rect.b);
        glRectf(rect.x1, rect.y1, rect.x2, rect.y2); // 채워진 사각형
    }
}

void Mouse(int button, int state, int x, int y) {
    float ndc_x = (2.0f * x / glutGet(GLUT_WINDOW_WIDTH)) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / glutGet(GLUT_WINDOW_HEIGHT));

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            selectedRectIndex = -1;
            for (int i = UserRects.size() - 1; i >= 0; --i) {
                if (ndc_x >= UserRects[i].x1 && ndc_x <= UserRects[i].x2 &&
                    ndc_y <= UserRects[i].y1 && ndc_y >= UserRects[i].y2 && !UserRects[i].ismatched) {
                    selectedRectIndex = i;
                    isDragging = true;
                    //std::cout << "Selected Rect: " << i << " at (" << ndc_x << ", " << ndc_y << ")\n";
                    break;
                }
            }
        }
        else if (state == GLUT_UP) {
            if (selectedRectIndex >= 0) {
                // 보드 사각형과 유저 사각형 비교
                const float EPSILON = 0.05f; // 오차 허용 범위 
                bool matched = false;
                int matchedBoardIndex = -1;

                for (int i = 0; i < boardRects.size(); ++i) {
                    const auto& board = boardRects[i];
                    const auto& user = UserRects[selectedRectIndex];

                    // 중심점 기반으로 근접 여부 확인
                    float userCenterX = (user.x1 + user.x2) / 2.0f;
                    float userCenterY = (user.y1 + user.y2) / 2.0f;
                    float boardCenterX = (board.x1 + board.x2) / 2.0f;
                    float boardCenterY = (board.y1 + board.y2) / 2.0f;

                    if (std::abs(userCenterX - boardCenterX) < EPSILON &&
                        std::abs(userCenterY - boardCenterY) < EPSILON &&
                        std::abs(user.x2 - user.x1 - (board.x2 - board.x1)) < EPSILON &&
                        std::abs(user.y1 - user.y2 - (board.y1 - board.y2)) < EPSILON
                       ) {
                        matched = true;
                        matchedBoardIndex = i;
                        break;
                    }
                }

                if (matched && UserRects[selectedRectIndex].r == boardRects[matchedBoardIndex].r
                    && UserRects[selectedRectIndex].g == boardRects[matchedBoardIndex].g
                    && UserRects[selectedRectIndex].b == boardRects[matchedBoardIndex].b) {
                    // 스냅: 유저 사각형을 보드 사각형의 정확한 위치로 이동
                    UserRects[selectedRectIndex].x1 = boardRects[matchedBoardIndex].x1;
                    UserRects[selectedRectIndex].y1 = boardRects[matchedBoardIndex].y1;
                    UserRects[selectedRectIndex].x2 = boardRects[matchedBoardIndex].x2;
                    UserRects[selectedRectIndex].y2 = boardRects[matchedBoardIndex].y2;

					UserRects[selectedRectIndex].ismatched = true; // 매칭 완료 표시
                    UserRects[selectedRectIndex].r = 1.0f;
                    UserRects[selectedRectIndex].g = 1.0f;
                    UserRects[selectedRectIndex].b = 1.0f;

                    std::cout << "UserRect " << selectedRectIndex << " matched to BoardRect " << matchedBoardIndex << "\n";
                }
            }
            isDragging = false;
            selectedRectIndex = -1;
            glutPostRedisplay();
        }
    }
}
void Motion(int x, int y) {
    if (isDragging && selectedRectIndex >= 0 && selectedRectIndex < static_cast<int>(UserRects.size())) {
        float ndc_x = (2.0f * x / glutGet(GLUT_WINDOW_WIDTH)) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / glutGet(GLUT_WINDOW_HEIGHT));
        // 사각형 이동
        float width = UserRects[selectedRectIndex].x2 - UserRects[selectedRectIndex].x1;
        float height = UserRects[selectedRectIndex].y1 - UserRects[selectedRectIndex].y2;
        UserRects[selectedRectIndex].x1 = ndc_x - width / 2.0f;
        UserRects[selectedRectIndex].x2 = ndc_x + width / 2.0f;
        UserRects[selectedRectIndex].y1 = ndc_y + height / 2.0f;
        UserRects[selectedRectIndex].y2 = ndc_y - height / 2.0f;
        glutPostRedisplay();
    }
}

void Keyboard(unsigned char key, int x, int y)
{
    if(key == 'q') // ESC 키
    {
        exit(0);
    }
    else if (key == 'r' || key == 'R') // 'r' 키를 눌러 초기화
    {
        InitUserRects();
        isDragging = false;
        selectedRectIndex = -1;
        glutPostRedisplay();
	}
}