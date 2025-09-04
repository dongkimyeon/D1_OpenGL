#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <random>

#define WINDOW_WIDTH 800
#define WINDOW_HIGHT 800

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> color_dist(0, 255); 

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void DrawRects();

struct Rect {
    float x1, y1, x2, y2;
    float r, g, b;       
};

Rect rects[4];      
GLclampf bgColor[3] = { 0.0f, 0.0f, 0.0f };

void InitRects() {
   
    // 0: 좌측 하단 (-1,-1) ~ (0,0), 빨강
    rects[0] = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
    // 1: 좌측 상단 (-1,0) ~ (0,1), 초록
    rects[1] = { -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    // 2: 우측 하단 (0,-1) ~ (1,0), 파랑
    rects[2] = { 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    // 3: 우측 상단 (0,0) ~ (1,1), 노랑
    rects[3] = { 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HIGHT);
    glutCreateWindow("실습2");

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
    for (int i = 0; i < 4; ++i) {
        glColor3f(rects[i].r, rects[i].g, rects[i].b);
        glRectf(rects[i].x1, rects[i].y1, rects[i].x2, rects[i].y2);
    }
}

void AdjustRectSize(int index, float scaleFactor) {
   
    float centerX = (rects[index].x1 + rects[index].x2) / 2.0f;
    float centerY = (rects[index].y1 + rects[index].y2) / 2.0f;
    float halfWidth = (rects[index].x2 - rects[index].x1) / 2.0f * scaleFactor;
    float halfHeight = (rects[index].y2 - rects[index].y1) / 2.0f * scaleFactor;

    rects[index].x1 = centerX - halfWidth;
    rects[index].x2 = centerX + halfWidth;
    rects[index].y1 = centerY - halfHeight;
    rects[index].y2 = centerY + halfHeight;
}

void Mouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return; 

  
    float ndc_x = (2.0f * x / WINDOW_WIDTH) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / WINDOW_HIGHT); // y축 뒤집기

    std::cout << "Click: button=" << button << ", ndc_x=" << ndc_x << ", ndc_y=" << ndc_y << std::endl;

 
    int hitIndex = -1; // -1: 외부
  
    for (int i = 0; i < 4; ++i) {
        if (ndc_x > rects[i].x1 && ndc_x < rects[i].x2 &&
            ndc_y > rects[i].y1 && ndc_y < rects[i].y2)
        {
            hitIndex = i;
            break;
        }
    }

    int hitQuadrant = -1; // -1: 해당없음
    if (ndc_x > 0.0f && ndc_y > 0.0f) {
        hitQuadrant = 4; 
    }
    else if (ndc_x < 0.0f && ndc_y > 0.0f) {
        hitQuadrant = 2;
    }
    else if (ndc_x < 0.0f && ndc_y < 0.0f) {
        hitQuadrant = 1; 
    }
    else if (ndc_x > 0.0f && ndc_y < 0.0f) {
        hitQuadrant = 3; 
    }

    if (button == GLUT_LEFT_BUTTON) {
     
        if (hitIndex != -1) {
            // 내부: 해당 사각형 색상 랜덤 변경
            rects[hitIndex].r = color_dist(mt) / 255.0f;
            rects[hitIndex].g = color_dist(mt) / 255.0f;
            rects[hitIndex].b = color_dist(mt) / 255.0f;
        }
        else {
            // 외부: 배경색 랜덤 변경
            bgColor[0] = color_dist(mt) / 255.0f;
            bgColor[1] = color_dist(mt) / 255.0f;
            bgColor[2] = color_dist(mt) / 255.0f;
        }
    }
    else if (button == GLUT_RIGHT_BUTTON) {
      
        if (hitIndex != -1) {
            // 내부: 해당 사각형 크기 축소 (0.9배)
            AdjustRectSize(hitIndex, 0.9f);
        }
        else if (hitQuadrant != -1) {
            // 외부: 해당 사분면의 사각형 크기 확대 (1.1배)
            int rectIndex = hitQuadrant - 1;
            if (rectIndex >= 0 && rectIndex < 4) 
            {
                AdjustRectSize(rectIndex, 1.1f);
            }
        }
    }
    glutPostRedisplay(); // 화면 갱신
}