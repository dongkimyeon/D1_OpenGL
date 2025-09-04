#include "CommonInclude.h"

using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> color_dist(0, 255);
std::uniform_int_distribution<int> coord_dist(0, 800);
std::uniform_int_distribution<int> state_dist(0, 3);
std::uniform_int_distribution<int> move_dist(0, 7);
enum MoveType
{
	LEFT, RIGHT, UP, DOWN,
	LEFTUP, LEFTDOWN, RIGHTUP, RIGHTDOWN, NONE  
};

enum State
{
	ONE, TWO, THREE, FOUR
    //ONE 좌우상하
	//TWO 대각선
	//THREE 한쪽방향
    //FOUR 8등분 좌우상하대각선   
};

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void DrawRects();
void Keyboard(unsigned char key, int x, int y);

void MoveRects(int value);

struct Rect {
    float x1, y1, x2, y2;
    float r, g, b;
    float size;
	MoveType moveType;
};

std::vector<Rect> movingRects;
std::vector<Rect> rects;
GLclampf bgColor[3] = { 0.5f, 0.5f, 0.5f };

void InitRects() {

    std::cout << "InitRects\n" << std::endl;
    for (int i = 0; i < 5; ++i)
    {
        Rect rect;
        rect.size = 0.2f;
        rect.x1 = coord_dist(mt) / 800.0f - rect.size;
        rect.y1 = rect.size - coord_dist(mt) / 800.0f;
        rect.x2 = rect.x1 + rect.size;
        rect.y2 = rect.y1 - rect.size;
        rect.r = color_dist(mt) / 255.0f;
        rect.g = color_dist(mt) / 255.0f;
        rect.b = color_dist(mt) / 255.0f;
        rect.moveType = NONE;
		rects.push_back(rect);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("실습6");
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

   
    for (auto it = rects.begin(); it != rects.end(); )
    {
        glColor3f(it->r, it->g, it->b);
        glRectf(it->x1, it->y1, it->x2, it->y2);
        ++it;
    }

    for (auto it = movingRects.begin(); it != movingRects.end(); )
    {
        glColor3f(it->r, it->g, it->b);       
        glRectf(it->x1, it->y1, it->x2, it->y2);
        ++it;
	}
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) 
    {
    default:
        break;
    }
}

void Mouse(int button, int state, int x, int y) {
    float ndc_x = (2.0f * x / WINDOW_WIDTH) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / WINDOW_HEIGHT);
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        for (auto it = rects.begin(); it != rects.end(); ) 
        {
            if (ndc_x >= it->x1 && ndc_x <= it->x2 && ndc_y <= it->y1 && ndc_y >= it->y2)
            {
                int StateType = state_dist(mt);
                //좌우상하
                if (StateType == ONE)
                {
                    float half_size = it->size / 2.0f;
                    Rect rect1, rect2, rect3, rect4;

                    // 첫 번째 사각형 (좌상, LEFT 방향)
                    rect1.size = half_size;
                    rect1.x1 = it->x1;
                    rect1.y1 = it->y1;
                    rect1.x2 = rect1.x1 + half_size;
                    rect1.y2 = rect1.y1 - half_size;
                    rect1.r = it->r;
                    rect1.g = it->g;
                    rect1.b = it->b;
                    rect1.moveType = LEFT;

                    // 두 번째 사각형 (좌하, DOWN 방향)
                    rect2.size = half_size;
                    rect2.x1 = it->x1;
                    rect2.y1 = it->y1 - half_size;
                    rect2.x2 = rect2.x1 + half_size;
                    rect2.y2 = rect2.y1 - half_size;
                    rect2.r = it->r;
                    rect2.g = it->g;
                    rect2.b = it->b;
                    rect2.moveType = DOWN;

                    // 세 번째 사각형 (우상, UP 방향)
                    rect3.size = half_size;
                    rect3.x1 = it->x1 + half_size;
                    rect3.y1 = it->y1;
                    rect3.x2 = rect3.x1 + half_size;
                    rect3.y2 = rect3.y1 - half_size;
                    rect3.r = it->r;
                    rect3.g = it->g;
                    rect3.b = it->b;
                    rect3.moveType = UP;

                    // 네 번째 사각형 (우하, RIGHT 방향)
                    rect4.size = half_size;
                    rect4.x1 = it->x1 + half_size;
                    rect4.y1 = it->y1 - half_size;
                    rect4.x2 = rect4.x1 + half_size;
                    rect4.y2 = rect4.y1 - half_size;
                    rect4.r = it->r;
                    rect4.g = it->g;
                    rect4.b = it->b;
                    rect4.moveType = RIGHT;


                    movingRects.push_back(rect1);
                    movingRects.push_back(rect2);
                    movingRects.push_back(rect3);
                    movingRects.push_back(rect4);

                    it = rects.erase(it);
                    glutTimerFunc(100, MoveRects, 0);
                }
                //대각선
                else if (StateType == TWO)
                {
                    float half_size = it->size / 2.0f;
                    Rect rect1, rect2, rect3, rect4;

                    // 첫 번째 사각형 (좌상, LEFT 방향)
                    rect1.size = half_size;
                    rect1.x1 = it->x1;
                    rect1.y1 = it->y1;
                    rect1.x2 = rect1.x1 + half_size;
                    rect1.y2 = rect1.y1 - half_size;
                    rect1.r = it->r;
                    rect1.g = it->g;
                    rect1.b = it->b;
                    rect1.moveType = LEFTUP;

                    // 두 번째 사각형 (좌하, DOWN 방향)
                    rect2.size = half_size;
                    rect2.x1 = it->x1;
                    rect2.y1 = it->y1 - half_size;
                    rect2.x2 = rect2.x1 + half_size;
                    rect2.y2 = rect2.y1 - half_size;
                    rect2.r = it->r;
                    rect2.g = it->g;
                    rect2.b = it->b;
                    rect2.moveType = LEFTDOWN;

                    // 세 번째 사각형 (우상, UP 방향)
                    rect3.size = half_size;
                    rect3.x1 = it->x1 + half_size;
                    rect3.y1 = it->y1;
                    rect3.x2 = rect3.x1 + half_size;
                    rect3.y2 = rect3.y1 - half_size;
                    rect3.r = it->r;
                    rect3.g = it->g;
                    rect3.b = it->b;
                    rect3.moveType = RIGHTUP;

                    // 네 번째 사각형 (우하, RIGHT 방향)
                    rect4.size = half_size;
                    rect4.x1 = it->x1 + half_size;
                    rect4.y1 = it->y1 - half_size;
                    rect4.x2 = rect4.x1 + half_size;
                    rect4.y2 = rect4.y1 - half_size;
                    rect4.r = it->r;
                    rect4.g = it->g;
                    rect4.b = it->b;
                    rect4.moveType = RIGHTDOWN;


                    movingRects.push_back(rect1);
                    movingRects.push_back(rect2);
                    movingRects.push_back(rect3);
                    movingRects.push_back(rect4);

                    it = rects.erase(it);
                    glutTimerFunc(100, MoveRects, 0);
                }
                //한쪽방향  
                else if (StateType == THREE)
                {
                    float half_size = it->size / 2.0f;
                    Rect rect1, rect2, rect3, rect4;
					MoveType _moveType = (MoveType)move_dist(mt);

                    // 첫 번째 사각형 (좌상, LEFT 방향)
                    rect1.size = half_size;
                    rect1.x1 = it->x1;
                    rect1.y1 = it->y1;
                    rect1.x2 = rect1.x1 + half_size;
                    rect1.y2 = rect1.y1 - half_size;
                    rect1.r = it->r;
                    rect1.g = it->g;
                    rect1.b = it->b;
                    rect1.moveType = _moveType;

                    // 두 번째 사각형 (좌하, DOWN 방향)
                    rect2.size = half_size;
                    rect2.x1 = it->x1;
                    rect2.y1 = it->y1 - half_size;
                    rect2.x2 = rect2.x1 + half_size;
                    rect2.y2 = rect2.y1 - half_size;
                    rect2.r = it->r;
                    rect2.g = it->g;
                    rect2.b = it->b;
                    rect2.moveType = _moveType;

                    // 세 번째 사각형 (우상, UP 방향)
                    rect3.size = half_size;
                    rect3.x1 = it->x1 + half_size;
                    rect3.y1 = it->y1;
                    rect3.x2 = rect3.x1 + half_size;
                    rect3.y2 = rect3.y1 - half_size;
                    rect3.r = it->r;
                    rect3.g = it->g;
                    rect3.b = it->b;
                    rect3.moveType = _moveType;

                    // 네 번째 사각형 (우하, RIGHT 방향)
                    rect4.size = half_size;
                    rect4.x1 = it->x1 + half_size;
                    rect4.y1 = it->y1 - half_size;
                    rect4.x2 = rect4.x1 + half_size;
                    rect4.y2 = rect4.y1 - half_size;
                    rect4.r = it->r;
                    rect4.g = it->g;
                    rect4.b = it->b;
                    rect4.moveType = _moveType;


                    movingRects.push_back(rect1);
                    movingRects.push_back(rect2);
                    movingRects.push_back(rect3);
                    movingRects.push_back(rect4);

                    it = rects.erase(it);
                    glutTimerFunc(100, MoveRects, 0);
                }
                //8등분
                else if (StateType == FOUR)
                {
                    float quarter_size = it->size / 2.0f; // 4등분 후 다시 반으로 나누기
                    Rect _rects[8];
                    // 1번 사각형 (좌상, LEFTUP)
                    _rects[0].size = quarter_size;
                    _rects[0].x1 = it->x1;
                    _rects[0].y1 = it->y1;
                    _rects[0].x2 = _rects[0].x1 + quarter_size;
                    _rects[0].y2 = _rects[0].y1 - quarter_size;
                    _rects[0].r = it->r;
                    _rects[0].g = it->g;
                    _rects[0].b = it->b;
                    _rects[0].moveType = LEFTUP;
                    // 2번 사각형 (좌하, LEFTDOWN)
                    _rects[1].size = quarter_size;
                    _rects[1].x1 = it->x1;
                    _rects[1].y1 = it->y1 - quarter_size;
                    _rects[1].x2 = _rects[1].x1 + quarter_size;
                    _rects[1].y2 = _rects[1].y1 - quarter_size;
                    _rects[1].r = it->r;
                    _rects[1].g = it->g;
                    _rects[1].b = it->b;
                    _rects[1].moveType = LEFTDOWN;
                    // 3번 사각형 (우상, RIGHTUP)
                    _rects[2].size = quarter_size;
                    _rects[2].x1 = it->x1 + quarter_size;
                    _rects[2].y1 = it->y1;
                    _rects[2].x2 = _rects[2].x1 + quarter_size;
                    _rects[2].y2 = _rects[2].y1 - quarter_size;
                    _rects[2].r = it->r;
                    _rects[2].g = it->g;
                    _rects[2].b = it->b;
                    _rects[2].moveType = RIGHTUP;
                    // 4번 사각형 (우하, RIGHTDOWN)
                    _rects[3].size = quarter_size;
                    _rects[3].x1 = it->x1 + quarter_size;
                    _rects[3].y1 = it->y1 - quarter_size;
                    _rects[3].x2 = _rects[3].x1 + quarter_size;
                    _rects[3].y2 = _rects[3].y1 - quarter_size;
                    _rects[3].r = it->r;
                    _rects[3].g = it->g;
                    _rects[3].b = it->b;
                    _rects[3].moveType = RIGHTDOWN;
                    // 5번 사각형 (좌상, LEFT)
                    _rects[4].size = quarter_size;
                    _rects[4].x1 = it->x1;
                    _rects[4].y1 = it->y1 - quarter_size / 2.0f;
                    _rects[4].x2 = _rects[4].x1 + quarter_size;
                    _rects[4].y2 = _rects[4].y1 - quarter_size;
                    _rects[4].r = it->r;
                    _rects[4].g = it->g;
                    _rects[4].b = it->b;
                    _rects[4].moveType = LEFT;
                    // 6번 사각형 (우상, RIGHT)
                    _rects[5].size = quarter_size;
                    _rects[5].x1 = it->x1 + quarter_size;
                    _rects[5].y1 = it->y1 - quarter_size / 2.0f;
                    _rects[5].x2 = _rects[5].x1 + quarter_size;
                    _rects[5].y2 = _rects[5].y1 - quarter_size;
                    _rects[5].r = it->r;
                    _rects[5].g = it->g;
                    _rects[5].b = it->b;
                    _rects[5].moveType = RIGHT;
                    // 7번 사각형 (좌하, DOWN)
                    _rects[6].size = quarter_size;
                    _rects[6].x1 = it->x1 + quarter_size / 2.0f;
                    _rects[6].y1 = it->y1 - quarter_size;
                    _rects[6].x2 = _rects[6].x1 + quarter_size;
                    _rects[6].y2 = _rects[6].y1 - quarter_size;
                    _rects[6].r = it->r;
                    _rects[6].g = it->g;
                    _rects[6].b = it->b;
                    _rects[6].moveType = DOWN;
                    // 8번 사각형 (우상, UP)
                    _rects[7].size = quarter_size;
                    _rects[7].x1 = it->x1 + quarter_size / 2.0f;
                    _rects[7].y1 = it->y1;
                    _rects[7].x2 = _rects[7].x1 + quarter_size;
                    _rects[7].y2 = _rects[7].y1 - quarter_size;
                    _rects[7].r = it->r;
                    _rects[7].g = it->g;
                    _rects[7].b = it->b;
                    _rects[7].moveType = UP;
                    for (int i = 0; i < 8; ++i)
                    {
                        movingRects.push_back(_rects[i]);
                    }
                    it = rects.erase(it);
                    glutTimerFunc(100, MoveRects, 0);
                }
            }
            else {
                ++it;
            }
        }
    }
    glutPostRedisplay();
}

void MoveRects(int value)
{
    for (auto it = movingRects.begin(); it != movingRects.end(); ) {
        // 크기 감소
        it->size -= 0.005f;

        // 크기가 0 이하가 되면 제거
        if (it->size <= 0.0f) {
            it = movingRects.erase(it);
            continue;
        }

        // 위치 업데이트 (이동 방향에 따라)
        if (it->moveType == LEFT) {
            it->x1 -= 0.01f;
            it->x2 = it->x1 + it->size;
            it->y2 = it->y1 - it->size;
        }
        else if (it->moveType == RIGHT) {
            it->x1 += 0.01f;
            it->x2 = it->x1 + it->size;
            it->y2 = it->y1 - it->size;
        }
        else if (it->moveType == UP) {
            it->y1 += 0.01f;
            it->x2 = it->x1 + it->size;
            it->y2 = it->y1 - it->size;
        }
        else if (it->moveType == DOWN) {
            it->y1 -= 0.01f;
            it->x2 = it->x1 + it->size;
            it->y2 = it->y1 - it->size;
        }
        else if (it->moveType == LEFTUP) {
            it->x1 -= 0.01f;
            it->y1 += 0.01f;
            it->x2 = it->x1 + it->size;
            it->y2 = it->y1 - it->size;
        }
        else if (it->moveType == LEFTDOWN) {
            it->x1 -= 0.01f;
            it->y1 -= 0.01f;
            it->x2 = it->x1 + it->size;
            it->y2 = it->y1 - it->size;
        }
        else if (it->moveType == RIGHTUP) {
            it->x1 += 0.01f;
            it->y1 += 0.01f;
            it->x2 = it->x1 + it->size;
            it->y2 = it->y1 - it->size;
        }
        else if (it->moveType == RIGHTDOWN) {
            it->x1 += 0.01f;
            it->y1 -= 0.01f;
            it->x2 = it->x1 + it->size;
            it->y2 = it->y1 - it->size;
        }

        ++it;
    }

    glutPostRedisplay();
    glutTimerFunc(100, MoveRects, 0); // 다음 타이머 호출
}
