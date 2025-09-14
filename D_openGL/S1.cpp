#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")


#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <random>

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int>color(0, 255);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLclampf Color[3] = { 0,0,0 };
bool b_TimerState = false;

void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void Keyboard(unsigned char key, int x, int y);

void TimerFunction(int value);
int main(int argc, char** argv) {
    // GLUT 초기화
    glutInit(&argc, argv); // 추가
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("실습1");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "GLEW Initialized\n";
    }
    
    // 콜백 함수 설정
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    //glutMouseFunc(Mouse);
    //glutPassiveMotionFunc(Motion);
   
    glutMainLoop();
    return 0;
}

GLvoid drawScene() {
    glClearColor(Color[0], Color[1], Color[2], 1.0f); // 바탕색을 파란색으로
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        std::cout << "x = " << x << " y = " << y << std::endl;
}

void Motion(int x, int y)
{

}

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'c':
        std::cout << "c" << std::endl;
        Color[0] = 0;
        Color[1] = 255;
        Color[2] = 255;
        break;
    case 'm':
        Color[0] = 255;
        Color[1] = 0;
        Color[2] = 255;
        break;
    case 'y':
        Color[0] = 255;
        Color[1] = 255;
        Color[2] = 0;
        break;
    case 'r':
        Color[0] = color(mt) / 255.0f;
        Color[1] = color(mt) / 255.0f;
        Color[2] = color(mt) / 255.0f;
        break;
    case 'w':
        Color[0] = 255;
        Color[1] = 255;
        Color[2] = 255;
        break;
    case 'b':
        Color[0] = 0;
        Color[1] = 0;
        Color[2] = 0;
        break;  
    case 't':
        b_TimerState = true;
        glutTimerFunc(100, TimerFunction, 1);
        //TODO
        break;
    case 's':
        b_TimerState = false;
        //TODO
        break;
    case 'q':
        glutDestroyWindow(1);
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

void TimerFunction(int value)
{
    if (b_TimerState)
    {
        glutPostRedisplay();
        Color[0] = color(mt) / 255.0f;
        Color[1] = color(mt) / 255.0f;
        Color[2] = color(mt) / 255.0f;

        glutTimerFunc(100, TimerFunction, 1);
    }
   
}

