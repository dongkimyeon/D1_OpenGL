#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" // filetobuf 함수가 포함된 헤더 파일

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);

GLint width = 800, height = 800;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

struct Triangle {
    std::vector<GLfloat> vertices; // x, y, r, g, b
};

std::vector<Triangle> mQudrantOneTriangles;
std::vector<Triangle> mQudrantTwoTriangles;
std::vector<Triangle> mQudrantThreeTriangles;
std::vector<Triangle> mQudrantFourTriangles;

int drawType = 0; // 0: 선 1: 면
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("S9");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        return -1;
    }

    MakeVertexShaders();
    MakeFragmentShaders();
    shaderProgramID = MakeShaderProgram();
    if (shaderProgramID == 0) {
        std::cerr << "셰이더 프로그램 생성 실패" << std::endl;
        return -1;
    }
    InitBuffers();

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutKeyboardFunc(Keyboard);

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
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // 위치 속성 (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 속성 (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void DrawScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO);

    // 화면을 4등분하는 
    std::vector<GLfloat> lines = {
        // 수직선 
        0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 
        0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
        // 수평선
        -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 
         1.0f, 0.0f, 1.0f, 0.0f, 0.0f  
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(GLfloat), lines.data(), GL_DYNAMIC_DRAW);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 4);

    //1사분면
    for (const auto& triangle : mQudrantOneTriangles) {
        // 정점 데이터 전송
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, triangle.vertices.size() * sizeof(GLfloat), triangle.vertices.data(), GL_DYNAMIC_DRAW);
        if(drawType == 0)
            glDrawArrays(GL_LINE_LOOP, 0, 3);
		else
            glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	//2사분면
    for (const auto& triangle : mQudrantTwoTriangles) {
        // 정점 데이터 전송
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, triangle.vertices.size() * sizeof(GLfloat), triangle.vertices.data(), GL_DYNAMIC_DRAW);
        if (drawType == 0)
            glDrawArrays(GL_LINE_LOOP, 0, 3);
        else
            glDrawArrays(GL_TRIANGLES, 0, 3);
    }
	//3사분면
    for (const auto& triangle : mQudrantThreeTriangles) {
        // 정점 데이터 전송
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, triangle.vertices.size() * sizeof(GLfloat), triangle.vertices.data(), GL_DYNAMIC_DRAW);
        if (drawType == 0)
            glDrawArrays(GL_LINE_LOOP, 0, 3);
        else
            glDrawArrays(GL_TRIANGLES, 0, 3);
          
	}
	//4사분면
    for (const auto& triangle : mQudrantFourTriangles) {
        // 정점 데이터 전송
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, triangle.vertices.size() * sizeof(GLfloat), triangle.vertices.data(), GL_DYNAMIC_DRAW);
        if (drawType == 0)
            glDrawArrays(GL_LINE_LOOP, 0, 3);
        else
            glDrawArrays(GL_TRIANGLES, 0, 3);
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
    float ndc_x = (2.0f * x / width) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / height);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) 
    {   
        Triangle newTriangle;
       
        // 이등변 삼각형의 크기
        float base = 0.1f; 
        float height_val = 0.1f; 

        float r = (rand() % 100) / 100.0f;
        float g = (rand() % 100) / 100.0f;
        float b = (rand() % 100) / 100.0f;

       
        float x1 = ndc_x;
        float y1 = ndc_y;
       
        float x2 = ndc_x - base / 2.0f;
        float y2 = ndc_y - height_val;
       
        float x3 = ndc_x + base / 2.0f;
        float y3 = ndc_y - height_val;

        newTriangle.vertices = {
            x1, y1, r, g, b,
            x2, y2, r, g, b,
            x3, y3, r, g, b
        };
        if(ndc_x >= 0.0f && ndc_y >= 0.0f) // 1사분면
        {
			mQudrantOneTriangles.clear();
            mQudrantOneTriangles.push_back(newTriangle);
        }
        else if (ndc_x < 0.0f && ndc_y >= 0.0f) // 2사분면
        {
			mQudrantTwoTriangles.clear();
            mQudrantTwoTriangles.push_back(newTriangle);
        }
        else if (ndc_x < 0.0f && ndc_y < 0.0f) // 3사분면
        {
			mQudrantThreeTriangles.clear();
            mQudrantThreeTriangles.push_back(newTriangle);
        }
        else
        {
			mQudrantFourTriangles.clear();
            mQudrantFourTriangles.push_back(newTriangle);
        }
			
        
    }
    else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) 
    {
        Triangle newTriangle;

        // 이등변 삼각형의 크기
        float base = 0.1f;
        float height_val = 0.1f;

        float r = (rand() % 100) / 100.0f;
        float g = (rand() % 100) / 100.0f;
        float b = (rand() % 100) / 100.0f;


        float x1 = ndc_x;
        float y1 = ndc_y;

        float x2 = ndc_x - base / 2.0f;
        float y2 = ndc_y - height_val;

        float x3 = ndc_x + base / 2.0f;
        float y3 = ndc_y - height_val;

        newTriangle.vertices = {
            x1, y1, r, g, b,
            x2, y2, r, g, b,
            x3, y3, r, g, b
        };
        if (ndc_x >= 0.0f && ndc_y >= 0.0f && mQudrantOneTriangles.size() < 4) // 1사분면
        {
            mQudrantOneTriangles.push_back(newTriangle);
        }
        else if (ndc_x < 0.0f && ndc_y >= 0.0f && mQudrantTwoTriangles.size() < 4) // 2사분면
        {
          
            mQudrantTwoTriangles.push_back(newTriangle);
        }
        else if (ndc_x < 0.0f && ndc_y < 0.0f && mQudrantThreeTriangles.size() < 4) // 3사분면
        {
           
            mQudrantThreeTriangles.push_back(newTriangle);
        }
		else if (ndc_x >= 0.0f && ndc_y < 0.0f && mQudrantFourTriangles.size() < 4) // 4사분면
        {
            
            mQudrantFourTriangles.push_back(newTriangle);
        }
	}
}

void Keyboard(unsigned char key, int x, int y) 
{
    switch (key) 
    {
    case 'a':
        drawType = 1;
        break;
    case 'b':
		drawType = 0; // 선
        break;
	case 'c':
		mQudrantOneTriangles.clear();
		mQudrantTwoTriangles.clear();
		mQudrantThreeTriangles.clear();
		mQudrantFourTriangles.clear();
        break;
    case 'q':
		// 프로그램 종료
        exit(0);
		break;
    default:
        break;
	}
    glutPostRedisplay();
}