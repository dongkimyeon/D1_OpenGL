#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h"
#define M_PI 3.14159265358979323846
#include <cmath>


struct Shape {
    std::vector<GLfloat> vertices; // x, y, r, g, b
    std::vector<GLuint> indices;   // 인덱스 데이터   
    float centerX, centerY; // 도형의 중심 좌표
    
    float rotation;
    float scale;
};

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void Motion(int x, int y);
void UpdateMovement();
void Timer(int value);
void InitShape();
void UpdateShapeBuffer(int index);

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO[2], VBO[2], EBO[2];

Shape mShape1;
Shape mShape2;

bool isDragging = false;
int selectedShapeIndex = -1;
bool animationRunning = true;
int rotation_dir = 0;
float rotationSpeed = 2.0f; // radians per second
float prev_ndc_x, prev_ndc_y;
float select_radius = 0.25f;
bool doScaling = false;
float animationTime = 0.0f;
float scalePeriod = 2.0f;
float scaleAmplitude = 0.5f;
int ShapeNumber = 0; // 0: shape1, 1: shape2


bool rflag = false;
float new_center_x = 0.1f;
float new_center_y = 0.1f;
int main(int argc, char** argv) {
    srand(static_cast<unsigned>(time(0))); // 랜덤 시드 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("S14");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        return -1;
    }
    InitShape();

    MakeVertexShaders();
    MakeFragmentShaders();
    shaderProgramID = MakeShaderProgram();
    if (shaderProgramID == 0) {
        std::cerr << "셰이더 프로그램 생성 실패" << std::endl;
        return -1;
    }
    InitBuffers();

    UpdateShapeBuffer(0);
    UpdateShapeBuffer(1);

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutMotionFunc(Motion);
    glutMouseFunc(Mouse);

    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, Timer, 0); // 약 60 FPS
    glutMainLoop();
    return 0;
}

void MakeVertexShaders() {
    GLchar* vertexSource = filetobuf("vertex.glsl");
    if (!vertexSource) {
        std::cerr << "ERROR: vertex.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(vertexSource);
}

void MakeFragmentShaders() {
    GLchar* fragmentSource = filetobuf("fragment.glsl");
    if (!fragmentSource) {
        std::cerr << "ERROR: fragment.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(fragmentSource);
}

GLuint MakeShaderProgram() {
    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);

    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderID);
    return shaderID;
}

void InitBuffers() {
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);

    for (int i = 0; i < 2; ++i) {
        Shape* sh = (i == 0 ? &mShape1 : &mShape2);
        glBindVertexArray(VAO[i]);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sh->vertices.size() * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sh->indices.size() * sizeof(GLuint), sh->indices.data(), GL_STATIC_DRAW);

        const GLsizei stride = 5 * sizeof(GLfloat);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
}

void UpdateShapeBuffer(int index) {
    Shape* sh = (index == 0 ? &mShape1 : &mShape2);
    GLuint vbo = VBO[index];

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    std::vector<GLfloat> transformed(sh->vertices.size());
    for (size_t j = 0; j < sh->vertices.size(); j += 5) {
        float local_x;
        float local_y;
        if(rflag)
        {
            local_x = sh->vertices[j] + 0.2f;
            local_y = sh->vertices[j+1] + 0.5f;
        }
        else
        {
             local_x = sh->vertices[j];
             local_y= sh->vertices[j + 1];
        }

        // Scale
        float scaled_x = local_x * sh->scale;
        float scaled_y = local_y * sh->scale;

        // Rotate around local center (0,0)
        float cos_a = static_cast<float>(cos(sh->rotation));
        float sin_a = static_cast<float>(sin(sh->rotation));
        float rot_x = scaled_x * cos_a - scaled_y * sin_a;
        float rot_y = scaled_x * sin_a + scaled_y * cos_a;

        // Translate by center
        float final_x = rot_x + sh->centerX;
        float final_y = rot_y + sh->centerY;

        transformed[j] = final_x;
        transformed[j + 1] = final_y;
        transformed[j + 2] = sh->vertices[j + 2];
        transformed[j + 3] = sh->vertices[j + 3];
        transformed[j + 4] = sh->vertices[j + 4];
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, transformed.size() * sizeof(GLfloat), transformed.data());
}

void DrawScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgramID);
        
    if (ShapeNumber == 3)
    {
       for (int i = 0; i < 2; ++i) {
            glBindVertexArray(VAO[i]);
            Shape* sh = (i == 0 ? &mShape1 : &mShape2);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sh->indices.size()), GL_UNSIGNED_INT, 0);
	   }
    }
    else
    {
        glBindVertexArray(VAO[ShapeNumber]);
        Shape* sh = (ShapeNumber == 0 ? &mShape1 : &mShape2);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sh->indices.size()), GL_UNSIGNED_INT, 0);

    }
       
    glBindVertexArray(0);
    glutSwapBuffers();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

void Mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        float win_w = static_cast<float>(glutGet(GLUT_WINDOW_WIDTH));
        float win_h = static_cast<float>(glutGet(GLUT_WINDOW_HEIGHT));
        float ndc_x = (2.0f * x / win_w) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / win_h);

        if (state == GLUT_DOWN) {
            selectedShapeIndex = -1;
            for (int i = 0; i < 2; ++i) {
                Shape* sh = (i == 0 ? &mShape1 : &mShape2);
                float dx = ndc_x - sh->centerX;
                float dy = ndc_y - sh->centerY;
                if (dx * dx + dy * dy < select_radius * select_radius) {
                    selectedShapeIndex = i;
                    isDragging = true;
                    prev_ndc_x = ndc_x;
                    prev_ndc_y = ndc_y;
                    break;
                }
            }
        }
        else if (state == GLUT_UP) {
            isDragging = false;
        }
    }
}

void Motion(int x, int y)
{
    if (isDragging && selectedShapeIndex >= 0) {
        float ndc_x = (2.0f * x / width) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / height);

        float delta_x = ndc_x - prev_ndc_x;
        float delta_y = ndc_y - prev_ndc_y;

        Shape* sh = (selectedShapeIndex == 0 ? &mShape1 : &mShape2);
        sh->centerX += delta_x;
        sh->centerY += delta_y;

        UpdateShapeBuffer(selectedShapeIndex);
        glutPostRedisplay();

        prev_ndc_x = ndc_x;
        prev_ndc_y = ndc_y;
    }
}


void Keyboard(unsigned char key, int x, int y) {
    switch (key)
    {
    case '1':
		ShapeNumber = 0;
        break;
	case '2':
		ShapeNumber = 1;
        break;  
	case '3':
        ShapeNumber = 3;
		break;
    case 'c':
        rotation_dir = 1;
        mShape1.centerX = 0.0f;
		mShape1.centerY = 0.0f;
		mShape2.centerX = 0.0f;
		mShape2.centerY = 0.0f;
        animationRunning = true;
        break;
    case 't':
        rotation_dir = -1;
        mShape1.centerX = 0.0f;
        mShape1.centerY = 0.0f;
        mShape2.centerX = 0.0f;
        mShape2.centerY = 0.0f;
        animationRunning = true;
        break;
    case 's':
        animationRunning = !animationRunning;
        break;
    case 'x': // 삼각형의 크기 변화
    {
        bool wasOff = !doScaling;
        doScaling = !doScaling;
        if (wasOff) {
            animationTime = 0.0f;
        }
        if (!doScaling) {
            mShape1.scale = 1.0f;
            mShape2.scale = 1.0f;
            UpdateShapeBuffer(0);
            UpdateShapeBuffer(1);
            glutPostRedisplay();
        }
    }
    break;
    case 'r': //회전 중심점을 다른 점으로 변경
		rflag = !rflag;
        break;
    case 'q':
        exit(0);
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void Timer(int value)
{
    if (animationRunning && rotation_dir != 0) {
        float delta = 16.0f / 1000.0f;
        animationTime += delta;

        // Rotate both shapes
        mShape1.rotation += static_cast<float>(rotation_dir) * rotationSpeed * delta;
        mShape2.rotation += static_cast<float>(rotation_dir) * rotationSpeed * delta;

        if (doScaling) {
            float current_scale = 1.0f + scaleAmplitude * sin(animationTime / scalePeriod * 2.0f * M_PI);
            mShape1.scale = current_scale;
            mShape2.scale = current_scale;
        }

        UpdateShapeBuffer(0);
        UpdateShapeBuffer(1);
        glutPostRedisplay();
    }
    glutTimerFunc(16, Timer, 0); // 다음 16ms 후에 다시 호출
}

void InitShape()
{

    //shape1
    {
        float offset_x = 0.2f;
        float offset_y = 0.2f;
        mShape1.vertices = {
            offset_x,  offset_y, 1.0f, 0.0f, 0.0f,
            offset_x * 2, 0.0f, 1.0f, 0.0f, 0.0f,
            offset_x, -offset_y, 1.0f, 0.0f, 0.0f,
            0, -offset_y * 2 , 1.0f, 0.0f, 0.0f,
            -offset_x, -offset_y, 1.0f, 0.0f, 0.0f,
            -offset_x * 2, 0.0f, 1.0f, 0.0f, 0.0f,
            -offset_x,  offset_y, 1.0f, 0.0f, 0.0f,
            0,  offset_y * 2 , 1.0f, 0.0f, 0.0f,

        };
        mShape1.indices = {
            0,1,2,
            2,3,4,
            4,5,6,
            6,7,0
        };
        mShape1.centerX = -0.4f;
        mShape1.centerY = 0.0f;
    
        mShape1.rotation = 0.0f;
        mShape1.scale = 1.0f;
    }


    //shape2
    {
        float offset_x = 0.2f;
        float offset_y = 0.2f;
        float inner_offset_x = 0.05f;
        float inner_offset_y = 0.05f;

        mShape2.vertices = {
            offset_x,  offset_y, 0.0f, 0.0f, 1.0f,
            inner_offset_x,  0, 0.0f, 0.0f, 1.0f,
            offset_x, -offset_y, 0.0f, 0.0f, 1.0f,
            0 , -inner_offset_y , 0.0f, 0.0f, 1.0f,
            -offset_x, -offset_y, 0.0f, 0.0f, 1.0f,
            -inner_offset_x, 0, 0.0f, 0.0f, 1.0f,
            -offset_x,  offset_y, 0.0f, 0.0f, 1.0f,
            0, inner_offset_y , 0.0f, 0.0f, 1.0f
        };

        mShape2.indices = {
            0,1,2,
            2,3,4,
            4,5,6,
            6,7,0
        };
        mShape2.centerX = 0.4f;
        mShape2.centerY = 0.0f;
        mShape2.rotation = 0.0f;
        mShape2.scale = 1.0f;
    }

}