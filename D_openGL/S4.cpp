#include "CommonInclude.h"
#include <queue>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

enum Direction
{
	LEFT, RIGHT, RIGHTUPDIG, RIGHTDOWNDIG, LEFTUPDIG, LEFTDOWNDIG
};

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> color_dist(0, 255);
std::uniform_int_distribution<int> coord_dist(0, 800);
std::uniform_int_distribution<int> size_dist(0, 1);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void DrawRects();
void Keyboard(unsigned char key, int x, int y);
void DiagonalFunction(int value);
void ZigzagFunction(int value);
void SizeChangeFunction(int value);
void ColorChangeFunction(int value);
void CopyMoveFunction(int value);

struct Rect {
    GLfloat x1, y1, x2, y2;
    float r, g, b;
    Direction dirX = RIGHT; // X축 이동 방향
    Direction dirY = RIGHTUPDIG; // Y축 이동 방향
    bool reverseflag = false; // 수직 이동 방향 플래그
};

// 리더 사각형의 위치와 상태를 저장하는 구조체
struct Position {
    GLfloat x1, y1, x2, y2;
    Direction dirX, dirY;
    bool reverseflag;
};

std::vector<Rect> rects;
std::vector<std::queue<Position>> rectHistories; // 위치 히스토리 저장
std::vector<Rect> originalRects; // 원래 위치 저장
GLclampf bgColor[3] = { 0.2f, 0.2f, 0.2f };
int selectedRectIndex = -1;
bool isDragging = false;
bool isColor = false;
bool isSize = false;
bool isDiagonal = false;
bool isZigzag = false;
bool isCopy = false;

int randomIndex = -1;

void InitRects()
{
    // TODO
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("실습４");
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "GLEW Initialized\n";
    }
    InitRects();
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutKeyboardFunc(Keyboard);
    glutMainLoop();
    return 0;
}

GLvoid drawScene() {
    glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    DrawRects();
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void DrawRects() {
    for (auto& rect : rects) {
        glColor3f(rect.r, rect.g, rect.b);
        glRectf(rect.x1, rect.y1, rect.x2, rect.y2);
    }
}

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1':
        isDiagonal = !isDiagonal;
        if (isDiagonal) isZigzag = false;
        glutTimerFunc(100, DiagonalFunction, 1);
        break;
    case '2':
        isZigzag = !isZigzag;
        if (isZigzag) isDiagonal = false;
        glutTimerFunc(100, ZigzagFunction, 1);
        break;
    case '3':
        isSize = !isSize;
        glutTimerFunc(100, SizeChangeFunction, 1);
        break;
    case '4':
        isColor = !isColor;
        glutTimerFunc(100, ColorChangeFunction, 1);
        break;
    case '5':
        isCopy = !isCopy;
        if (isCopy && !rects.empty()) {
            randomIndex = rand() % rects.size();
            // 히스토리 초기화
            for (auto& history : rectHistories) {
                while (!history.empty()) history.pop();
            }
        }
        else {
            randomIndex = -1; // isCopy가 꺼지면 randomIndex 초기화
        }
        glutTimerFunc(100, CopyMoveFunction, 1);
        break;
    case 's':
        isColor = false;
        isSize = false;
        isZigzag = false;
        isDiagonal = false;
        isCopy = false;
        randomIndex = -1;
        break;
    case 'm':
        for(int i =0; i < rects.size(); i++)
        {
            rects[i] = originalRects[i]; // 원래 위치로 복원
            while (!rectHistories[i].empty()) rectHistories[i].pop(); 
		}
        break;
    case 'r':
        rects.clear();
        rectHistories.clear(); // 히스토리 초기화
        randomIndex = -1;
        glutPostRedisplay();
        break;
    case 'q':
        glutDestroyWindow(1);
        break;
    default:
        break;
    }
}

void Mouse(int button, int state, int x, int y) {
    float ndc_x = (2.0f * x / WINDOW_WIDTH) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / WINDOW_HEIGHT);

    // 좌클릭 누름
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (rects.size() < 5)
        {
            Rect tempRect;
            float size = 0.1f;
            tempRect.x1 = ndc_x - size;
            tempRect.x2 = ndc_x + size;
            tempRect.y1 = ndc_y - size;
            tempRect.y2 = ndc_y + size;
            tempRect.r = color_dist(mt) / 255.0f;
            tempRect.g = color_dist(mt) / 255.0f;
            tempRect.b = color_dist(mt) / 255.0f;
            rects.push_back(tempRect);
            rectHistories.push_back(std::queue<Position>()); // 히스토리 큐 추가
			originalRects.push_back(tempRect); // 원래 위치 저장
        }
    }
    glutPostRedisplay();
}

void DiagonalFunction(int value)
{
    if (!isDiagonal) return;

    glutPostRedisplay();
    for (auto& rect : rects)
    {
        // 방향에 따른 이동
        if (rect.dirY == RIGHTUPDIG)
        {
            rect.x1 += 0.05f;
            rect.x2 += 0.05f;
            rect.y1 += 0.05f;
            rect.y2 += 0.05f;
            if (rect.x2 >= 1.0f || rect.y2 >= 1.0f) rect.dirY = RIGHTDOWNDIG;
        }
        else if (rect.dirY == RIGHTDOWNDIG)
        {
            rect.x1 += 0.05f;
            rect.x2 += 0.05f;
            rect.y1 -= 0.05f;
            rect.y2 -= 0.05f;
            if (rect.x2 >= 1.0f || rect.y1 <= -1.0f) rect.dirY = LEFTDOWNDIG;
        }
        else if (rect.dirY == LEFTDOWNDIG)
        {
            rect.x1 -= 0.05f;
            rect.x2 -= 0.05f;
            rect.y1 -= 0.05f;
            rect.y2 -= 0.05f;
            if (rect.x1 <= -1.0f || rect.y1 <= -1.0f) rect.dirY = LEFTUPDIG;
        }
        else if (rect.dirY == LEFTUPDIG)
        {
            rect.x1 -= 0.05f;
            rect.x2 -= 0.05f;
            rect.y1 += 0.05f;
            rect.y2 += 0.05f;
            if (rect.x1 <= -1.0f || rect.y2 >= 1.0f) rect.dirY = RIGHTUPDIG;
        }
    }
    glutTimerFunc(100, DiagonalFunction, 1);
}

void ZigzagFunction(int value)
{
    if (!isZigzag) return;

    glutPostRedisplay();

    for (auto& rect : rects)
    {
        // 수평 이동 (좌우)
        if (rect.dirX == RIGHT)
        {
            rect.x1 += 0.05f;
            rect.x2 += 0.05f;
            if(rect.reverseflag) // 위로 이동
            {
                rect.y1 += 0.01f;
                rect.y2 += 0.01f;
            }
            else // 아래로 이동
            {
                rect.y1 -= 0.01f;
                rect.y2 -= 0.01f;
            }

            if (rect.x2 >= 1.0f) // 오른쪽 벽에 닿음
            {
                rect.dirX = LEFT;
                // x축 방향 전환 시 수직 이동
                if (rect.y1 >= 1.0f) // 상단에 닿음
                {
                    rect.reverseflag = false; // 아래로 이동
                 
                }
                else if (rect.y2 <= -1.0f) // 하단에 닿음
                {
                    rect.reverseflag = true; // 위로 이동
                  
                }
                
            }
        }
        else if (rect.dirX == LEFT)
        {
            rect.x1 -= 0.05f;
            rect.x2 -= 0.05f;
            if (rect.reverseflag) // 위로 이동
            {
                rect.y1 += 0.01f;
                rect.y2 += 0.01f;
            }
            else // 아래로 이동
            {
                rect.y1 -= 0.01f;
                rect.y2 -= 0.01f;
            }

            if (rect.x1 <= -1.0f) // 왼쪽 벽에 닿음
            {
                rect.dirX = RIGHT;
                // x축 방향 전환 시 수직 이동
                if (rect.y1 >= 1.0f) // 상단에 닿음
                {
                    rect.reverseflag = false; // 아래로 이동
                  
                }
                else if (rect.y2 <= -1.0f) // 하단에 닿음
                {
                    rect.reverseflag = true; // 위로 이동
                   
                }
               
            }
        }
    }

    glutTimerFunc(100, ZigzagFunction, 1);
}

void SizeChangeFunction(int value)
{
    if (!isSize) return;

    for (auto& rect : rects)
    {
        int sizeChangeType = size_dist(mt);
        if (rect.x2 - rect.x1 >= 0.5f) value = -1;
        else if (rect.x2 - rect.x1 <= 0.1f) value = 1;
        float diff = (value == 1) ? 0.010f : -0.010f;
        float centerX = (rect.x1 + rect.x2) / 2.0f;
        float centerY = (rect.y1 + rect.y2) / 2.0f;
        float halfWidth = std::max(0.01f, ((rect.x2 - rect.x1) / 2.0f) + diff);
        float halfHeight = std::max(0.01f, ((rect.y2 - rect.y1) / 2.0f) + diff);
        if (sizeChangeType == 0) // 좌우
        {
            rect.x1 = centerX - halfWidth;
            rect.x2 = centerX + halfWidth;
        }
        else // 상하
        {
            rect.y1 = centerY - halfHeight;
            rect.y2 = centerY + halfHeight;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(100, SizeChangeFunction, value);
}

void ColorChangeFunction(int value)
{
    if (!isColor) return;

    glutPostRedisplay();
    for (auto& rect : rects)
    {
        rect.r = color_dist(mt) / 255.0f;
        rect.g = color_dist(mt) / 255.0f;
        rect.b = color_dist(mt) / 255.0f;
    }
    glutTimerFunc(100, ColorChangeFunction, 1);
}

void CopyMoveFunction(int value)
{
    if (!isCopy || rects.empty() || randomIndex < 0 || randomIndex >= rects.size()) return;

    glutPostRedisplay();

    // 리더 사각형의 움직임 업데이트 (Diagonal 또는 Zigzag에 따라)
    if (isDiagonal) {
        for (auto& rect : rects) {
            if (&rect == &rects[randomIndex]) { // 리더만 업데이트
                // 방향에 따른 이동
                if (rect.dirY == RIGHTUPDIG)
                {
                    rect.x1 += 0.05f;
                    rect.x2 += 0.05f;
                    rect.y1 += 0.05f;
                    rect.y2 += 0.05f;
                    if (rect.x2 >= 1.0f || rect.y2 >= 1.0f) rect.dirY = RIGHTDOWNDIG;
                }
                else if (rect.dirY == RIGHTDOWNDIG)
                {
                    rect.x1 += 0.05f;
                    rect.x2 += 0.05f;
                    rect.y1 -= 0.05f;
                    rect.y2 -= 0.05f;
                    if (rect.x2 >= 1.0f || rect.y1 <= -1.0f) rect.dirY = LEFTDOWNDIG;
                }
                else if (rect.dirY == LEFTDOWNDIG)
                {
                    rect.x1 -= 0.05f;
                    rect.x2 -= 0.05f;
                    rect.y1 -= 0.05f;
                    rect.y2 -= 0.05f;
                    if (rect.x1 <= -1.0f || rect.y1 <= -1.0f) rect.dirY = LEFTUPDIG;
                }
                else if (rect.dirY == LEFTUPDIG)
                {
                    rect.x1 -= 0.05f;
                    rect.x2 -= 0.05f;
                    rect.y1 += 0.05f;
                    rect.y2 += 0.05f;
                    if (rect.x1 <= -1.0f || rect.y2 >= 1.0f) rect.dirY = RIGHTUPDIG;
                }
            }
        }
    }
    else if (isZigzag) {
        for (auto& rect : rects) {
            if (&rect == &rects[randomIndex]) { // 리더만 업데이트
                if (rect.dirX == RIGHT) {
                    rect.x1 += 0.05f;
                    rect.x2 += 0.05f;
                    if (rect.x2 >= 1.0f) {
                        rect.dirX = LEFT;
                        if (rect.y1 >= 1.0f) {
                            rect.reverseflag = false;
                            rect.y1 -= 0.1f;
                            rect.y2 -= 0.1f;
                        }
                        else if (rect.y2 <= -1.0f) {
                            rect.reverseflag = true;
                            rect.y1 += 0.1f;
                            rect.y2 += 0.1f;
                        }
                        else {
                            if (!rect.reverseflag) {
                                rect.y1 -= 0.1f;
                                rect.y2 -= 0.1f;
                            }
                            else {
                                rect.y1 += 0.1f;
                                rect.y2 += 0.1f;
                            }
                        }
                    }
                }
                else if (rect.dirX == LEFT) {
                    rect.x1 -= 0.05f;
                    rect.x2 -= 0.05f;
                    if (rect.x1 <= -1.0f) {
                        rect.dirX = RIGHT;
                        if (rect.y1 >= 1.0f) {
                            rect.reverseflag = false;
                            rect.y1 -= 0.1f;
                            rect.y2 -= 0.1f;
                        }
                        else if (rect.y2 <= -1.0f) {
                            rect.reverseflag = true;
                            rect.y1 += 0.1f;
                            rect.y2 += 0.1f;
                        }
                        else {
                            if (!rect.reverseflag) {
                                rect.y1 -= 0.1f;
                                rect.y2 -= 0.1f;
                            }
                            else {
                                rect.y1 += 0.1f;
                                rect.y2 += 0.1f;
                            }
                        }
                    }
                }
            }
        }
    }

    // 리더 사각형의 현재 위치와 상태 저장
    Position leaderPos = { rects[randomIndex].x1, rects[randomIndex].y1,
                          rects[randomIndex].x2, rects[randomIndex].y2,
                          rects[randomIndex].dirX, rects[randomIndex].dirY,
                          rects[randomIndex].reverseflag };

    // 리더의 위치 히스토리에 추가
    if (rectHistories[randomIndex].size() >= rects.size()) {
        rectHistories[randomIndex].pop(); // 히스토리 크기 제한
    }
    rectHistories[randomIndex].push(leaderPos);

    // 따라가는 순서 정의: randomIndex -> (randomIndex-1) -> (randomIndex+1) -> ...
    std::vector<int> followOrder;
    followOrder.push_back(randomIndex); // 리더
    for (int i = 1; i < rects.size(); ++i) {
        int idx = (randomIndex - i + rects.size()) % rects.size();
        if (idx != randomIndex) {
            followOrder.push_back(idx);
        }
    }

    // 각 사각형의 위치와 상태 업데이트
    for (size_t i = 1; i < followOrder.size(); ++i) {
        int currIdx = followOrder[i];
        int prevIdx = followOrder[i - 1];

        // 이전 사각형의 히스토리에서 위치 가져오기
        if (!rectHistories[prevIdx].empty()) {
            Position prevPos = rectHistories[prevIdx].front();
            rects[currIdx].x1 = prevPos.x1;
            rects[currIdx].y1 = prevPos.y1;
            rects[currIdx].x2 = prevPos.x2;
            rects[currIdx].y2 = prevPos.y2;
            rects[currIdx].dirX = prevPos.dirX;
            rects[currIdx].dirY = prevPos.dirY;
            rects[currIdx].reverseflag = prevPos.reverseflag;

            // 현재 사각형의 히스토리 업데이트
            if (rectHistories[currIdx].size() >= rects.size()) {
                rectHistories[currIdx].pop();
            }
            rectHistories[currIdx].push(prevPos);
        }
    }

    glutTimerFunc(100, CopyMoveFunction, 1);
}