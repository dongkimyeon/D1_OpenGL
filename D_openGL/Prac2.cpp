#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" // filetobuf 함수와 ObjRead.h 포함

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y); // 특수 키(화살표) 처리 함수 선언
void LoadOBJ(const char* filename);

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

Model model;

// 도형 타입 정의
enum ShapeType { P, LINE, TRIANGLE, RECTANGLE };

struct Shape {
    ShapeType type;
    std::vector<GLfloat> coord;
    std::vector<GLfloat> color;
};

Shape test = { P, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };
glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);
float cameraAngle = glm::atan(1.0f); // 초기 각도 (45도 라디안)

// 도형(모델) 이동 및 회전 관련 추가 전역 변수
glm::vec3 modelPos = glm::vec3(0.0f, 0.0f, 0.0f);  // 도형 위치 (X, Y, Z 이동량)
float modelRotY = 0.0f;  // Y축 회전 각도 (라디안)
float modelRotX = 0.0f;  // X축 회전 각도 (라디안)

int main(int argc, char** argv) {
    srand(static_cast<unsigned>(time(0))); // 랜덤 시드 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // GLUT_DEPTH 추가
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Prac");

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

    // OBJ 파일 로드
    LoadOBJ("cube.obj");

    InitBuffers();

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard); // 특수 키(화살표) 함수 등록

    glutMainLoop();
    return 0;
}

void LoadOBJ(const char* filename) {
    read_obj_file(filename, &model);
    if (model.vertex_count == 0 || model.face_count == 0) {
        std::cerr << "OBJ 로드 실패: " << filename << std::endl;
    }
    else {
        std::cout << "OBJ 로드 완료: " << model.vertex_count << " 버텍스, " << model.face_count << " 페이스" << std::endl;
    }
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

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);


    const GLsizei stride = 3 * sizeof(GLfloat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);  // 언바인드 추가
}

void DrawScene() {
    if (model.vertex_count == 0 || model.face_count == 0) {
        std::cerr << "OBJ 데이터가 로드되지 않았습니다." << std::endl;
        glutSwapBuffers();
        return;
    }

    // 카메라 위치 업데이트 (Y축 회전)
    float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z); // XZ 평면 거리
    cameraPos.x = r * glm::cos(cameraAngle);
    cameraPos.z = r * glm::sin(cameraAngle);

    glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_CULL_FACE); // 은면 제거 활성화
    glCullFace(GL_BACK); // 뒷면 제거 (기본 설정)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 깊이 버퍼 클리어 추가

    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO);

    // OBJ 버텍스 데이터: 위치 (x,y,z)만 (색상은 uniform으로 처리)
    std::vector<GLfloat> vertexData;
    for (size_t i = 0; i < model.vertex_count; ++i) {
        Vertex v = model.vertices[i];
        // OBJ 좌표를 OpenGL에 맞게 사용 (스케일링 제거, 0~1 범위 그대로)
        float x = v.x;
        float y = v.y;
        float z = v.z;
        vertexData.push_back(x);
        vertexData.push_back(y);
        vertexData.push_back(z);
    }

    // 인덱스 데이터 (총 36 indices, 12 faces * 3)
    std::vector<GLuint> indices;
    for (size_t i = 0; i < model.face_count; ++i) {
        indices.push_back(model.faces[i].v1);
        indices.push_back(model.faces[i].v2);
        indices.push_back(model.faces[i].v3);
    }

    // VBO에 위치 데이터만 업로드
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

    // EBO에 인덱스 데이터 업로드
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

    // Uniform 매트릭스 설정
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");
    GLint faceColorLoc = glGetUniformLocation(shaderProgramID, "faceColor");

    // 모델 매트릭스: 이동 + 회전 적용 (회전은 이동 후 적용되도록 순서 주의)
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, modelPos);  // 먼저 이동 적용
    modelMat = glm::rotate(modelMat, modelRotY, glm::vec3(0.0f, 1.0f, 0.0f));  // Y축 회전
    modelMat = glm::rotate(modelMat, modelRotX, glm::vec3(1.0f, 0.0f, 0.0f));  // X축 회전
    //--------------------------------------------------------------------------
    // 대각선 뷰: 카메라를 (-5, 5, 5) 위치로 해서 큐브와 축 잘 보이게
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 뷰 매트릭스
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f); // 프로젝션 매트릭스

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    // 면 색상 배열 (OBJ face 순서에 맞춰 6면 그룹화: 각 면 2 faces = 6 indices)
    glm::vec3 colors[6] = {
        glm::vec3(1.0f, 0.0f, 0.0f), // 0: back (faces 0-1)
        glm::vec3(0.0f, 1.0f, 0.0f), // 1: front? (faces 2-3) - 실제 OBJ에 맞춰 조정 가능
        glm::vec3(0.0f, 1.0f, 1.0f), // 2: top (faces 4-5)
        glm::vec3(1.0f, 1.0f, 0.0f), // 3: bottom (faces 6-7)
        glm::vec3(1.0f, 0.0f, 1.0f), // 4: left (faces 8-9)
        glm::vec3(1.0f, 0.5f, 0.0f)  // 5: right (faces 10-11)
    };

    // 각 면(6 indices)마다 색상 uniform 설정 후 그리기
    for (int face = 0; face < 6; ++face) {
        int startIdx = face * 6; // 각 면 6 indices (2 tris * 3)
        glUniform3f(faceColorLoc, colors[face].r, colors[face].g, colors[face].b);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
    }
	//---------------------------------------------------------------------------
    // XYZ 축 그리기 전에 모델 매트릭스를 단위 행렬로 재설정 (축은 고정되게 함)
    glm::mat4 axisModelMat = glm::mat4(1.0f);  // 단위 행렬 (이동/회전 없음)
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(axisModelMat));

    // XYZ 축 그리기 (큐브와 별도로, 위치만 업로드하고 색상 uniform으로 빨강/녹색/파랑)
    std::vector<GLfloat> axesData = {
        // X축
        -5.0f, 0.0f, 0.0f,  // 시작
         5.0f, 0.0f, 0.0f,  // 끝

         // Y축
          0.0f,-5.0f, 0.0f,
          0.0f, 5.0f, 0.0f,

          // Z축
           0.0f, 0.0f,-5.0f,
           0.0f, 0.0f, 5.0f
    };

    // 축용 VBO 업로드 (위치만, 색상 uniform 사용)
    glBufferData(GL_ARRAY_BUFFER, axesData.size() * sizeof(GLfloat), axesData.data(), GL_DYNAMIC_DRAW);

    // X축
    glUniform3f(faceColorLoc, 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 0, 2);

    // Y축 
    glUniform3f(faceColorLoc, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 2, 2);

    // Z축 
    glUniform3f(faceColorLoc, 0.0f, 1.0f, 0.0f);
    glDrawArrays(GL_LINES, 4, 2);

    glBindVertexArray(0);
    glutSwapBuffers();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

void Mouse(int button, int state, int x, int y) {

}

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'a': //카메라를 Y축 중심으로 회전 
        cameraAngle += 0.1f; // 각도 증가 (라디안)
        break;
    case 'd':
        cameraAngle -= 0.1f; // 각도 감소 (라디안)
        break;
    case 'w': // 도형을 Z축 방향으로 앞으로 이동 (큐브를 카메라 쪽으로)
        modelPos.z += 0.1f;  // Z 증가 (앞으로)
        break;
    case 's': // 도형을 Z축 방향으로 뒤로 이동
        modelPos.z -= 0.1f;  // Z 감소 (뒤로)
        break;
    case 'q':
        exit(0);
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

// 특수 키(화살표) 처리 함수: 도형 회전
void SpecialKeyboard(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:    // 위 화살표: X축으로 도형 회전 (위로 기울임)
        modelRotX += 0.1f;  // X축 회전 증가
        break;
    case GLUT_KEY_DOWN:  // 아래 화살표: X축으로 도형 회전 (아래로 기울임)
        modelRotX -= 0.1f;
        break;
    case GLUT_KEY_LEFT:  // 왼쪽 화살표: Y축으로 도형 좌회전
        modelRotY += 0.1f;  // Y축 회전 증가
        break;
    case GLUT_KEY_RIGHT: // 오른쪽 화살표: Y축으로 도형 우회전
        modelRotY -= 0.1f;  // Y축 회전 감소
        break;
    default:
        break;
    }

    glutPostRedisplay();  // 재렌더링 요청
}