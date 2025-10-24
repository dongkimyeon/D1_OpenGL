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
bool LoadFBX(const char* filename); // FBX 로드 함수 
glm::vec3 CalculateModelCenter(const std::vector<glm::vec3>& vertices); // 센터 계산 

void ProcessNode(aiNode* node, int depth);

// 전역 변수
GLint width = 1280, height = 720;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO

// FBX 모델 데이터 
struct FBXModel {
    std::vector<glm::vec3> vertices;  // 위치
    std::vector<glm::vec3> colors;    
    std::vector<GLuint> indices;      // 인덱스
    glm::vec3 center;
    bool loaded = false;
};


FBXModel fbxModel;

float cameraAngle = glm::atan(1.0f);
float targetCameraAngle = glm::atan(1.0f);
glm::vec3 cameraPos = glm::vec3(100.0f, 100.0f, 100.0f);
float interpSpeed = 10.0f;

// ImGui 관련
bool showAxis = true;  // 축 표시 토글
float modelRotationY = 45.0f;  // ImGui로 제어할 모델 Y 회전
float modelScale = 0.1f;       // ImGui로 제어할 스케일
int targetFrameDelay = 1;
bool wireframeMode = false;
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

    // 초기 FBX 로드 (예시 파일)
    if (!LoadFBX("SK_West_Heli_AH64D.FBX")) {
        std::cerr << "초기 FBX 로드 실패. 파일을 확인하세요." << std::endl;
    }
    Time::Initialize();

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(targetFrameDelay, Timer, 0); // ~60 FPS
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);

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
        -200.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // X red
        200.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, -200.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Y blue
        0.0f, 200.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, -200.0f, 0.0f, 1.0f, 0.0f, // Z green
        0.0f, 0.0f, 200.0f, 0.0f, 1.0f, 0.0f
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
    if (!fbxModel.loaded || fbxModel.vertices.empty() || fbxModel.indices.empty()) {
        std::cerr << "모델 버퍼 업데이트 실패: 로드된 데이터 없음" << std::endl;
        return;
    }

    // 1. 버텍스 데이터: 위치 + 색상 (센터 정규화)
    std::vector<GLfloat> vertexData;
    // std::vector::reserve를 사용하여 미리 메모리를 할당하면 성능에 약간 더 좋습니다.
    vertexData.reserve(fbxModel.vertices.size() * 6); // 6 = pos(3) + color(3)

    for (size_t i = 0; i < fbxModel.vertices.size(); ++i) {
        glm::vec3 pos = fbxModel.vertices[i] - fbxModel.center;
        vertexData.insert(vertexData.end(), { pos.x, pos.y, pos.z });
        vertexData.insert(vertexData.end(), {
            fbxModel.colors[i].r, fbxModel.colors[i].g, fbxModel.colors[i].b
            });
    }

    // 2. VBO 및 EBO에 데이터 업로드 (GL_STATIC_DRAW 사용)
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, fbxModel.indices.size() * sizeof(GLuint), fbxModel.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    std::cout << "모델 버퍼가 GPU에 업로드되었습니다. (정점 " << fbxModel.vertices.size() << "개)" << std::endl;
}

// Assimp로 FBX 로드
bool LoadFBX(const char* filename) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp 로드 실패: " << importer.GetErrorString() << std::endl;
        return false;
    }

    fbxModel.vertices.clear();
    fbxModel.colors.clear();
    fbxModel.indices.clear();
    fbxModel.loaded = false;

     // --- 골격(Bone) 구조 출력 ---
    std::cout << "\n--- FBX Node/Skeleton Hierarchy ---" << std::endl;
    // aiScene의 루트 노드부터 재귀적으로 노드 계층 구조를 출력합니다.
    ProcessNode(scene->mRootNode, 0);
    std::cout << "-----------------------------------" << std::endl;
    // 모든 메쉬를 순회하며 데이터를 병합합니다.
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];

        // 이 메쉬의 정점이 시작되기 전, fbxModel에 이미 저장된 정점의 개수.
        // 이것이 이 메쉬의 인덱스 오프셋(offset)이 됩니다.
        GLuint vertexOffset = static_cast<GLuint>(fbxModel.vertices.size());

        // 1. 정점(Vertices)과 색상(Colors) 데이터 추가
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            // 위치
            fbxModel.vertices.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
            // 색상 
            glm::vec3 colors[4] = { {1,0,0}, {0,1,0}, {0,0,1}, {1,1,0} };
            fbxModel.colors.push_back(colors[(vertexOffset + i) % 4]);
        }

        // 2. 인덱스(Indices) 데이터 추가
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            // Assimp가 aiProcess_Triangulate를 보장하므로 mNumIndices는 3입니다.
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                // 중요: 그냥 face.mIndices[j]를 추가하면 안 됩니다!
                // 이미 fbxModel.vertices에 저장된 정점들을 고려하여
                // 'vertexOffset'을 더해줘야 합니다.
                fbxModel.indices.push_back(face.mIndices[j] + vertexOffset);
            }
        }
    } // 다음 메쉬로 이동

    // 모든 메쉬가 병합된 후 센터 계산
    fbxModel.center = CalculateModelCenter(fbxModel.vertices);
    fbxModel.loaded = true;
    std::cout << "FBX 로드 성공: " << scene->mNumMeshes << " meshes combined." << std::endl;
    std::cout << "Total Vertices: " << fbxModel.vertices.size() << ", Total Indices: " << fbxModel.indices.size() << std::endl;

    UpdateModelBuffers();
    return fbxModel.loaded;
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

void ProcessNode(aiNode* node, int depth)
{
    // 들여쓰기를 위한 문자열 생성
    std::string indent(depth * 2, ' ');

    // 노드 이름 출력
    std::cout << indent << "Node Name: " << node->mName.C_Str();

    // 이 노드가 특정 메쉬의 뼈(Bone) 이름과 일치하는지 확인하는 로직을 추가할 수 있습니다.
    // 여기서는 간단하게 노드의 이름만 출력합니다. (대부분의 뼈가 노드 이름으로 존재합니다)
    std::cout << std::endl;

    // 모든 자식 노드에 대해 재귀적으로 처리
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], depth + 1);
    }
}

void DrawScene() 
{
    Time::Update();
    // 1. 데이터 로드 확인 
    if (!fbxModel.loaded || fbxModel.indices.empty()) {
       
        std::cerr << "FBX 데이터가 로드되지 않았습니다." << std::endl;
    }

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
    if (fbxModel.loaded && !fbxModel.indices.empty())
    {
        // 5-1. 모델 매트릭스 설정
        glm::mat4 modelMat = glm::mat4(1.0f);
        //modelMat = glm::rotate(modelMat, glm::radians(modelRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMat = glm::rotate(modelMat, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // X축 -90도 회전 -> opengl 기본 좌표계에 맞춤
        modelMat = glm::scale(modelMat, glm::vec3(modelScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

        // 5-2. 그리기 (데이터 업로드X, 그리기 명령O)
        glBindVertexArray(VAO);
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
        glDrawElements(GL_TRIANGLES, fbxModel.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

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
    ImGui::Begin("FBX Model Controls");


    static float time = 0.0f;

    time += Time::DeltaTime();
    float fps = 1.0f / Time::DeltaTime();


    ImGui::Text("FPS: %.1f", fps);

    // === 갱신 주기 제어 슬라이더 ===
  
    ImGui::SliderInt("Frame Delay ", &targetFrameDelay, 1, 16);
    ImGui::Separator();



    if (ImGui::Button("wired frame")) 
    {
        wireframeMode = !wireframeMode;
    }

    ImGui::End();

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
    glutTimerFunc(targetFrameDelay, Timer, 0);
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