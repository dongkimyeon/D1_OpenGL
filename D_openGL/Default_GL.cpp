#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include "CommonInclude.h" 
#include "Time.h"
#include "Texture.h"


// FBX 모델 데이터 
struct FBXModel
{
    std::vector<glm::vec3> vertices;  // 위치
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> uvs; // UV 좌표
    std::vector<GLuint> indices;      // 인덱스
    std::vector<Texture*> textureList; // 텍스처 리스트
    glm::vec3 center;
    bool loaded = false;
};

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);

//glut 콜백 함함수
void Keyboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y);
void Mouse(int button, int state, int x, int y);
void WhellFunc(int whell, int dir, int x, int y);
void Motion(int x, int y);
void Timer(int value);
void UpdateModelBuffers(FBXModel* model, GLuint vao, GLuint vbo, GLuint ebo); //해당 모델을 버퍼에 업데이트 (버퍼 ID 매개변수 추가)
bool LoadFBX(const char* filename, FBXModel* model); // FBX 로드 함수 
glm::vec3 CalculateModelCenter(const std::vector<glm::vec3>& vertices); // 센터 계산 


//스카이 박스 관련 함수
void MakeSkyboxVertexShader();
void MakeSkyboxFragmentShader();
GLuint MakeSkyboxShaderProgram();
GLuint loadCubemap(std::vector<std::string> faces);

// 전역 변수
GLint width = 1280, height = 720;
GLuint shaderProgramID, vertexShader, fragmentShader;

// 모델별 VAO/VBO/EBO (변경: 각 모델별로 분리)
GLuint VAO_Body, VBO_Body, EBO_Body;
GLuint VAO_Blade, VBO_Blade, EBO_Blade;
GLuint VAO_Tail, VBO_Tail, EBO_Tail;
GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO




//스카이박스 관련
GLuint skyboxVAO, skyboxVBO;
GLuint skyboxShaderProgramID, vertexSkyboxShader, fragmentSkyboxShader;
GLuint cubemapTexture;


//카메라 관련
glm::vec3 cameraPos = glm::vec3(75.0f, 75.0f, 75.0f);
float cameraRadius = 0.0f; // 카메라 반지름 추가
float cameraAngle = 0.0f; // 초기 X-Z 각도 (45도 라디안)
float cameraYAngle = 0.0f; // 초기 Y 각도 (고도각)
float targetCameraXAngle = 0.0f; // 타겟 X-Z 카메라 각도
float targetCameraYAngle = 0.0f; // 타겟 Y 카메라 각도

bool rightClickDown = false; // 마우스 우클릭 상태
int lastMouseX = 0;          // 마지막 마우스 X 위치
int lastMouseY = 0;          // 마지막 마우스 Y 위치
float rotationSpeed = 0.005f; // 카메라 회전 속도 (조정 가능)

//선형보간 속도
float interpSpeed = 10.0f;



//imgui 관련 변수
bool showAxis = true;  // 축 표시 토글

float modelScale = 0.1f;       // ImGui로 제어할 스케일
int targetFrameDelay = 1;
bool wireframeMode = false;

float mainBladeRotation = 0.0f;
float mainBladeSpeed = 100.0f;
float mainBladeX = 0.0f;
float mainBladeY = 0.0f;
float mainBladeZ = 0.0f;

float tailBladeRotation = 0.0f;
float tailBladeSpeed = 100.0f;
float tailBladeX = 0.0f;
float tailBladeY = 0.0f;
float tailBladeZ = 0.0f;

// FBX 모델들
FBXModel mHeliBody;
FBXModel mHeliBlade;
FBXModel mHeliTail;

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

    MakeSkyboxVertexShader();
    MakeSkyboxFragmentShader();
    skyboxShaderProgramID = MakeSkyboxShaderProgram();
    if (skyboxShaderProgramID == 0) {
        std::cerr << "스카이박스 셰이더 프로그램 생성 실패" << std::endl;
        return -1;
    }

    InitBuffers();  // 변경: 모델별 버퍼 초기화
    // 카메라 초기 설정
    cameraRadius = glm::length(cameraPos);
    float r_xz_init = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z);
    cameraAngle = atan2(cameraPos.z, cameraPos.x);
    cameraYAngle = atan2(cameraPos.y, r_xz_init);
    targetCameraXAngle = cameraAngle;
    targetCameraYAngle = cameraYAngle;

    // 초기 FBX 로드 (예시 파일) + 버퍼 업데이트
    if (!LoadFBX("HeliBlade.FBX", &mHeliBlade)) {
        std::cerr << "초기 FBX 로드 실패. 파일을 확인하세요." << std::endl;
    }
    else {
        UpdateModelBuffers(&mHeliBlade, VAO_Blade, VBO_Blade, EBO_Blade);  // 변경: 모델별 버퍼 호출
    }
    if (!LoadFBX("HeliBody.FBX", &mHeliBody)) {
        std::cerr << "초기 FBX 로드 실패. 파일을 확인하세요." << std::endl;
    }
    else {
        UpdateModelBuffers(&mHeliBody, VAO_Body, VBO_Body, EBO_Body);  // 변경: 모델별 버퍼 호출
    }
    if (!LoadFBX("HeliTail.FBX", &mHeliTail)) {
        std::cerr << "초기 FBX 로드 실패. 파일을 확인하세요." << std::endl;
    }
    else {
        UpdateModelBuffers(&mHeliTail, VAO_Tail, VBO_Tail, EBO_Tail);  // 변경: 모델별 버퍼 호출
    }

    std::vector<std::string> faces = {
          "SkyBox-Texture/px.png", // 오른쪽
          "SkyBox-Texture/nx.png", // 왼쪽
          "SkyBox-Texture/py.png", // 위
          "SkyBox-Texture/ny.png", // 아래
          "SkyBox-Texture/pz.png", // 앞
          "SkyBox-Texture/nz.png"  // 뒤
    };
    cubemapTexture = loadCubemap(faces);
    if (cubemapTexture == 0) {
        std::cerr << "스카이박스 큐브맵 로드 실패" << std::endl;
    }
    Time::Initialize();

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(targetFrameDelay, Timer, 0); // ~60 FPS
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutSpecialFunc(SpecialKeyboard);
	glutMouseWheelFunc(WhellFunc);

    glutMainLoop();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
void WhellFunc(int whell, int dir, int x, int y)
{
	float zoomSpeed = 5.0f;
    if(dir > 0)
    {
        cameraRadius -= zoomSpeed;
        if (cameraRadius < 2.0f)
            cameraRadius = zoomSpeed;
    }
    else
    {
        cameraRadius += zoomSpeed;
	}
}
void UpdateModelBuffers(FBXModel* model, GLuint vao, GLuint vbo, GLuint ebo)  // 변경: 버퍼 ID 매개변수 추가
{
    if (!model->loaded || model->vertices.empty() || model->indices.empty()) {
        std::cerr << "모델 버퍼 업데이트 실패: 로드된 데이터 없음" << std::endl;
        return;
    }

    // 1. 버텍스 데이터: 위치 + 색상 + UV (센터 정규화)
    std::vector<GLfloat> vertexData;
    vertexData.reserve(model->vertices.size() * 8); // 8 = pos(3) + color(3) + uv(2)

    for (size_t i = 0; i < model->vertices.size(); ++i) {
        glm::vec3 pos = model->vertices[i] - model->center;
        vertexData.insert(vertexData.end(), { pos.x, pos.y, pos.z });
        vertexData.insert(vertexData.end(), { model->colors[i].r, model->colors[i].g, model->colors[i].b });
        vertexData.insert(vertexData.end(), { model->uvs[i].x, model->uvs[i].y }); // UV 추가 
    }

    // 2. VBO 및 EBO에 데이터 업로드 (GL_STATIC_DRAW 사용, 해당 버퍼만)
    glBindVertexArray(vao);  // 변경: 매개변수 vao 사용

    glBindBuffer(GL_ARRAY_BUFFER, vbo);  // 변경: 매개변수 vbo 사용
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);  // 변경: 매개변수 ebo 사용
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices.size() * sizeof(GLuint), model->indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    std::cout << "모델 버퍼가 GPU에 업로드되었습니다. (정점 " << model->vertices.size() << "개)" << std::endl;  // 변경: model 이름 출력 위해 간단히
}
bool LoadFBX(const char* filename, FBXModel* fbxModel)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp 로드 실패: " << importer.GetErrorString() << std::endl;
        return false;
    }

    fbxModel->vertices.clear();
    fbxModel->colors.clear();
    fbxModel->uvs.clear();
    fbxModel->indices.clear();
    fbxModel->loaded = false;

    // 메쉬
    // 모든 메쉬를 순회하며 데이터를 병합합니다.
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];

        // 이 메쉬의 정점이 시작되기 전, fbxModel에 이미 저장된 정점의 개수.
        // 이것이 이 메쉬의 인덱스 오프셋(offset)이 됩니다.
        GLuint vertexOffset = static_cast<GLuint>(fbxModel->vertices.size());

        // 1. 정점(Vertices), UV, 색상(Colors) 데이터 추가
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            // 위치 로드 후 변환 적용
            glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            // 좌표계 변환: Y-up -> Z-up (X축 90도 회전 매트릭스 적용)
            glm::mat4 coordTransform = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            pos = glm::vec3(coordTransform * glm::vec4(pos, 1.0f));

            fbxModel->vertices.push_back(pos);

            // UV 로드 추가: aiTextureCoords[0] 채널 사용 (없으면 0으로 기본값)
            glm::vec2 uv(0.0f, 0.0f);
            if (mesh->HasTextureCoords(0)) {
                uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }
            fbxModel->uvs.push_back(uv);

            // 색상 
            glm::vec3 colors[4] = { {1,0,0}, {0,1,0}, {0,0,1}, {1,1,0} };
            fbxModel->colors.push_back(colors[(vertexOffset + i) % 4]);
        }

        // 2. 인덱스(Indices) 데이터 추가
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            // Assimp가 aiProcess_Triangulate를 보장하므로 mNumIndices는 3입니다.
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                // 중요: 그냥 face.mIndices[j]를 추가하면 안 됩니다!
                // 이미 fbxModel.vertices에 저장된 정점들을 고려하여
                // 'vertexOffset'을 더해줘야 합니다.
                fbxModel->indices.push_back(face.mIndices[j] + vertexOffset);
            }
        }
    } // 다음 메쉬로 이동


    // 모든 메쉬가 병합된 후 센터 계산
    fbxModel->center = CalculateModelCenter(fbxModel->vertices);
    fbxModel->loaded = true;
    std::cout << "FBX 로드 성공: " << scene->mNumMeshes << " meshes combined." << std::endl;
    std::cout << "Total Vertices: " << fbxModel->vertices.size() << ", Total Indices: " << fbxModel->indices.size() << std::endl;



    // 텍스처 로딩: material별로
    fbxModel->textureList.resize(scene->mNumMaterials);
    for (size_t i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        fbxModel->textureList[i] = nullptr;

        fbxModel->textureList[i] = new Texture("HeliTexture.png");
        if (!fbxModel->textureList[i]->LoadTexture()) {  
			std::cerr << "텍스처 로드 실패: HeliTexture.png" << std::endl;  // *** 로그 ***
            delete fbxModel->textureList[i];
            fbxModel->textureList[i] = nullptr;
        }
        else {
            std::cout << "텍스처 로드 성공: HeliTexture.png" << std::endl;  // *** 로그 ***
        }
        
        
    }

    // *** 추가: 모든 textureList 유효성 로그 ***
    for (size_t i = 0; i < fbxModel->textureList.size(); ++i) {
        if (fbxModel->textureList[i]) {
            std::cout << "Material " << i << " Texture ID: " << fbxModel->textureList[i] << std::endl;  // Texture.h에 public GLuint textureID; 가정
        }
        else {
            std::cout << "Material " << i << ": No Texture" << std::endl;
        }
    }

    return fbxModel->loaded;
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

    mainBladeRotation += mainBladeSpeed * Time::DeltaTime();
    tailBladeRotation += tailBladeSpeed * Time::DeltaTime();
    // 카메라 위치 업데이트 (구면 좌표계 사용)
    float theta = cameraAngle;
    float beta = cameraYAngle; // 고도각 
    float r_xz = cameraRadius * glm::cos(beta);
    cameraPos.x = r_xz * glm::cos(theta);
    cameraPos.z = r_xz * glm::sin(theta);
    cameraPos.y = cameraRadius * glm::sin(beta);

    // 씬 클리어
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // --- 기본 깊이 테스트 함수 설정 ---
    glDepthFunc(GL_LESS);

    glUseProgram(shaderProgramID);

    // Uniform 설정 (View, Proj - 모든 객체 공통)
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");
    GLint textureLoc = glGetUniformLocation(shaderProgramID, "textureSampler");
    GLint useVertexColorLoc = glGetUniformLocation(shaderProgramID, "useVertexColor");

    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 뷰 매트릭스

    glm::mat4 proj = glm::perspective(glm::radians(100.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");

    // 5. 모델 그리기
    //몸체 (변경: VAO_Body 바인딩)
    if (mHeliBody.loaded && !mHeliBody.indices.empty())
    {
        glUniform1i(useVertexColorLoc, 0);
        // 5-1. 모델 매트릭스 설정
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::scale(modelMat, glm::vec3(modelScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

        // *** 텍스처 바인딩 추가 ***
        if (mHeliBody.textureList.size() > 0 && mHeliBody.textureList[0]) {
            mHeliBody.textureList[0]->UseTexture();
            glUniform1i(textureLoc, 0);
        }

        // 5-2. 그리기
        glBindVertexArray(VAO_Body);
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, mHeliBody.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    //Main Blade
    if (mHeliBlade.loaded && !mHeliBlade.indices.empty())
    {
        glUniform1i(useVertexColorLoc, 0);
        // 5-1. 모델 매트릭스 설정
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, glm::vec3(2.5f, 18.0f, 0.0f));
        modelMat = glm::rotate(modelMat, glm::radians(mainBladeRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMat = glm::scale(modelMat, glm::vec3(modelScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

        // *** 텍스처 바인딩 추가 ***
        if (mHeliBlade.textureList.size() > 0 && mHeliBlade.textureList[0]) {
            mHeliBlade.textureList[0]->UseTexture();
            glUniform1i(textureLoc, 1);
        }

        // 5-2. 그리기
        glBindVertexArray(VAO_Blade);
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, mHeliBlade.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    //Tail Blade
    if (mHeliTail.loaded && !mHeliTail.indices.empty())
    {
        glUniform1i(useVertexColorLoc, 0);
        // 5-1. 모델 매트릭스 설정
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, glm::vec3(-88.0f, 17.0f, -7.0f));
        modelMat = glm::rotate(modelMat, glm::radians(tailBladeRotation), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMat = glm::scale(modelMat, glm::vec3(modelScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

        // *** 텍스처 바인딩 추가 ***
        if (mHeliTail.textureList.size() > 0 && mHeliTail.textureList[0]) {
            mHeliTail.textureList[0]->UseTexture();
            glUniform1i(textureLoc, 0);
        }

        // 5-2. 그리기
        glBindVertexArray(VAO_Tail);
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, mHeliTail.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // 축 그리기 (토글 가능)
    if (showAxis) {
        glUniform1i(useVertexColorLoc, 1);
        // 6-1. 모델 매트릭스 설정 (축은 기본)
        glm::mat4 axisMat = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(axisMat));

        // 6-2. 그리기
        glBindVertexArray(axisVAO);
        glDrawArrays(GL_LINES, 0, 6);
        glBindVertexArray(0);
    }

    // --- 스카이박스 그리기 (모델 그린 후, ImGui 그리기 전) ---

    // 깊이 테스트 함수를 GL_LEQUAL로 변경합니다. (스카이박스 셰이더의 pos.xyww 트릭 때문)
    glDepthFunc(GL_LEQUAL);

    glUseProgram(skyboxShaderProgramID); // 스카이박스 전용 셰이더 사용

    // 셰이더에 view, projection 행렬 전달
    // (스카이박스 셰이더가 내부적으로 view의 이동값을 제거함)
    GLint skyViewLoc = glGetUniformLocation(skyboxShaderProgramID, "view");
    GLint skyProjLoc = glGetUniformLocation(skyboxShaderProgramID, "projection");
    glUniformMatrix4fv(skyViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(skyProjLoc, 1, GL_FALSE, glm::value_ptr(proj));

    // 스카이박스 큐브맵 텍스처 바인딩
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0); // 텍스처 유닛 0 사용
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    // 셰이더의 skybox 샘플러가 텍스처 유닛 0을 사용하도록 설정
    glUniform1i(glGetUniformLocation(skyboxShaderProgramID, "skybox"), 0);

    glDrawArrays(GL_TRIANGLES, 0, 36); // 스카이박스 큐브 그리기
    glBindVertexArray(0);

    glDepthFunc(GL_LESS); // 깊이 테스트 함수를 다시 기본값으로 복원
    // --- 스카이박스 그리기 끝 ---


    // ImGui 렌더링 (이하 동일)
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
    ImGui::SliderFloat("Blade Speed", &mainBladeSpeed, 100, 1000);
    ImGui::Separator();
    ImGui::SliderFloat("Tail Speed", &tailBladeSpeed, 100, 1000);
    ImGui::Separator();



    if (ImGui::Button("wired frame"))
    {
        wireframeMode = !wireframeMode;
    }

    if (ImGui::Button("XYZ"))
    {
        showAxis = !showAxis;
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
    switch (key)
    {
    case 'w': mainBladeSpeed += 10.0f; break;
    case 's': mainBladeSpeed -= 10.0f; break;
    case 'a': tailBladeSpeed += 10.0f; break;
    case 'd': tailBladeSpeed -= 10.0f; break;
    case 'q': exit(0); break;
    }
    glutPostRedisplay();
}

void SpecialKeyboard(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_RIGHT:
        targetCameraXAngle -= glm::radians(5.0f); // 카메라 각도 5도 감소
        break;
    case GLUT_KEY_LEFT:
        targetCameraXAngle += glm::radians(5.0f); // 카메라 각도 5도 증가
        break;
    case GLUT_KEY_UP:
        targetCameraYAngle += glm::radians(5.0f); // 카메라 각도 5도 감소
        break;
    case GLUT_KEY_DOWN:
        targetCameraYAngle -= glm::radians(5.0f); // 카메라 각도 5도 증가
        break;
    }
}

void Timer(int value) {

    // X 각도 보간
    cameraAngle = glm::mix(cameraAngle, targetCameraXAngle, interpSpeed * Time::DeltaTime());

    // Y 각도 보간 추가
    cameraYAngle = glm::mix(cameraYAngle, targetCameraYAngle, interpSpeed * Time::DeltaTime());

    glutPostRedisplay();
    glutTimerFunc(targetFrameDelay, Timer, 0);
}

void Mouse(int button, int state, int x, int y) {
    ImGui_ImplGLUT_MouseFunc(button, state, x, y);

    // ImGui가 마우스를 사용하지 않을 때만 사용자의 마우스 로직 처리
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse)
    {
        if (button == GLUT_RIGHT_BUTTON) // 우클릭
        {
            if (state == GLUT_DOWN) // 눌림
            {
                rightClickDown = true;
                lastMouseX = x; // 현재 위치 저장
                lastMouseY = y;
            }
            else if (state == GLUT_UP) // 떼짐
            {
                rightClickDown = false;
            }
        }
    }
}

void Motion(int x, int y) {
    ImGui_ImplGLUT_MotionFunc(x, y);

    // ImGui가 마우스를 사용하지 않을 때만 사용자의 마우스 로직 처리
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse)
    {
        // 우클릭 드래그 중인 경우
        if (rightClickDown)
        {
            // 마우스 이동량 계산
            int deltaX = x - lastMouseX;
            int deltaY = y - lastMouseY;

            // X 이동량으로 X-Z 평면 각도 업데이트 (Y축 기준 회전)
            // 화면 우측 이동 -> targetCameraXAngle 감소 (시계 방향 회전)
            targetCameraXAngle += (float)deltaX * rotationSpeed;

            // Y 이동량으로 고도 각도 업데이트
            // 화면 위로 이동 -> targetCameraYAngle 증가 (카메라 상승)
            targetCameraYAngle += (float)deltaY * rotationSpeed;

            // 고도각 (Y) 제한 (예: 수직에서 너무 가까워지는 것 방지)
            // GLM_PI/2는 90도 라디안. 89도로 제한
            float limit = glm::radians(89.0f);
            targetCameraYAngle = glm::clamp(targetCameraYAngle, -limit, limit);

            // 다음 프레임을 위해 현재 위치 저장
            lastMouseX = x;
            lastMouseY = y;

            // 변경 사항을 반영하기 위해 화면 다시 그리기 요청
            glutPostRedisplay();
        }
    }
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


GLuint loadCubemap(std::vector<std::string> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        // 큐브맵은 y축이 뒤집히지 않으므로 stbi_set_flip_vertically_on_load(false)
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            // 텍스처 타겟: GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
            std::cout << "큐브맵 텍스처 로드 성공: " << faces[i] << std::endl;
        }
        else
        {
            std::cerr << "큐브맵 텍스처 로드 실패: " << faces[i] << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID); // 실패 시 생성된 텍스처 삭제
            return 0; // 0 반환하여 실패 알림
        }
    }

    // 텍스처 파라미터 설정
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(true); // 다른 텍스처 로드를 위해 복원 (선택적)
    return textureID;
}


void MakeSkyboxVertexShader() {
    GLchar* vertexSource = filetobuf("skyboxVertex.glsl"); // 1단계에서 만든 파일
    if (!vertexSource) {
        std::cerr << "ERROR: skyboxVertex.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }

    vertexSkyboxShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexSkyboxShader, 1, (const GLchar**)&vertexSource, NULL);
    glCompileShader(vertexSkyboxShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexSkyboxShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexSkyboxShader, 512, NULL, errorLog);
        std::cerr << "ERROR: skybox vertex shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(vertexSource);
}

void MakeSkyboxFragmentShader() {
    GLchar* fragmentSource = filetobuf("skyboxFrag.glsl"); // 1단계에서 만든 파일
    if (!fragmentSource) {
        std::cerr << "ERROR: skybox.frag 파일을 읽지 못했습니다." << std::endl;
        return;
    }

    fragmentSkyboxShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentSkyboxShader, 1, (const GLchar**)&fragmentSource, NULL);
    glCompileShader(fragmentSkyboxShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentSkyboxShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragmentSkyboxShader, 512, NULL, errorLog);
        std::cerr << "ERROR: skybox fragment shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(fragmentSource);
}

GLuint MakeSkyboxShaderProgram() {
    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexSkyboxShader);
    glAttachShader(shaderID, fragmentSkyboxShader);
    glLinkProgram(shaderID);

    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: skybox shader program 연결 실패\n" << errorLog << std::endl;
        return 0;
    }

    // 스카이박스 셰이더는 삭제하지 않고 유지 (vertexShader, fragmentShader는 삭제됨)
    // glDeleteShader(vertexSkyboxShader); 
    // glDeleteShader(fragmentSkyboxShader);
    // glUseProgram(shaderID); // 바로 사용하지 않음
    return shaderID;
}


void InitBuffers() {  // 변경: 모델별 버퍼 초기화
    
    
    // 스카이박스 버텍스 (위치만 필요)
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    // layout (location = 0) in vec3 aPos;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    // --- Body 버퍼 ---
    glGenVertexArrays(1, &VAO_Body);
    glBindVertexArray(VAO_Body);

    glGenBuffers(1, &VBO_Body);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Body);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO_Body);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Body);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    const GLsizei stride = 8 * sizeof(GLfloat);  // *** 8로 변경 ***
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);  // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));  // color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(GLfloat)));  // *** UV 추가 (인덱스 2) ***
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // --- Blade 버퍼 ---
    glGenVertexArrays(1, &VAO_Blade);
    glBindVertexArray(VAO_Blade);

    glGenBuffers(1, &VBO_Blade);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Blade);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO_Blade);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Blade);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(GLfloat)));  // *** UV 추가 (인덱스 2) ***
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // --- Tail 버퍼 ---
    glGenVertexArrays(1, &VAO_Tail);
    glBindVertexArray(VAO_Tail);

    glGenBuffers(1, &VBO_Tail);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Tail);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO_Tail);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Tail);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(GLfloat)));  // *** UV 추가 (인덱스 2) ***
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // --- 축 데이터 (기존 그대로) ---
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
    glBufferData(GL_ARRAY_BUFFER, axesData.size() * sizeof(GLfloat), axesData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}