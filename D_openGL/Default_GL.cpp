#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include "CommonInclude.h" 
#include "Time.h"

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void Timer(int value);
void UpdateModelBuffers();
glm::vec3 CalculateModelCenter(const std::vector<glm::vec3>& vertices); // 센터 계산 (벡터 기반으로 변경)

// 전역 변수
GLint width = 1280, height = 720;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;
GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO


Model BaseBody;
Model MiddleBody;
Model TopBody;
Model FlagModel[2];
Model BarrelModel[2];


float cameraAngle = glm::atan(1.0f);
float targetCameraAngle = glm::atan(1.0f);
glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);
float interpSpeed = 10.0f;

// ImGui 관련
bool showAxis = true;  // 축 표시 토글
float modelRotationY = 45.0f;  // ImGui로 제어할 모델 Y 회전
float modelScale = 1.5f;       // ImGui로 제어할 스케일

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Default_GL_imgui_assimp");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        return -1;
    }

    read_obj_file("cube.obj", &BaseBody);
    read_obj_file("cube.obj", &MiddleBody);
    read_obj_file("cube.obj", &TopBody);
    read_obj_file("cube.obj", &FlagModel[0]);
    read_obj_file("cube.obj", &FlagModel[1]);
    read_obj_file("cube.obj", &BarrelModel[0]);
    read_obj_file("cube.obj", &BarrelModel[1]);

    // ImGui 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGLUT_Init(); 
    ImGui_ImplOpenGL3_Init("#version 330");

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
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, Timer, 0); // ~60 FPS
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	Time::Initialize();
    glutMainLoop();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
    return 0;
}

void MakeVertexShaders() {
    GLchar* vertexSource = filetobuf("vertex3D.glsl");
    if (!vertexSource) {
        std::cerr << "ERROR: vertex3D.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, NULL);
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
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, NULL);
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
    // 메인 모델 VAO/VBO/EBO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 데이터를 나중에 채울 것이므로 GL_STATIC_DRAW로 변경
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // 데이터를 나중에 채울 것이므로 GL_STATIC_DRAW로 변경
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    const GLsizei stride = 6 * sizeof(GLfloat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // --- 축 데이터  ---

    std::vector<GLfloat> axesData = {
        -10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // X red
        10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, -10.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Y blue
        0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, // Z green
        0.0f, 0.0f, 10.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &axisVAO);
    glBindVertexArray(axisVAO);

    glGenBuffers(1, &axisVBO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    // 축 데이터를 여기서 GL_STATIC_DRAW로 한 번만 업로드합니다.
    glBufferData(GL_ARRAY_BUFFER, axesData.size() * sizeof(GLfloat), axesData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
void UpdateModelBuffers()
{

    /
    glBindVertexArray(0);

    std::cout << "모델 버퍼가 GPU에 업로드되었습니다. (정점 " << fbxModel.vertices.size() << "개)" << std::endl;
}


glm::vec3 CalculateModelCenter(const std::vector<glm::vec3>& vertices) {
    if (vertices.empty()) return glm::vec3(0.0f);
    glm::vec3 center(0.0f);
    for (const auto& v : vertices) {
        center += v;
    }
    center /= static_cast<float>(vertices.size());
    return center;
}

void DrawScene() 
{
    Time::Update();
  
    // 2. 카메라 위치 업데이트
    float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z);
    cameraPos.x = r * glm::cos(cameraAngle);
    cameraPos.z = r * glm::sin(cameraAngle);

    // 3. 씬 클리어
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(shaderProgramID);

    // 4. Uniform 설정 (View, Proj - 모든 객체 공통)
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");

    // 5. 모델 그리기
   

    // 6. 축 그리기 (토글 가능)
    if (showAxis) {
        // 6-1. 모델 매트릭스 설정 (축은 기본)
        glm::mat4 axisMat = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(axisMat));

        // 6-2. 그리기 (데이터 업로드X, 그리기 명령O)
        glBindVertexArray(axisVAO);
        glDrawArrays(GL_LINES, 0, 6);
        glBindVertexArray(0);
    }


  
    // 7. ImGui 렌더링 (이하 동일)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(
     
        ImVec2((float)width, 0.0f),
        ImGuiCond_FirstUseEver,
        ImVec2(1.0f, 0.0f)
    );
    ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);

    // ImGui UI 윈도우
    ImGui::Begin("TEST");


    static float time = 0.0f;

    time += Time::DeltaTime();
    float fps = 1.0f / Time::DeltaTime();


    ImGui::Text("FPS: %.1f", fps);



    if (ImGui::Button("test")) 
    {
		std::cout << "Test button clicked!" << std::endl;
    }

    ImGui::End();

    // ImGui 렌더링
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
}
void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    ImGui_ImplGLUT_ReshapeFunc(w, h);  // ImGui 리셰이프
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a': targetCameraAngle += 0.1f; break;
    case 'd': targetCameraAngle -= 0.1f; break;
    case 'q': exit(0); break;
    }
    glutPostRedisplay();
}

void Timer(int value) {
   
    cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * Time::DeltaTime());
    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}

void Mouse(int button, int state, int x, int y) {
    ImGui_ImplGLUT_MouseFunc(button, state, x, y);

    // ImGui가 마우스를 사용하지 않을 때만 사용자의 마우스 로직 처리
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse)
    {
        // (사용자의 마우스 클릭 로직이 있다면 여기에 추가)
    }
}


void Motion(int x, int y) {
    ImGui_ImplGLUT_MotionFunc(x, y);

    // ImGui가 마우스를 사용하지 않을 때만 사용자의 마우스 로직 처리
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse)
    {
        // (사용자의 마우스 이동 로직이 있다면 여기에 추가)
    }
}