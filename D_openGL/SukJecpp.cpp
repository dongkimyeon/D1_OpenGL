#include "CommonInclude.h"

std::uniform_real_distribution <float> randomSpeed(0.2f, 1.5f);
std::uniform_real_distribution <float> randomMaxHeight(4.0f, 6.0f);
std::uniform_real_distribution <float> randomMinHeight(1.0f, 2.0f);
std::uniform_int_distribution <int> randomInt(0, 1);
std::random_device rd;

//구조체
struct OBB {
    glm::vec3 center = glm::vec3(0.0f);     // OBB의 중심 (월드 좌표계)
    glm::vec3 u[3];                         // OBB의 세 정규직교 축 (u[0]=x축, u[1]=y축, u[2]=z축)
    glm::vec3 half_length = glm::vec3(0.0f); // 각 축을 따른 중심으로부터의 반치수 (Local Space)
};

struct ExtendedModel {
    Model baseModel;
    glm::vec3 center; // 미로: 월드 XZ 위치, 플레이어: 월드 XYZ 위치
    GLuint indexCount = 0;
    std::vector<GLuint> indices;
    float minHeight = randomMinHeight(rd);
    float MaxHeight = randomMaxHeight(rd);
    float height = minHeight; // 미로 큐브: Y축 높이, 플레이어: 균일 스케일
    float speed = randomSpeed(rd);
    bool renderFlag = true;

    // OBB 관련 추가 항목 (첫 번째 파일에서 가져옴)
    OBB local_obb;  // 모델링 시점의 로컬 OBB (Model Matrix가 Identity일 때)
    OBB world_obb;  // 현재 프레임의 월드 OBB (Model Matrix 적용 후)
    bool is_colliding = false; // 충돌 상태
};

void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
glm::vec3 CalculateModelCenter(Model* model);
void InitBuffers();
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);
void Mouse(int button, int state, int x, int y);
void SpecialKeyboard(int key, int x, int y);
void Motion(int x, int y);
void update_world_obb(ExtendedModel& model); 
bool check_obb_collision(const ExtendedModel& modelA, const ExtendedModel& modelB); 
bool is_separated(const OBB& a, const OBB& b, const glm::vec3& axis);
void ComputeLocalOBB(ExtendedModel& model); 
//void RenderOBB(const OBB& obb, glm::vec3 color); 
void ResetAll(); // 모든 큐브와 플레이어 초기화 함수


bool CheckPlayerCollision(const ExtendedModel& playerModel);



bool IsBlockingView(const glm::vec3& cubeCenter, const glm::vec3& cameraPos, const glm::vec3& playerPos) {

    glm::vec3 camToPlayer = playerPos - cameraPos;
    float camToPlayerDist = glm::length(camToPlayer);

    if (camToPlayerDist < 0.01f) return false;

    glm::vec3 camToPlayerDir = glm::normalize(camToPlayer);


    glm::vec3 camToCube = cubeCenter - cameraPos;
    float camToCubeDist = glm::length(camToCube);


    if (camToCubeDist >= camToPlayerDist) return false;


    float projLength = glm::dot(camToCube, camToPlayerDir);


    if (projLength < 0.0f) return false;


    glm::vec3 projPoint = cameraPos + camToPlayerDir * projLength;


    float distToCube = glm::length(cubeCenter - projPoint);

 
    float threshold = 0.8f;

    return distToCube < threshold;
}

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
//선형보간 속도
float interpSpeed = 10.0f;

float playerScale = 0.2f;
// 미로 VAO, VBO, EBO
GLuint MazeVAO, MazeVBO, MazeEBO;
// 플레이어 VAO, VBO, EBO
GLuint PlayerVAO, PlayerVBO, PlayerEBO;
int n = 0; // 큐브 개수 
int m = 0;

glm::vec3 mazeMoveVec = glm::vec3(0.0f, 0.0f, 0.0f);

// n x m 큐브 모델 배열
ExtendedModel** CubeModels;
ExtendedModel Player;

//초기 시작 애니메이션 관련 변수 
bool enterAnimation = true;
float yMoveValue = -5.0f;

//카메라 관련 변수 
float cameraAngle = glm::atan(1.0f);
float targetCameraAngle = glm::atan(1.0f);

glm::vec3 cameraPos = glm::vec3(0.0f, 23.5f, 23.5f);
glm::vec3 centerPoint = glm::vec3(0.0f, 0.0f, 0.0f);


int lastMouseX = 0;          // 마지막 마우스 X 위치
int lastMouseY = 0;          // 마지막 마우스 Y 위치

//명령어 변수
bool mFlag = false;
bool oFlag = true;
bool sFlag = false;
bool rFlag = false;

// 플레이어 위치 
glm::vec3 playerPosition = glm::vec3(0.0f, 10.0f, 0.0f); // 플레이어 월드 위치

// --- 플레이어 이동 방향 벡터 ---
glm::vec3 playerMoveDir = glm::vec3(0.0f);
glm::vec3 playerFacing = glm::vec3(0.0f, 0.0f, -1.0f); // 플레이어가 바라보는 방향 (초기: -Z 방향)
float playerYaw = 0.0f;


float groundY = 1.0f; // 지면 Y 좌표
float gravity = 9.8f; // 중력 가속도


int cameraMode = 0; // 0: 처음모드 1: 객체 1인칭 모드 2: 객체 3인칭 모드 

void CubeSpeedUp() {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            CubeModels[i][j].speed += 0.1f;
        }
    }
}
void CubeSpeedDown() {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (CubeModels[i][j].speed > 0.1f) {
                CubeModels[i][j].speed -= 0.1f;
            }

        }
    }
}
void InitBuffers() {
    const GLsizei stride = 6 * sizeof(GLfloat);  // 위치(3) + 색상(3)

    glGenVertexArrays(1, &MazeVAO);
    glBindVertexArray(MazeVAO);
    glGenBuffers(1, &MazeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, MazeVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &MazeEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MazeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenVertexArrays(1, &PlayerVAO);
    glBindVertexArray(PlayerVAO);
    glGenBuffers(1, &PlayerVBO);
    glBindBuffer(GL_ARRAY_BUFFER, PlayerVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &PlayerEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PlayerEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // --- 추가된 부분: 플레이어 모델 데이터를 VBO/EBO에 한 번 버퍼링 ---
    std::vector<GLfloat> playerVertexData;
    std::vector<GLuint> playerIndices;
    GLuint playerVertexBase = 0;

    Model& pMdl = Player.baseModel;
    glm::vec3 pCenter = Player.center; // Player.center는 CalculateModelCenter로 계산된 로컬 중심
    // 플레이어 색상 (예: 흰색)
    float pr = 1.0f; float pg = 1.0f; float pb = 1.0f;

    for (size_t v = 0; v < pMdl.vertex_count; ++v) {
        Vertex vert = pMdl.vertices[v];
        float x = vert.x - pCenter.x;
        float y = vert.y - pCenter.y;
        float z = vert.z - pCenter.z;
        playerVertexData.push_back(x);
        playerVertexData.push_back(y);
        playerVertexData.push_back(z);
        playerVertexData.push_back(pr);
        playerVertexData.push_back(pg);
        playerVertexData.push_back(pb);
    }

    for (size_t f = 0; f < pMdl.face_count; ++f) {
        playerIndices.push_back(static_cast<GLuint>(pMdl.faces[f].v1) + playerVertexBase);
        playerIndices.push_back(static_cast<GLuint>(pMdl.faces[f].v2) + playerVertexBase);
        playerIndices.push_back(static_cast<GLuint>(pMdl.faces[f].v3) + playerVertexBase);
    }
    // 그릴 인덱스 개수 저장
    Player.indexCount = playerIndices.size();

    // 데이터 버퍼링
    glBindVertexArray(PlayerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, PlayerVBO);
    glBufferData(GL_ARRAY_BUFFER, playerVertexData.size() * sizeof(GLfloat), playerVertexData.data(), GL_STATIC_DRAW); // STATIC_DRAW
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PlayerEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, playerIndices.size() * sizeof(GLuint), playerIndices.data(), GL_STATIC_DRAW); // STATIC_DRAW
    glBindVertexArray(0);
    // --- 플레이어 데이터 버퍼링 끝 ---
}

void main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    width = 800; // 적절한 값으로 초기화     
    height = 600;
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("3D Maze");

    glewExperimental = GL_TRUE;
    glewInit();

    MakeVertexShaders(); // 버텍스 셰이더 생성
    MakeFragmentShaders(); // 프래그먼트 셰이더 생성
    shaderProgramID = MakeShaderProgram(); // 셰이더 프로그램 생성

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard);
    glutTimerFunc(16, Timer, 0);
    glutMouseFunc(Mouse);
	glutPassiveMotionFunc(Motion);


	Input::Initialize();
    std::cin >> n >> m;
    //n x m 큐브 모델 동적 할당
    CubeModels = new ExtendedModel * [n];
    for (int i = 0; i < n; ++i) {
        CubeModels[i] = new ExtendedModel[m];
        for (int j = 0; j < m; ++j) {
            read_obj_file("cube.obj", &CubeModels[i][j].baseModel);
      
            const float spacing = 1.0f;
            glm::vec3 worldTranslation = glm::vec3((j - (m - 1) / 2.0f) * spacing,
                0.0f,
                (i - (n - 1) / 2.0f) * spacing);
            CubeModels[i][j].center = worldTranslation; // 월드 XZ 위치 저장
      

            CubeModels[i][j].speed = randomSpeed(rd);
            ComputeLocalOBB(CubeModels[i][j]);
        }
    }

    read_obj_file("cube.obj", &Player.baseModel);

    Player.center = CalculateModelCenter(&Player.baseModel); 
    Player.speed = 1.0f;
    Player.height = playerScale;


    // 플레이어 OBB 초기화
    ComputeLocalOBB(Player);
    InitBuffers(); // 버퍼 초기화
    glutMainLoop();
}


void ComputeLocalOBB(ExtendedModel& model) {
    if (model.baseModel.vertex_count == 0) return;

    // AABB 계산 (모든 정점 대상)
    glm::vec3 min_v = glm::vec3(FLT_MAX);
    glm::vec3 max_v = glm::vec3(-FLT_MAX);
    for (size_t i = 0; i < model.baseModel.vertex_count; ++i) {
        Vertex v = model.baseModel.vertices[i];
        min_v.x = glm::min(min_v.x, v.x);
        min_v.y = glm::min(min_v.y, v.y);
        min_v.z = glm::min(min_v.z, v.z);
        max_v.x = glm::max(max_v.x, v.x);
        max_v.y = glm::max(max_v.y, v.y);
        max_v.z = glm::max(max_v.z, v.z);
    }

    // OBB 설정 
    glm::vec3 local_center = (min_v + max_v) * 0.5f; // (참고용: obj 파일의 AABB 중심)
    glm::vec3 local_half_length = (max_v - min_v) * 0.5f;

  
    model.local_obb.center = glm::vec3(0.0f);


    model.local_obb.half_length = local_half_length;
    model.local_obb.u[0] = glm::vec3(1.0f, 0.0f, 0.0f);
    model.local_obb.u[1] = glm::vec3(0.0f, 1.0f, 0.0f);
    model.local_obb.u[2] = glm::vec3(0.0f, 0.0f, 1.0f);
}

// OBB 렌더링 함수
//void RenderOBB(const OBB& obb, glm::vec3 color) {
//    // 8 꼭짓점 계산
//    glm::vec3 corners[8];
//    corners[0] = obb.center - obb.half_length.x * obb.u[0] - obb.half_length.y * obb.u[1] - obb.half_length.z * obb.u[2];
//    corners[1] = obb.center + obb.half_length.x * obb.u[0] - obb.half_length.y * obb.u[1] - obb.half_length.z * obb.u[2];
//    corners[2] = obb.center + obb.half_length.x * obb.u[0] + obb.half_length.y * obb.u[1] - obb.half_length.z * obb.u[2];
//    corners[3] = obb.center - obb.half_length.x * obb.u[0] + obb.half_length.y * obb.u[1] - obb.half_length.z * obb.u[2];
//    corners[4] = obb.center - obb.half_length.x * obb.u[0] - obb.half_length.y * obb.u[1] + obb.half_length.z * obb.u[2];
//    corners[5] = obb.center + obb.half_length.x * obb.u[0] - obb.half_length.y * obb.u[1] + obb.half_length.z * obb.u[2];
//    corners[6] = obb.center + obb.half_length.x * obb.u[0] + obb.half_length.y * obb.u[1] + obb.half_length.z * obb.u[2];
//    corners[7] = obb.center - obb.half_length.x * obb.u[0] + obb.half_length.y * obb.u[1] + obb.half_length.z * obb.u[2];
//
//    // 선 데이터: 12 에지 (24 버텍스, 각 에지 양 끝)
//    std::vector<GLfloat> lineVertices;
//    // 에지 정의: (0-1, 1-2, 2-3, 3-0, 4-5, 5-6, 6-7, 7-4, 0-4, 1-5, 2-6, 3-7)
//    std::vector<std::pair<int, int>> edges = {
//        {0, 1}, {1, 2}, {2, 3}, {3, 0},
//        {4, 5}, {5, 6}, {6, 7}, {7, 4},
//        {0, 4}, {1, 5}, {2, 6}, {3, 7}
//    };
//
//    for (const auto& edge : edges) {
//        // 시작점
//        lineVertices.push_back(corners[edge.first].x);
//        lineVertices.push_back(corners[edge.first].y);
//        lineVertices.push_back(corners[edge.first].z);
//        lineVertices.push_back(color.x);
//        lineVertices.push_back(color.y);
//        lineVertices.push_back(color.z);
//        // 끝점
//        lineVertices.push_back(corners[edge.second].x);
//        lineVertices.push_back(corners[edge.second].y);
//        lineVertices.push_back(corners[edge.second].z);
//        lineVertices.push_back(color.x);
//        lineVertices.push_back(color.y);
//        lineVertices.push_back(color.z);
//    }
//
//    // 임시 VAO/VBO 생성 및 바인딩 (간단 구현, 매 프레임 재생성)
//    GLuint tempVAO, tempVBO;
//    glGenVertexArrays(1, &tempVAO);
//    glBindVertexArray(tempVAO);
//    glGenBuffers(1, &tempVBO);
//    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
//    glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(GLfloat), lineVertices.data(), GL_STATIC_DRAW);
//
//    const GLsizei stride = 6 * sizeof(GLfloat);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
//    glEnableVertexAttribArray(1);
//
//    // 선 그리기 (GL_LINES, 24 버텍스)
//    glDrawArrays(GL_LINES, 0, 24);
//
//    // 정리
//    glBindVertexArray(0);
//    glDeleteBuffers(1, &tempVBO);
//    glDeleteVertexArrays(1, &tempVAO);
//}

void ResetAll()
{
    // 모든 큐브와 플레이어 초기화
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            CubeModels[i][j].minHeight = randomMinHeight(rd);
            CubeModels[i][j].MaxHeight = randomMaxHeight(rd);
            CubeModels[i][j].height = CubeModels[i][j].minHeight;
            CubeModels[i][j].speed = randomSpeed(rd);
            CubeModels[i][j].renderFlag = true;
        }
    }
    playerPosition = glm::vec3(0.0f, 10.0f, 0.0f);
    Player.center = CalculateModelCenter(&Player.baseModel);
    cameraAngle = glm::atan(1.0f);
    targetCameraAngle = glm::atan(1.0f);
    cameraPos = glm::vec3(0.0f, 23.5f, 23.5f);
    centerPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    playerMoveDir = glm::vec3(0.0f);
    playerFacing = glm::vec3(0.0f, 0.0f, -1.0f);
    playerYaw = 0.0f;
    enterAnimation = true;
	yMoveValue = -5.0f;

     mFlag = false;
     oFlag = true;
     sFlag = false;
     rFlag = false;
}

void MakeVertexShaders()
{
    GLchar* vertexSource;
    //--- 버텍스 세이더 읽어 저장하고 컴파일 하기
    //--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
    vertexSource = filetobuf("vertex3D.glsl");
    if (vertexSource == NULL) {
        std::cerr << "ERROR: vertex.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

void MakeFragmentShaders()
{
    GLchar* fragmentSource;
    //--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
    fragmentSource = filetobuf("fragment.glsl"); // 프래그세이더 읽어오기
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

GLuint MakeShaderProgram()
{
    GLuint shaderID;

    GLint result;
    GLchar errorLog[512];

    shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
    glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
    glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
    glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기
    glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
    glDeleteShader(fragmentShader);
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기

    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        return false;  // 수정: GLuint 반환 타입이지만 false 반환 시 0으로 처리됨
    }

    glUseProgram(shaderID);
    return shaderID;
}

// --- 수정된 부분: OBB 업데이트 함수  ---
void update_world_obb(ExtendedModel& model) {

    glm::mat4 modelMat = glm::mat4(1.0f);

    if (&model == &Player) {
        // --- 플레이어 변환 로직 (DrawScene과 일치) ---
        // 1. 월드 위치로 이동 + Y=0.25 (플레이어 바닥을 Y=0에 맞춤)
        // Player.height는 0.5이므로, half-length는 0.25입니다.
        modelMat = glm::translate(modelMat, model.center + glm::vec3(0.0f, 0.25f, 0.0f));
        // 2. 균일 스케일 적용 (model.height = 0.5f)
        modelMat = glm::scale(modelMat, glm::vec3(model.height));
    }
    else {
        // --- 미로 큐브 변환 로직 (DrawScene과 일치) ---
        // 1. Enter Animation (월드 Y 이동)
        if (enterAnimation) {
            modelMat = glm::translate(modelMat, glm::vec3(0.0f, yMoveValue, 0.0f));
        }

        // 2. 월드 XZ 위치로 이동 + Y=0.5 만큼 월드 이동 (FIX: 바닥을 Y=0에 맞춤)
        // model.center는 XZ 위치만 담고 있습니다. 여기에 +0.5 Y를 추가합니다.
        modelMat = glm::translate(modelMat, model.center + glm::vec3(0.0f, 0.5f, 0.0f));

        // 3. Y축 변환 (로컬 기준 - 바닥 고정)
        modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.5f, 0.0f));
        modelMat = glm::scale(modelMat, glm::vec3(1.0f, -model.height, 1.0f));
        modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.5f, 0.0f));
    }

    // --- (이하 OBB 계산 로직은 동일) ---

    // 모델 행렬의 회전/스케일 부분 (3x3) 추출
    glm::mat3 rotation_scale_mat = glm::mat3(modelMat);

    // 1. 월드 중심 변환
    glm::vec4 local_center_h = glm::vec4(model.local_obb.center, 1.0f);
    model.world_obb.center = glm::vec3(modelMat * local_center_h);

    // 2. 월드 축 변환
    for (int i = 0; i < 3; i++) {
        model.world_obb.u[i] = glm::normalize(rotation_scale_mat * model.local_obb.u[i]);
    }

    // 3. 반치수 설정: 스케일 벡터를 추출하여 local_obb에 적용
    glm::vec3 scale_factors = glm::vec3(
        glm::length(rotation_scale_mat[0]), // X축 스케일
        glm::length(rotation_scale_mat[1]), // Y축 스케일
        glm::length(rotation_scale_mat[2])  // Z축 스케일
    );

    // 로컬 반치수에 스케일 팩터를 곱하여 월드 반치수 설정
    model.world_obb.half_length = model.local_obb.half_length * scale_factors;
}

// 분리 검사 함수 
bool is_separated(const OBB& a, const OBB& b, const glm::vec3& axis) {
    // 축의 길이가 너무 작으면 무시 (부동소수점 오차 처리)
    if (glm::length(axis) < 1e-6) return false;

    // 두 OBB 중심 사이의 벡터
    glm::vec3 T = b.center - a.center;

    // 1. 중심 간 거리 투영
    float distance_proj = glm::abs(glm::dot(T, axis));

    // 2. OBB A의 '반지름' 투영 (각 로컬 축의 투영 길이의 합)
    float radius_a =
        glm::abs(glm::dot(a.half_length.x * a.u[0], axis)) +
        glm::abs(glm::dot(a.half_length.y * a.u[1], axis)) +
        glm::abs(glm::dot(a.half_length.z * a.u[2], axis));

    // 3. OBB B의 '반지름' 투영
    float radius_b =
        glm::abs(glm::dot(b.half_length.x * b.u[0], axis)) +
        glm::abs(glm::dot(b.half_length.y * b.u[1], axis)) +
        glm::abs(glm::dot(b.half_length.z * b.u[2], axis));

    // 중심 간 거리가 두 반지름의 합보다 크면 분리 (충돌 아님)
    return distance_proj > (radius_a + radius_b);
}

// OBB 충돌 검사 함수 
bool check_obb_collision(const ExtendedModel& modelA, const ExtendedModel& modelB) {
    const OBB& a = modelA.world_obb;
    const OBB& b = modelB.world_obb;

    // 1. OBB A의 축 3개 검사
    for (int i = 0; i < 3; i++) {
        if (is_separated(a, b, a.u[i])) return false;
    }

    // 2. OBB B의 축 3개 검사
    for (int i = 0; i < 3; i++) {
        if (is_separated(a, b, b.u[i])) return false;
    }

    // 3. 교차 축 (A_i x B_j) 9개 검사
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            glm::vec3 cross_axis = glm::cross(a.u[i], b.u[j]);
            if (is_separated(a, b, cross_axis)) return false;
        }
    }

    // 15개 축 모두에서 분리되지 않았다면 충돌!
    return true;
}


void DrawScene() {
    float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z); // XZ plane distance
    cameraPos.x = r * glm::cos(cameraAngle);
    cameraPos.z = r * glm::sin(cameraAngle);


    glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    std::vector<GLfloat> vertexData; // x,y,z,r,g,b
    std::vector<GLuint> indices;

    GLuint vertexBase = 0;
    const float spacing = 1.0f;


    std::vector<int> cubeIndexCounts;
    GLuint currentIndexOffset = 0;  // EBO offset 추적 

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            Model& mdl = CubeModels[i][j].baseModel;
            // --- 수정: center를 로컬 중심으로 사용 (VBO 생성 시점) ---
            glm::vec3 localCenter = CalculateModelCenter(&mdl); // (0,0,0) 가정

            float r = (float)i / std::max(1, n - 1);
            float g = (float)j / std::max(1, m - 1);
            float b = 0.5f;

       
            // add vertices
            for (size_t v = 0; v < mdl.vertex_count; ++v) {
                Vertex vert = mdl.vertices[v];
                // --- 수정: 로컬 중심(localCenter) 기준으로 VBO 생성 (월드 XZ 이동 제거) ---
                float x = vert.x - localCenter.x;
                float y = vert.y - localCenter.y;
                float z = vert.z - localCenter.z;
                vertexData.push_back(x);
                vertexData.push_back(y);
                vertexData.push_back(z);
                vertexData.push_back(r);
                vertexData.push_back(g);
                vertexData.push_back(b);
            }
            // --- VBO 생성 수정 끝 ---

            int thisCubeIndexCount = 0;  // 이 큐브의 index 수
            for (size_t f = 0; f < mdl.face_count; ++f) {
                // assume faces are zero-based indices
                indices.push_back(static_cast<GLuint>(mdl.faces[f].v1) + vertexBase);
                indices.push_back(static_cast<GLuint>(mdl.faces[f].v2) + vertexBase);
                indices.push_back(static_cast<GLuint>(mdl.faces[f].v3) + vertexBase);
                thisCubeIndexCount += 3;
            }
            cubeIndexCounts.push_back(thisCubeIndexCount);  // 저장

            vertexBase += static_cast<GLuint>(mdl.vertex_count);
        }
    }

    glBindVertexArray(MazeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, MazeVBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MazeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);


    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

    // 1. 메인 뷰: 전체 화면 렌더링 (기존 카메라)
    glViewport(0, 0, width, height);  // 전체 뷰포트
	glm::mat4 mainView = glm::mat4(1.0f);
    if (cameraMode == 0)
    {
         mainView = glm::lookAt(cameraPos, centerPoint, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (cameraMode == 1)
    {
        // 1인칭 카메라 (수정 필요)
        glm::vec3 cameraPosition = playerPosition + glm::vec3(0.0f, 0.5f, 0.0f); // 플레이어 눈 높이
        glm::vec3 cameraTarget = cameraPosition + playerFacing; // 바라보는 방향
        mainView = glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (cameraMode == 2)
    {
        // 3인칭 카메라 (수정 필요)
        glm::vec3 cameraOffset = -playerFacing * 1.2f + glm::vec3(0.0f, 0.75, 0.0f); // 뒤쪽 + 위
        glm::vec3 cameraPosition = playerPosition + cameraOffset;
        glm::vec3 cameraTarget = playerPosition + glm::vec3(0.0f, 0.5f, 0.0f);
        mainView = glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    
    glm::mat4 mainProj;
    if (oFlag) {
        //원근
        mainProj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

    }
    else {
        //직교
        mainProj = glm::ortho(-10.0f, 10.0f, -10.0f * (float)height / (float)width, 10.0f * (float)height / (float)width, 0.1f, 100.0f);

    }
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mainView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(mainProj));

    int cubeIdx = 0;
    currentIndexOffset = 0;
    glBindVertexArray(MazeVAO);

    // --- 3인칭 카메라 위치 계산 (렌더링 전에 미리 계산) ---
    glm::vec3 actualCameraPos = cameraPos; // 기본값
    if (cameraMode == 2) {
        glm::vec3 cameraOffset = -playerFacing * 1.5f + glm::vec3(0.0f, 1.0f, 0.0f);
        actualCameraPos = playerPosition + cameraOffset;
    }
    // --- 계산 끝 ---

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            // --- 추가: 3인칭 모드에서 시야를 가리는 큐브는 스킵 ---
            if (cameraMode == 2 && CubeModels[i][j].renderFlag) {
                // 큐브 중심 위치 계산 (Y축 높이 고려)
                glm::vec3 cubeWorldCenter = CubeModels[i][j].center + glm::vec3(0.0f, CubeModels[i][j].height * 0.5f, 0.0f);

                if (IsBlockingView(cubeWorldCenter, actualCameraPos, playerPosition)) {
                    
                    cubeIdx++;
                    currentIndexOffset += cubeIndexCounts[cubeIdx - 1];
                    continue;
                }
            }
       

            glm::mat4 modelMat = glm::mat4(1.0f);

            if (enterAnimation) {
                modelMat = glm::translate(modelMat, glm::vec3(0.0f, yMoveValue, 0.0f));
            }

            modelMat = glm::translate(modelMat, CubeModels[i][j].center + glm::vec3(0.0f, 0.5f, 0.0f));

            modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.5f, 0.0f));
            modelMat = glm::scale(modelMat, glm::vec3(1.0f, -CubeModels[i][j].height, 1.0f));
            modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.5f, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
            int thisCubeIndexCount = cubeIndexCounts[cubeIdx];
            if (CubeModels[i][j].renderFlag)
            {
                glDrawElements(GL_TRIANGLES, thisCubeIndexCount, GL_UNSIGNED_INT,
                    (void*)(currentIndexOffset * sizeof(GLuint)));
            }

            currentIndexOffset += thisCubeIndexCount;
            cubeIdx++;
        }
    }

    // --- 플레이어 렌더링 ---

    glm::mat4 playerModelMat = glm::translate(glm::mat4(1.0f), playerPosition + glm::vec3(0.0f, 0.25f, 0.0f));
	playerModelMat = glm::rotate(playerModelMat, -playerYaw, glm::vec3(0.0f, 1.0f, 0.0f));
    playerModelMat = glm::scale(playerModelMat, glm::vec3(Player.height));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(playerModelMat));

    glBindVertexArray(PlayerVAO);
    if (sFlag)
    {
        glDrawElements(GL_TRIANGLES, Player.indexCount, GL_UNSIGNED_INT, (void*)0);
    }

    

    glBindVertexArray(0);

    //glm::mat4 identityMat = glm::mat4(1.0f);
    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identityMat));
    //// --- OBB 렌더링 ---
    //for (int i = 0; i < n; ++i) {
    //    for (int j = 0; j < m; ++j) {
    //        if (CubeModels[i][j].renderFlag) {
    //            // 충돌 시 노란색, 아닐 시 빨간색
    //            glm::vec3 mazeColor = CubeModels[i][j].is_colliding ? glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
    //            RenderOBB(CubeModels[i][j].world_obb, mazeColor);
    //        }
    //    }
    //}
        
        
    
  

    // 미니맵
    int miniWidth = 200;
    int miniHeight = 200;
    int miniX = width - miniWidth;         // 오른쪽 시작
    int miniY = height - miniHeight;       // 위쪽 시작

  
    glDisable(GL_DEPTH_TEST);  
    glEnable(GL_SCISSOR_TEST); 
    glScissor(miniX, miniY, miniWidth, miniHeight); 
    glViewport(miniX, miniY, miniWidth, miniHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);  

    // 탑뷰 카메라/프로젝션
    glm::mat4 topView = glm::lookAt(glm::vec3(0.0f, (float)std::max(n, m) + 10.0f, 0.0f), centerPoint, glm::vec3(1.0f, 0.0f, 0.0f));
    topView = glm::rotate(topView, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 topProj = glm::perspective(glm::radians(45.0f), (float)miniWidth / (float)miniHeight, 0.1f, 1000.0f);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(topView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(topProj));

    
    cubeIdx = 0;
    currentIndexOffset = 0;
    glBindVertexArray(MazeVAO); 
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            
            glm::mat4 modelMat = glm::mat4(1.0f);

            // 1. Enter Animation (월드 Y 이동)
            if (enterAnimation) {
                modelMat = glm::translate(modelMat, glm::vec3(0.0f, yMoveValue, 0.0f));
            }

            // 2. 월드 XZ 위치로 이동 + Y=0.5 만큼 월드 이동 (FIX: 바닥을 Y=0에 맞춤)
            modelMat = glm::translate(modelMat, CubeModels[i][j].center + glm::vec3(0.0f, 0.5f, 0.0f));

            // 3. Y축 변환 (로컬 기준 - 바닥 고정)
            modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.5f, 0.0f));
            modelMat = glm::scale(modelMat, glm::vec3(1.0f, -CubeModels[i][j].height, 1.0f));
            modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.5f, 0.0f));
            // --- 수정 끝 ---

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
            int thisCubeIndexCount = cubeIndexCounts[cubeIdx];
            if (CubeModels[i][j].renderFlag)
            {
                glDrawElements(GL_TRIANGLES, thisCubeIndexCount, GL_UNSIGNED_INT,
                    (void*)(currentIndexOffset * sizeof(GLuint)));

            }
            currentIndexOffset += thisCubeIndexCount;
            cubeIdx++;
        }
    }

    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(playerModelMat)); // 메인 뷰에서 쓴 playerModelMat 재사용
    glBindVertexArray(PlayerVAO);
   if (sFlag)
    {
        glDrawElements(GL_TRIANGLES, Player.indexCount, GL_UNSIGNED_INT, (void*)0);
    }

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glutSwapBuffers();
    glBindVertexArray(0);
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);  // 전체 뷰포트 유지
    width = w;
    height = h;
}

glm::vec3 CalculateModelCenter(Model* model) {
    if (model->vertex_count == 0) return glm::vec3(0.0f);

    glm::vec3 center(0.0f);
    for (size_t i = 0; i < model->vertex_count; ++i) {
        Vertex v = model->vertices[i];
        center.x += v.x;
        center.y += v.y;
        center.z += v.z;
    }
    center /= static_cast<float>(model->vertex_count);
    return center;
}

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'o': // 직각 투영 / 원근 투영 토글
        oFlag = !oFlag;
        break;
    case 'm': // 애니메이션 정지/재개 토글
        mFlag = !mFlag;
        break;
    case 'y':
        targetCameraAngle += glm::radians(5.0f);
        break;
    case 'Y':
        targetCameraAngle -= glm::radians(5.0f);
        break;
    case 'z':
        mazeMoveVec.z += 1.0f;
        break;
    case 'Z':
        mazeMoveVec.z -= 1.0f;
        break;
    case 'r':
    {
        rFlag = true;
        if (n < 3 || m < 3) {
            std::cout << "미로가 너무 작아요! (최소 3x3 이상)" << std::endl;
            break;
        }


        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                CubeModels[i][j].height = -0.1f;
                CubeModels[i][j].renderFlag = false;
            }
        }


        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                if (i == 0 || i == n - 1 || j == 0 || j == m - 1 || (i % 2 == 0 && j % 2 == 0)) {
                    CubeModels[i][j].height = 4.0f;
                    CubeModels[i][j].renderFlag = true;
                }
            }
        }


        int cell_rows = (n - 1) / 2;
        int cell_cols = (m - 1) / 2;
        int total_cells = cell_rows * cell_cols;


        std::vector<int> parent(total_cells);
        for (int i = 0; i < total_cells; ++i) parent[i] = i;

        auto find = [&](auto&& self, int x) -> int {
            if (parent[x] != x) parent[x] = self(self, parent[x]);
            return parent[x];
            };
        auto unite = [&](int x, int y) {
            int px = find(find, x), py = find(find, y);
            if (px != py) parent[px] = py;
            };


        struct Wall {
            int c1, c2;
            int wi, wj;
        };
        std::vector<Wall> walls;


        for (int cr = 0; cr < cell_rows; ++cr) {
            for (int cc = 0; cc < cell_cols - 1; ++cc) {
                int c1 = cr * cell_cols + cc;
                int c2 = cr * cell_cols + (cc + 1);
                int wi = 2 * cr + 1;
                int wj = 2 * (cc + 1);
                walls.push_back({ c1, c2, wi, wj });
            }
        }


        for (int cr = 0; cr < cell_rows - 1; ++cr) {
            for (int cc = 0; cc < cell_cols; ++cc) {
                int c1 = cr * cell_cols + cc;
                int c2 = (cr + 1) * cell_cols + cc;
                int wi = 2 * (cr + 1);
                int wj = 2 * cc + 1;
                walls.push_back({ c1, c2, wi, wj });
            }
        }


        for (const auto& w : walls) {
            if (w.wi < n && w.wj < m) {  // 안전 체크
                CubeModels[w.wi][w.wj].height = 4.0f;
                CubeModels[w.wi][w.wj].renderFlag = true;
            }
        }

        if (total_cells <= 1) {
            std::cout << "셀이 1개 이하라서 단순 미로 (벽 거의 없음)" << std::endl;
        }

        // 8. 랜덤 셔플
        std::mt19937 rng(rd());
        std::shuffle(walls.begin(), walls.end(), rng);

        // 9. Kruskal: 연결 안 된 벽만 제거 (walls 비어 있으면 자동 스킵)
        for (const auto& w : walls) {
            if (find(find, w.c1) != find(find, w.c2)) {
                unite(w.c1, w.c2);
                CubeModels[w.wi][w.wj].renderFlag = false;
                CubeModels[w.wi][w.wj].height = -0.1f;
            }
        }

        // 10. 입구/출구 뚫기 (항상 실행 → 작은 미로에서도 자연스럽게 뚫림)
        if (n > 2 && m > 2) {
            // 왼쪽 입구
            CubeModels[1][0].renderFlag = false;
            CubeModels[1][0].height = -0.1f;
            // 오른쪽 출구
            CubeModels[n - 2][m - 1].renderFlag = false;
            CubeModels[n - 2][m - 1].height = -0.1f;
        }

        // 미로 재생성 후 OBB 재계산
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                if (CubeModels[i][j].renderFlag) {
                    ComputeLocalOBB(CubeModels[i][j]);
                }
            }
        }

        break;
    }
    case 'v': // 육면체들이 움직임이 멈추고 낮은 높이로 변함
        mFlag = !mFlag;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                CubeModels[i][j].height = 2.0f;
            }
        }
        break;
    case 's': // 미로에 객체 생성
        if (rFlag)
        {
            sFlag = !sFlag;
            playerPosition.y = 10.0f;

            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j) {
                    if (CubeModels[i][j].renderFlag == false) {
                        playerPosition.x = CubeModels[i][j].center.x;
                        playerPosition.z = CubeModels[i][j].center.z;
                        i = n; // 바깥 루프 탈출
                        break;
                    }
                }
            }
        }
		
        break;
    case '-': // 육면체 스피드 감소
        CubeSpeedDown();
        break;
    case '=': // 육면체 스피드 증가
        CubeSpeedUp();
        break;
	case '0': 
		cameraMode = 0; // 기본 시점(쿼터뷰)   
        break;
    case '1': // 객체 시점 (1인칭)
		if (sFlag)
		    cameraMode = 1;
        break;
    case '3': // 객체 시점 (3인칭)
		if (sFlag)
		    cameraMode = 2;
        break;
    case 'c': // 모든 값 초기화
		ResetAll();
        break;
    case 'q':
        exit(0);
        break;
    }
}

// --- 추가된 부분: 플레이어 충돌 검사 헬퍼 함수 ---
bool CheckPlayerCollision(const ExtendedModel& playerModel) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (CubeModels[i][j].renderFlag && check_obb_collision(playerModel, CubeModels[i][j])) {
                return true; // 하나라도 충돌하면 즉시 true 반환
            }
        }
    }
    return false; 
}

void Timer(int value)
{
    float delta = 1.0f / 60.0f;
    cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);
    
	Input::Update();
	

    float moveSpeed = 1.0f; // 방향 설정용

    // 플레이어의 오른쪽 방향 벡터 계산 (playerFacing과 수직)
    glm::vec3 rightDir = glm::normalize(glm::cross(playerFacing, glm::vec3(0.0f, 1.0f, 0.0f)));

    playerMoveDir = glm::vec3(0.0f); // 매 프레임 초기화

    if (Input::GetKey(eKeyCode::Up))
    {
        playerMoveDir += playerFacing * moveSpeed;
    }
    if (Input::GetKey(eKeyCode::Down))
    {
        playerMoveDir += -playerFacing * moveSpeed;
    }
    if (Input::GetKey(eKeyCode::Left))
    {
        playerMoveDir += -rightDir * moveSpeed;
    }
    if (Input::GetKey(eKeyCode::Right))
    {
        playerMoveDir += rightDir * moveSpeed;
    }

    if (playerPosition.y > groundY && sFlag)
    {
        playerPosition.y -= gravity * delta;

    }
	
    // 0. 충돌 상태 플래그 초기화 (시각화용)
    Player.is_colliding = false;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            CubeModels[i][j].is_colliding = false;
        }
    }

    // 1. 미로 큐브 애니메이션 (높이 변경) - (사용자 코드 유지)
    if (!enterAnimation)
    {
        if (!mFlag)
        {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j) {
                    CubeModels[i][j].height += CubeModels[i][j].speed * delta;
                    if (CubeModels[i][j].height >= CubeModels[i][j].MaxHeight) {
                        CubeModels[i][j].height = CubeModels[i][j].MaxHeight;
                        CubeModels[i][j].speed = -randomSpeed(rd);
                    }
                    else if (CubeModels[i][j].height <= CubeModels[i][j].minHeight) {
                        CubeModels[i][j].height = CubeModels[i][j].minHeight;
                        CubeModels[i][j].speed = randomSpeed(rd);
                    }
                }
            }
        }
    }
    else
    {
        if (yMoveValue <= 0.0f) { yMoveValue += 2.0f * delta; }
        else { yMoveValue = -5.0f; enterAnimation = false; }
    }


    // 2. 미로 큐브 OBB 업데이트 (변경된 높이/애니메이션 반영)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (CubeModels[i][j].renderFlag) {
                update_world_obb(CubeModels[i][j]);
            }
        }
    }

    // --- 수정된 부분: 플레이어 이동 및 충돌 응답 (Timer로 이동) ---
    float playerMoveSpeed = 3.0f; // 초당 이동 속도
    if (glm::length(playerMoveDir) > 0.01f) {
        glm::vec3 desiredMove = glm::normalize(playerMoveDir) * playerMoveSpeed * delta;
        glm::vec3 oldPos = playerPosition;

        // 3a. X축 이동 시도
        glm::vec3 tryXPos = oldPos + glm::vec3(desiredMove.x, 0, 0);
        Player.center = tryXPos;
        Player.height = playerScale; // --- 수정: 스케일 0.5 ---
        update_world_obb(Player); // X 이동 후의 OBB
        if (!CheckPlayerCollision(Player)) {
            playerPosition.x = tryXPos.x; // 충돌 안하면 X 이동
        }

        // 3b. Z축 이동 시도 
        glm::vec3 tryZPos = playerPosition + glm::vec3(0, 0, desiredMove.z);
        Player.center = tryZPos;
        Player.height = playerScale; // --- 수정: 스케일 0.5 ---
        update_world_obb(Player); // Z 이동 후의 OBB
        if (!CheckPlayerCollision(Player)) {
            playerPosition.z = tryZPos.z; // 충돌 안하면 Z 이동
        }
    }
    // --- 수정 끝 ---

    // 4. 최종 플레이어 OBB 업데이트 (시각화 및 다음 프레임용)
    Player.center = playerPosition;
    Player.height = playerScale; // --- 수정: 스케일 0.5 ---
    update_world_obb(Player);

    // 5. 충돌 플래그 설정 (시각화용)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (CubeModels[i][j].renderFlag && check_obb_collision(Player, CubeModels[i][j])) {
                Player.is_colliding = true;
                CubeModels[i][j].is_colliding = true;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}

void Mouse(int button, int state, int x, int y)
{
  
}

void SpecialKeyboard(int key, int x, int y)
{
    float moveSpeed = 1.0f; // 방향 설정용

    // 플레이어의 오른쪽 방향 벡터 계산 (playerFacing과 수직)
    glm::vec3 rightDir = glm::normalize(glm::cross(playerFacing, glm::vec3(0.0f, 1.0f, 0.0f)));

    if (key == GLUT_KEY_UP) {
        // 앞으로 이동 (플레이어가 바라보는 방향)
        playerMoveDir = playerFacing * moveSpeed;
    }
    else if (key == GLUT_KEY_DOWN) {
        // 뒤로 이동 (플레이어가 바라보는 반대 방향)
        playerMoveDir = -playerFacing * moveSpeed;
    }
    else if (key == GLUT_KEY_LEFT) {
        // 왼쪽으로 이동 (오른쪽 방향의 반대)
        playerMoveDir = -rightDir * moveSpeed;
    }
    else if (key == GLUT_KEY_RIGHT) {
        // 오른쪽으로 이동
        playerMoveDir = rightDir * moveSpeed;
    }
}




void Motion(int x, int y)
{
    if (sFlag && cameraMode != 0)
    {

        // 화면 중앙 좌표 계산
        int centerX = width / 2;
        int centerY = height / 2;

        // 마우스 이동량 계산
        int deltaX = x - centerX;
        int deltaY = y - centerY;

        // 마우스 감도 설정
        float sensitivity = 0.005f;

        // Yaw 각도 업데이트 (좌우 회전)
        playerYaw += deltaX * sensitivity;

        // playerFacing 벡터 업데이트 (Y축 기준 회전)
        playerFacing.x = sin(playerYaw);
        playerFacing.z = -cos(playerYaw);
        playerFacing.y = 0.0f;
        playerFacing = glm::normalize(playerFacing);

        // 현재 마우스 위치 저장
        lastMouseX = x;
        lastMouseY = y;

        glutWarpPointer(centerX, centerY);

        // 화면 다시 그리기
        glutPostRedisplay();
    }
}