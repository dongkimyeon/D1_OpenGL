#include "CommonInclude.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> color_dist(0, 255);
std::uniform_int_distribution<int> coord_dist(0, 1000);
std::uniform_real_distribution<float> size_dist(0.1f, 0.25f); 

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void DrawRects();
void Keyboard(unsigned char key, int x, int y);
void Motion(int x, int y);
void TimerFunc(int value);

struct Rect 
{
    float x1, y1, x2, y2;
    float r, g, b;
    float size;
};

std::vector<Rect> rects;
GLclampf bgColor[3] = { 0.0f, 0.0f, 0.0f };

int selectedRectIndex = -1;
bool isDragging = false;

void InitRects() {
  
    Rect temp = { -0.1f, -0.1f, 0.1f, 0.1f, 1.0f, 0.0f, 0.0f };
    rects.push_back(temp);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("실습3");
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
    glutMotionFunc(Motion);
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

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
        if (rects.size() < 10) {
            Rect tempRect;
            float x = (coord_dist(mt) / 1000.0f) * 2.0f - 1.0f;
            float y = (coord_dist(mt) / 1000.0f) * 2.0f - 1.0f;
			float size = size_dist(mt);
            tempRect.x1 = x - size;
            tempRect.x2 = x + size;
            tempRect.y1 = y - size;
            tempRect.y2 = y + size;
            tempRect.r = color_dist(mt) / 255.0f;
            tempRect.g = color_dist(mt) / 255.0f;
            tempRect.b = color_dist(mt) / 255.0f;
            rects.push_back(tempRect);
            glutPostRedisplay();
        }
        break;
    default:
        break;
    }
}

void Mouse(int button, int state, int x, int y) {
    float ndc_x = (2.0f * x / 1000.0f) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / 1000.0f);

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            selectedRectIndex = -1;
            for (int i = rects.size() - 1; i >= 0; --i) {
                if (ndc_x > rects[i].x1 && ndc_x < rects[i].x2 &&
                    ndc_y > rects[i].y1 && ndc_y < rects[i].y2) {
                    selectedRectIndex = i;
                    isDragging = true;
                    break;
                }
            }
        }
        else if (state == GLUT_UP)
        {
            isDragging = false;
            selectedRectIndex = -1;
        }
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        // 오른쪽 클릭으로 사각형을 누르면 두 개로 분리
        for (int i = rects.size() - 1; i >= 0; --i) {
            if (ndc_x > rects[i].x1 && ndc_x < rects[i].x2 &&
                ndc_y > rects[i].y1 && ndc_y < rects[i].y2)
            {
                if (rects.size() < 30) // 최대 10개
                {
                    // 원래 사각형의 크기
                    float halfWidth = (rects[i].x2 - rects[i].x1) / 4.0f;
                    float halfHeight = (rects[i].y2 - rects[i].y1) / 4.0f;

                    // 첫 번째 새 사각형: 랜덤 좌표
                    Rect newRect1;
                    float x1 = (coord_dist(mt) / 1000.0f) * 2.0f - 1.0f;
                    float y1 = (coord_dist(mt) / 1000.0f) * 2.0f - 1.0f;
                    newRect1.x1 = x1 - halfWidth;
                    newRect1.x2 = x1 + halfWidth;
                    newRect1.y1 = y1 - halfHeight;
                    newRect1.y2 = y1 + halfHeight;
                    newRect1.r = color_dist(mt) / 255.0f;
                    newRect1.g = color_dist(mt) / 255.0f;
                    newRect1.b = color_dist(mt) / 255.0f;

                    // 두 번째 새 사각형: 랜덤 좌표
                    Rect newRect2;
                    float x2 = (coord_dist(mt) / 1000.0f) * 2.0f - 1.0f;
                    float y2 = (coord_dist(mt) / 1000.0f) * 2.0f - 1.0f;
                    newRect2.x1 = x2 - halfWidth;
                    newRect2.x2 = x2 + halfWidth;
                    newRect2.y1 = y2 - halfHeight;
                    newRect2.y2 = y2 + halfHeight;
                    newRect2.r = color_dist(mt) / 255.0f;
                    newRect2.g = color_dist(mt) / 255.0f;
                    newRect2.b = color_dist(mt) / 255.0f;

                    // 새 사각형 추가 및 원래 사각형 제거
                    rects.push_back(newRect1);
                    rects.push_back(newRect2);
                    rects.erase(rects.begin() + i);
                }
                break;
            }
        }
    }
    glutPostRedisplay();
}
void Motion(int x, int y)
{
    if (isDragging && selectedRectIndex >= 0 && selectedRectIndex < static_cast<int>(rects.size()))
    {
        float ndc_x = (2.0f * x / 1000.0f) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / 1000.0f);

        // 사각형 이동
        float width = rects[selectedRectIndex].x2 - rects[selectedRectIndex].x1;
        float height = rects[selectedRectIndex].y2 - rects[selectedRectIndex].y1;

        rects[selectedRectIndex].x1 = ndc_x - width / 2.0f;
        rects[selectedRectIndex].x2 = ndc_x + width / 2.0f;
        rects[selectedRectIndex].y1 = ndc_y - height / 2.0f;
        rects[selectedRectIndex].y2 = ndc_y + height / 2.0f;

        // 충돌체크 및 병합
        int currentIndex = 0;
        for (auto it = rects.begin(); it != rects.end(); )
        {
            if (currentIndex != selectedRectIndex) 
            {
                // 충돌 확인
                if (!(rects[selectedRectIndex].x2 < it->x1 || rects[selectedRectIndex].x1 > it->x2 ||
                    rects[selectedRectIndex].y2 < it->y1 || rects[selectedRectIndex].y1 > it->y2))
                {
                    // 새로운 사각형의 경계 계산
                    rects[selectedRectIndex].x1 = std::min(rects[selectedRectIndex].x1, it->x1);
                    rects[selectedRectIndex].y1 = std::min(rects[selectedRectIndex].y1, it->y1);
                    rects[selectedRectIndex].x2 = std::max(rects[selectedRectIndex].x2, it->x2);
                    rects[selectedRectIndex].y2 = std::max(rects[selectedRectIndex].y2, it->y2);

                    // 랜덤 색상 설정
                    rects[selectedRectIndex].r = color_dist(mt) / 255.0f;
                    rects[selectedRectIndex].g = color_dist(mt) / 255.0f;
                    rects[selectedRectIndex].b = color_dist(mt) / 255.0f;

                    // 겹친 사각형 제거
                    it = rects.erase(it);

                    // 인덱스 조정
                    if (currentIndex < selectedRectIndex)
                    {
                        --selectedRectIndex;
                    }

                    // 벡터 크기 확인
                    if (selectedRectIndex >= static_cast<int>(rects.size()))
                    {
                        isDragging = false;
                        selectedRectIndex = -1;
                        break;
                    }
                }
                else
                {
                    ++it;
                    ++currentIndex;
                }
            }
            else
            {
                ++it;
                ++currentIndex;
            }
        }
        glutPostRedisplay();
    }
}