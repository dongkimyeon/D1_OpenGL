#include "CommonInclude.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> color_dist(0, 255);
std::uniform_int_distribution<int> coord_dist(0, 1000);

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
  
    Rect temp = { -0.25f, -0.25f, 0.25f, 0.25f, 1.0f, 0.0f, 0.0f };
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
	glutTimerFunc(100, TimerFunc, 1);
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
        if (rects.size() < 5) {
            Rect tempRect;
            float x = (coord_dist(mt) / 1000.0f) * 2.0f - 1.0f;
            float y = (coord_dist(mt) / 1000.0f) * 2.0f - 1.0f;
            float size = 0.25f;
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
    glutPostRedisplay();
}

void Motion(int x, int y) 
{
    //충돌체크 후 해당 사각형 크기 변경 및 색상 변경
    if (isDragging && selectedRectIndex != -1) 
    {
       
        float ndc_x = (2.0f * x / 1000.0f) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / 1000.0f); 

        
        float width = rects[selectedRectIndex].x2 - rects[selectedRectIndex].x1;
        float height = rects[selectedRectIndex].y2 - rects[selectedRectIndex].y1;

     
        rects[selectedRectIndex].x1 = ndc_x - width / 2.0f;
        rects[selectedRectIndex].x2 = ndc_x + width / 2.0f;
        rects[selectedRectIndex].y1 = ndc_y - height / 2.0f;
        rects[selectedRectIndex].y2 = ndc_y + height / 2.0f;

        glutPostRedisplay();
    }
}

void TimerFunc(int value)
{
    //충돌체크
    for (auto it = rects.begin(); it != rects.end();)
    {
        //움직이는 사각형과 배경 사각형과 충돌체크해서 사이즈변경

    }


	glutTimerFunc(100, TimerFunc, 1);
}
