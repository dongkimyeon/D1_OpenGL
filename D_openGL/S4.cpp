#include "CommonInclude.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

enum Direction
{
    LEFT, RIGHT, UPDIG, DOWNDIG
};

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> color_dist(0, 255);
std::uniform_int_distribution<int> coord_dist(0, 800);
std::uniform_int_distribution<int> size_dist(0,1);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void DrawRects();
void Keyboard(unsigned char key, int x, int y);
void DiagonalFunction(int value);
void ZigzagFunction(int value);
void SizeChangeFunction(int value);
void ColorChangeFunction(int value);

struct Rect {
    float x1, y1, x2, y2;
    float r, g, b;
	Direction dirX = RIGHT; // X축 이동 방향
	Direction dirY = UPDIG; // Y축 이동 방향
};

std::vector<Rect> rects;
GLclampf bgColor[3] = {0.2f, 0.2f, 0.2f };
int selectedRectIndex = -1;
bool isDragging = false;
bool isColor = false;
bool isSize = false;
bool isDiagonal = false;
bool isZigzag = false;

void InitRects()
{
    //TODO

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
    case 'd':
    {
        //위치 변화1  사각형들은 각각 대각선으로 이동하고 벽에 닿으면 튕겨 다른 방향으로 이동한다/멈춘다
		isDiagonal = !isDiagonal;
		if (isDiagonal) isZigzag = false;
		glutTimerFunc(100, DiagonalFunction, 1);
        break;
    }
    case 'z':
    {
        //위치 변화2  사각형들이 지그재그로 이동한다/멈춘다.
		isZigzag = !isZigzag;
		if (isZigzag) isDiagonal = false;
		glutTimerFunc(100, ZigzagFunction, 1);
        break;  
    }
    case 'c':
    {
        //c: 크기 변화  사각형의 크기가 다양하게 변한다/변하지 않는다.
        isSize = !isSize;
		glutTimerFunc(100, SizeChangeFunction, 1);
        break;  
    }
    case 'o':
    {
        //o: 색상 변화  사각형의 색상이 다양하게 변한다/변하지 않는다.
		isColor = !isColor;
		glutTimerFunc(100, ColorChangeFunction, 1);
		break;
    }
    case 's':
    {
        //s: 모든 애니메이션이 멈춘다.
		isColor = false;
		isSize = false;
		isZigzag = false;
		isDiagonal = false;
        break;
    }
    case 'm':
    {
        //m: 처음 원래 사각형 위치로 이동한다.
        break;
	}
    case 'r':
    {
        //r: 모든 사각형이 삭제되고 처음 상태로 돌아간다.
        rects.clear();
        glutPostRedisplay();
        break;
	}
    case 'q':
    {
        //종료
        glutDestroyWindow(1);
        break;
    }
    default:
        break;
    }
}

void Mouse(int button, int state, int x, int y) {
    float ndc_x = (2.0f * x / WINDOW_WIDTH) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / WINDOW_HEIGHT);

   
    //좌클릭 누름 
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) 
    {
        if (rects.size() < 5)
        {
            Rect tempRect;
            float size = 0.25f;
            tempRect.x1 = ndc_x - size;
            tempRect.x2 = ndc_x + size;
            tempRect.y1 = ndc_y - size;
            tempRect.y2 = ndc_y + size;
            tempRect.r = color_dist(mt) / 255.0f;
            tempRect.g = color_dist(mt) / 255.0f;
            tempRect.b = color_dist(mt) / 255.0f;
            rects.push_back(tempRect);
        }
    }
    glutPostRedisplay();
}



void DiagonalFunction(int value)
{
	if (!isDiagonal) return;

    if (isDiagonal)
    {
		glutPostRedisplay();
        for (auto& rect : rects)
        {
			
            if (rect.dirY == UPDIG)
            {
                rect.x1 += 0.05f;
                rect.x2 += 0.05f;
				rect.y1 += 0.05f;
				rect.y2 += 0.05f;

                if (rect.x2 >= 1.0f || rect.y2 >= 1.0f) rect.dirY = DOWNDIG;
            }
            else if (rect.dirY == DOWNDIG)
            {
                rect.x1 -= 0.05f;
                rect.x2 -= 0.05f;
				rect.y1 -= 0.05f;
				rect.y2 -= 0.05f;

                if (rect.x1 <= -1.0f || rect.y1 <= -1.0f) rect.dirY = UPDIG;
            }
		}
    }
	glutTimerFunc(100, DiagonalFunction, 1);
}

void ZigzagFunction(int value)
{
	if (!isZigzag) return;

    if (isZigzag)
    {
        glutPostRedisplay();
        //TODO
        for (auto& rect : rects)
        {
            if(rect.dirX == RIGHT)
            {
                rect.x1 += 0.05f;
                rect.x2 += 0.05f;
                if (rect.x2 >= 1.0f) rect.dirX = LEFT;
            }
			else if (rect.dirX == LEFT)
            {
                rect.x1 -= 0.05f;
                rect.x2 -= 0.05f;
                if (rect.x1 <= -1.0f) rect.dirX = RIGHT;
			}
        }

	}
	glutTimerFunc(100, ZigzagFunction, 1);
}

void SizeChangeFunction(int value)
{
    //좌우 상하 크기 변경
    if (!isSize) return;

    for (auto& rect : rects)
    {
        int sizeChangeType = size_dist(mt);
        if(rect.x2 - rect.x1 >= 0.5f) value = -1;
		else if (rect.x2 - rect.x1 <= 0.1f) value = 1;
        float diff = (value == 1) ? 0.010f : -0.010f; 
        float centerX = (rect.x1 + rect.x2) / 2.0f;
        float centerY = (rect.y1 + rect.y2) / 2.0f;
        float halfWidth = std::max(0.01f, ((rect.x2 - rect.x1) / 2.0f) + diff);
        float halfHeight = std::max(0.01f, ((rect.y2 - rect.y1) / 2.0f) + diff);
        //rect.x1 = centerX - halfWidth;
        if(sizeChangeType == 0) //좌우
        {
            rect.x1 = centerX - halfWidth;
            rect.x2 = centerX + halfWidth;
        }
        else //상하
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
    if (isColor)
    {
        glutPostRedisplay();

        for (auto& rect : rects)
        {
            rect.r = color_dist(mt) / 255.0f;
            rect.g = color_dist(mt) / 255.0f;
            rect.b = color_dist(mt) / 255.0f;
        }

        glutTimerFunc(100, ColorChangeFunction, 1);
    }
   
}
