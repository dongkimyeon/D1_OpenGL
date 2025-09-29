#include "CommonInclude.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> color_dist(0, 255);
std::uniform_int_distribution<int> coord_dist(0, 800);
std::uniform_int_distribution<int> rect_dist(20, 40);
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void DrawRects();
void Keyboard(unsigned char key, int x, int y);
void Motion(int x, int y);

struct Rect {
    float x1, y1, x2, y2;
    float r, g, b;
};

std::vector<Rect> rects;
GLclampf bgColor[3] = { 0.5f, 0.5f, 0.5f };

bool isDragging = false;
bool isLButtonDown = false;

float tempsize = 0.1;

int rectNum = rect_dist(mt);

void InitRects() {

	std::cout << "InitRects\n" << std::endl;
    //지우개 
    Rect temp = { -0.1f, -0.1f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f };
    rects.push_back(temp);
        
    //사각형 20개
    for (int i = 0; i < rectNum; ++i)
    {
        Rect tempRect;
        float x = (coord_dist(mt) / 800.0f) * 2.0f - 1.0f;
        float y = (coord_dist(mt) / 800.0f) * 2.0f - 1.0f;
        float size = 0.05f;
        tempRect.x1 = x - size;
        tempRect.x2 = x + size;
        tempRect.y1 = y - size;
        tempRect.y2 = y + size;
        tempRect.r = color_dist(mt) / 255.0f;
        tempRect.g = color_dist(mt) / 255.0f;
        tempRect.b = color_dist(mt) / 255.0f;
        rects.push_back(tempRect);

    }

   
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("실습5");
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

    if (isLButtonDown)
    {
        glColor3f(rects[0].r, rects[0].g, rects[0].b);
        glRectf(rects[0].x1, rects[0].y1, rects[0].x2, rects[0].y2);
    }
    for (auto it = rects.begin() + 1; it != rects.end(); )
    {
        glColor3f(it->r, it->g, it->b);
		glRectf(it->x1, it->y1, it->x2, it->y2);
		++it;
    }
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'r':
    {
        //reset 
        rectNum = rect_dist(mt);
        for (auto it = rects.begin() + 1; it != rects.end(); )
        {
            it = rects.erase(it);
        }
        // Reset eraser to original size
        rects[0] = { -0.1f, -0.1f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f };
        tempsize = 0.1f;
        for (int i = 0; i < rectNum; ++i)
        {
            Rect tempRect;
            float x = (coord_dist(mt) / 800.0f) * 2.0f - 1.0f;
            float y = (coord_dist(mt) / 800.0f) * 2.0f - 1.0f;
            float size = 0.05f;
            tempRect.x1 = x - size;
            tempRect.x2 = x + size;
            tempRect.y1 = y - size;
            tempRect.y2 = y + size;
            tempRect.r = color_dist(mt) / 255.0f;
            tempRect.g = color_dist(mt) / 255.0f;
            tempRect.b = color_dist(mt) / 255.0f;
            rects.push_back(tempRect);
        }
        glutPostRedisplay();
        break;
    }
    default:
        break;
    }
}

void Mouse(int button, int state, int x, int y) {
    float ndc_x = (2.0f * x / WINDOW_WIDTH) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / WINDOW_HEIGHT);

    if (button == GLUT_LEFT_BUTTON) 
    {
        if (state == GLUT_DOWN) {
			isLButtonDown = true;
            
			float size = tempsize / 2.0f;
            rects[0].x1 = ndc_x - size;
			rects[0].x2 = ndc_x + size;
			rects[0].y1 = ndc_y - size;
            rects[0].y2 = ndc_y + size;

			isDragging = true;
         
        }
        else if (state == GLUT_UP) 
        {
            rects[0] = { -0.1f, -0.1f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f };
			isLButtonDown = false;
            isDragging = false;

			//tempsize = rects[0].x2 - rects[0].x1;
			rects[0].r = 0.0f;
			rects[0].g = 0.0f;
			rects[0].b = 0.0f;

            
        }
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && rects.size() <= rectNum)
    {
        Rect tempRect;
        float size = 0.05f;
        tempRect.x1 = ndc_x - size;
        tempRect.x2 = ndc_x + size;
        tempRect.y1 = ndc_y - size;
        tempRect.y2 = ndc_y + size;
        tempRect.r = color_dist(mt) / 255.0f;
        tempRect.g = color_dist(mt) / 255.0f;
        tempRect.b = color_dist(mt) / 255.0f;
        rects.push_back(tempRect);

        if (tempsize >= 0.1f)
        {
            tempsize -= 0.05f;
        }
			
		
    }
    glutPostRedisplay();
}

void Motion(int x, int y) {
    if (isDragging) {
        float ndc_x = (2.0f * x / WINDOW_WIDTH) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / WINDOW_HEIGHT);


        float width = rects[0].x2 - rects[0].x1;
        float height = rects[0].y2 - rects[0].y1;


        rects[0].x1 = ndc_x - width / 2.0f;
        rects[0].x2 = ndc_x + width / 2.0f;
        rects[0].y1 = ndc_y - height / 2.0f;
        rects[0].y2 = ndc_y + height / 2.0f;


        //충돌체크 

        for(auto it = rects.begin() + 1; it != rects.end(); )
        {
            if (!(rects[0].x2 < it->x1 || rects[0].x1 > it->x2 || rects[0].y2 < it->y1 || rects[0].y1 > it->y2))
            {
                
                rects[0].r = it->r;
                rects[0].g = it->g;
                rects[0].b = it->b;
                float size = 0.01f;
				rects[0].x1 = rects[0].x1 - size;
				rects[0].x2 = rects[0].x2 + size;
				rects[0].y1 = rects[0].y1 - size;
				rects[0].y2 = rects[0].y2 + size;

                it = rects.erase(it); 
            }
            else
            {
                ++it;
            }
		}
       

        glutPostRedisplay();
    }
}