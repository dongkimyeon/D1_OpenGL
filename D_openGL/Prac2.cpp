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
void LoadOBJ(const char* filename);

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

// OBJ 데이터 저장 (ObjRead.h에서 정의된 구조체 사용)
Model model;

// 도형 타입 정의
enum ShapeType { P, LINE, TRIANGLE, RECTANGLE };

struct Shape {
    ShapeType type;
    std::vector<GLfloat> coord;
    std::vector<GLfloat> color;
};

Shape test = { P, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };

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

    // 실제 데이터 크기: 3(pos) + 3(color) = 6 floats
    const GLsizei stride = 6 * sizeof(GLfloat);
    // 위치 속성 (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 속성 (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));  // 오프셋: 위치 다음
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);  // 언바인드 추가
}

void DrawScene() {
    if (model.vertex_count == 0 || model.face_count == 0) {
        std::cerr << "OBJ 데이터가 로드되지 않았습니다." << std::endl;
        glutSwapBuffers();
        return;
    }

    glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 깊이 버퍼 클리어 추가

    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO);

    // OBJ 버텍스 데이터: 위치 (x,y,z) + 색상 (r,g,b) 인터리브 (각 면 색상 다르게)
    std::vector<GLfloat> vertexData;
    // 면 순서: back(0), left(1), top(2), right(3), bottom(4), front(5)
    glm::vec3 colors[6] = {
        glm::vec3(1.0f, 0.0f, 0.0f), // back: red
        glm::vec3(0.0f, 1.0f, 0.0f), // left: green
        glm::vec3(0.0f, 1.0f, 1.0f), // top: cyan
        glm::vec3(1.0f, 1.0f, 0.0f), // right: yellow
        glm::vec3(1.0f, 0.0f, 1.0f), // bottom: magenta
        glm::vec3(1.0f, 0.5f, 0.0f)  // front: orange
    };

    // 각 면의 색상을 버텍스에 적용 (OBJ 인덱스 기반으로 그룹화)
    for (size_t i = 0; i < model.vertex_count; ++i) {
        Vertex v = model.vertices[i];
        // OBJ 좌표를 OpenGL에 맞게 -1 ~ 1로 스케일링 및 센터링 (0~1 범위 가정)
        float x = (v.x - 0.5f) * 2.0f;
        float y = (v.y - 0.5f) * 2.0f;
        float z = (v.z - 0.5f) * 2.0f;
        vertexData.push_back(x);
        vertexData.push_back(y);
        vertexData.push_back(z);
        // 간단히 버텍스 인덱스 기반으로 색상 할당 (실제로는 면 그룹화 필요하지만, 데모용으로 랜덤/고정)
        // 여기서는 모든 버텍스에 빨강으로 (이전 기본처럼), 또는 i % 6으로 색상 변경
        int faceIdx = i % 6;
        vertexData.push_back(colors[faceIdx].r);
        vertexData.push_back(colors[faceIdx].g);
        vertexData.push_back(colors[faceIdx].b);
    }

    // 인덱스 데이터
    std::vector<GLuint> indices;
    for (size_t i = 0; i < model.face_count; ++i) {
        indices.push_back(model.faces[i].v1);
        indices.push_back(model.faces[i].v2);
        indices.push_back(model.faces[i].v3);
    }

    // VBO에 버텍스 데이터 업로드
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

    // EBO에 인덱스 데이터 업로드
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

    // Uniform 매트릭스 설정
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

    glm::mat4 modelMat = glm::mat4(1.0f); // 모델 매트릭스 (회전 없음)
    // 대각선 뷰: 카메라를 (5, 5, 5) 위치로 해서 큐브와 축 잘 보이게
    glm::mat4 view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 뷰 매트릭스
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f); // 프로젝션 매트릭스

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    // OBJ 큐브 그리기 (삼각형 모드)
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    // XYZ 축 그리기 (5,5,5까지 선으로, 큐브 위에 오버레이)
    std::vector<GLfloat> axesData = {
        // X축: 빨강
        -5.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // 원점
        5.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // X 끝

        // Y축: 녹색
        0.0f, -5.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // 원점
        0.0f, 5.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Y 끝

        // Z축: 파랑
        0.0f, 0.0f, -5.0f,  0.0f, 0.0f, 1.0f,  // 원점
        0.0f, 0.0f, 5.0f,  0.0f, 0.0f, 1.0f   // Z 끝
    };

    // VBO에 축 데이터 업로드 (큐브 데이터 덮어쓰기, 하지만 배열 모드로 그리기)
    glBufferData(GL_ARRAY_BUFFER, axesData.size() * sizeof(GLfloat), axesData.data(), GL_DYNAMIC_DRAW);

    // 축 그리기 (라인 모드, EBO 무시)
    glDrawArrays(GL_LINES, 0, 6);

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

void Keyboard(unsigned char key, int x, int y) {

}