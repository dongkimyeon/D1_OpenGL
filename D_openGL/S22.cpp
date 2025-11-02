#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" 

// Enum
enum MoveDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// Structs
struct AABB {
    glm::vec3 min;  // 최소 좌표 (x_min, y_min, z_min)
    glm::vec3 max;  // 최대 좌표 (x_max, y_max, z_max)
};

struct ExtendedModel {
    Model baseModel;
    glm::vec3 center;
    GLuint indexCount = 0;
    std::vector<GLuint> indices;
};

// Function declarations (prototypes)
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);
void Mouse(int button, int state, int x, int y);
void SpecialKeyboard(int key, int x, int y);
void Motion(int x, int y);
glm::vec3 CalculateModelCenter(Model* model);
void DrawWalls();
void DrawObstacles();
void UpdateModelBuffers(ExtendedModel* model, GLuint vao, GLuint vbo, GLuint ebo, glm::vec3 color);

// AABB-related functions (prototypes)
bool CheckAABBCollision(const AABB& a, const AABB& b);
AABB GetCharacterAABB();
AABB GetObstacleAABB(int index);
bool HasObstacleCollision();
bool CheckJumpOverObstacle();
float GetHighestObstacleTop();  // New: Get highest colliding obstacle top Y
void DrawAABB(const AABB& box, glm::vec3 color);  // New: Draw AABB as wireframe

// Global variables
GLint width = 1280, height = 720;
GLuint shaderProgramID, vertexShader, fragmentShader;

// Human parts VAO/VBO/EBO
GLuint bodyVAO, bodyVBO, bodyEBO;
GLuint headVAO, headVBO, headEBO;
GLuint leftLegVAO, leftLegVBO, leftLegEBO;
GLuint rightLegVAO, rightLegVBO, rightLegEBO;
GLuint leftArmVAO, leftArmVBO, leftArmEBO;
GLuint rightArmVAO, rightArmVBO, rightArmEBO;
GLuint noseVAO, noseVBO, noseEBO;
GLuint axisVAO, axisVBO;  // For drawing axes

// Obstacle VAO/VBO/EBO
GLuint obstacleVAO, obstacleVBO, obstacleEBO;

// Wall drawing
GLuint wallVAO, wallVBO;

// Floor drawing
GLuint floorVAO, floorVBO, floorEBO;

// New: AABB drawing VAO/VBO 
GLuint aabbVAO, aabbVBO;

ExtendedModel Body;
ExtendedModel Head;
ExtendedModel LeftLeg;
ExtendedModel RightLeg;
ExtendedModel LeftArm;
ExtendedModel RightArm;
ExtendedModel Nose;
ExtendedModel Obstacle[3]; // Obstacles

float cameraAngle = glm::atan(1.0f);
float targetCameraAngle = glm::atan(1.0f);

// Camera position related
glm::vec3 cameraPos = glm::vec3(12.0f, 12.0f, 12.0f);
float interpSpeed = 10.0f;
glm::vec3 moveVec(0.0f, 1.1f, 0.0f); // Human overall movement vector
bool cameraOrbitAnim = false;

// Camera rotation related (Y-axis yaw rotation)
float cameraYaw = 0.0f;
float targetCameraYaw = 0.0f;

// Human animation related
float armSwingAngle = 0.0f;
float legSwingAngle = 0.0f;
MoveDirection currentMoveDir = RIGHT;

float modelMoveSpeed = 0.05f;
float modelAngle = 0.0f;

// Command flags
bool rotateFlag = false;
bool armFlag = true;
bool legFlag = true;

// Wall related variables
bool floorOpen = false;
const float ROOM_SIZE = 4.0f;
const float FLOOR_Y = -4.0f;
float wallRotationZ = 0.0f;

// Front wall slide related variables
bool frontOpen = false;
float frontWallY = FLOOR_Y;
float targetFrontWallY = FLOOR_Y;

// Obstacle positions
glm::vec3 obstaclePositions[3] = {
    glm::vec3(-2.0f, 0.5f, -2.0f),
    glm::vec3(1.5f, 0.5f, 0.0f),
    glm::vec3(2.0f, 0.5f, 2.0f)
};

// Jump related variables
bool isJumping = false;
float jumpVelocity = 0.0f;
float gravity = -20.0f;  // Adjustable via ImGui
const float jumpForce = 8.0f;
const float GROUND_Y = 1.1f;  // New: Ground level for character center

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Human Model with Room Walls");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        return -1;
    }

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

    // Load OBJ files
    read_obj_file("cube.obj", &Body.baseModel);
    read_obj_file("cube.obj", &Head.baseModel);
    read_obj_file("cube.obj", &LeftLeg.baseModel);
    read_obj_file("cube.obj", &RightLeg.baseModel);
    read_obj_file("cube.obj", &LeftArm.baseModel);
    read_obj_file("cube.obj", &RightArm.baseModel);
    read_obj_file("cube.obj", &Nose.baseModel);

    for (int i = 0; i < 3; ++i) {
        read_obj_file("cube.obj", &Obstacle[i].baseModel);
    }

    InitBuffers();

    // Update buffers
    UpdateModelBuffers(&Body, bodyVAO, bodyVBO, bodyEBO, glm::vec3(0.8f, 0.5f, 0.3f));
    UpdateModelBuffers(&Head, headVAO, headVBO, headEBO, glm::vec3(1.0f, 0.8f, 0.6f));
    UpdateModelBuffers(&LeftLeg, leftLegVAO, leftLegVBO, leftLegEBO, glm::vec3(1.0f, 0.0f, 0.0f));
    UpdateModelBuffers(&RightLeg, rightLegVAO, rightLegVBO, rightLegEBO, glm::vec3(0.5f, 0.5f, 1.0f));
    UpdateModelBuffers(&LeftArm, leftArmVAO, leftArmVBO, leftArmEBO, glm::vec3(1.0f, 0.0f, 1.0f));
    UpdateModelBuffers(&RightArm, rightArmVAO, rightArmVBO, rightArmEBO, glm::vec3(0.0f, 1.0f, 0.0f));
    UpdateModelBuffers(&Nose, noseVAO, noseVBO, noseEBO, glm::vec3(1.0f, 1.0f, 1.0f));

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard);
    glutTimerFunc(16, Timer, 0);

    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);


    // main() 함수의 glutMainLoop() 직전 (InitBuffers()나 UpdateModelBuffers() 후)에 추가
    std::cout << "=== 사용 가능한 명령어 ===" << std::endl;
    std::cout << "'a': 카메라 궤도 애니메이션 토글" << std::endl;
    std::cout << "'r' / 'R': 카메라 공전" << std::endl;
    std::cout << "'y' / 'Y': 카메라 자전" << std::endl;
    std::cout << "'z' / 'Z': 카메라 전진 / 후진 (Z축)" << std::endl;
    std::cout << "'x' / 'X': 카메라 오른쪽 / 왼쪽 이동 (X축)" << std::endl;
    std::cout << "'j': 점프 " << std::endl;
    std::cout << "'i': 위치 및 상태 초기화" << std::endl;
    std::cout << "'+' / '-': 이동 속도 증가 / 감소" << std::endl;
    std::cout << "'o' / 'O': 앞 벽 열기/닫기 토글" << std::endl;
    std::cout << "화살표 키: 캐릭터 이동 (UP/DOWN/LEFT/RIGHT)" << std::endl;
    std::cout << "'q': 프로그램 종료" << std::endl;


    glutMainLoop();

    glutMainLoop();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

// Function implementations

void MakeVertexShaders() {
    GLchar* vertexSource = filetobuf("vertex3D.glsl");
    if (!vertexSource) {
        std::cerr << "ERROR: vertex3D.glsl 파일을 읽지 못했습니다." << std::endl;
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
    const GLsizei stride = 6 * sizeof(GLfloat);  // 위치(3) + 색상(3)

    // Obstacle buffers
    glGenVertexArrays(1, &obstacleVAO);
    glBindVertexArray(obstacleVAO);
    glGenBuffers(1, &obstacleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, obstacleVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &obstacleEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obstacleEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Nose buffers
    glGenVertexArrays(1, &noseVAO);
    glBindVertexArray(noseVAO);
    glGenBuffers(1, &noseVBO);
    glBindBuffer(GL_ARRAY_BUFFER, noseVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &noseEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, noseEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Body buffers
    glGenVertexArrays(1, &bodyVAO);
    glBindVertexArray(bodyVAO);
    glGenBuffers(1, &bodyVBO);
    glBindBuffer(GL_ARRAY_BUFFER, bodyVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &bodyEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bodyEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Head buffers
    glGenVertexArrays(1, &headVAO);
    glBindVertexArray(headVAO);
    glGenBuffers(1, &headVBO);
    glBindBuffer(GL_ARRAY_BUFFER, headVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &headEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, headEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // LeftLeg buffers
    glGenVertexArrays(1, &leftLegVAO);
    glBindVertexArray(leftLegVAO);
    glGenBuffers(1, &leftLegVBO);
    glBindBuffer(GL_ARRAY_BUFFER, leftLegVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &leftLegEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leftLegEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // RightLeg buffers
    glGenVertexArrays(1, &rightLegVAO);
    glBindVertexArray(rightLegVAO);
    glGenBuffers(1, &rightLegVBO);
    glBindBuffer(GL_ARRAY_BUFFER, rightLegVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &rightLegEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rightLegEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // LeftArm buffers
    glGenVertexArrays(1, &leftArmVAO);
    glBindVertexArray(leftArmVAO);
    glGenBuffers(1, &leftArmVBO);
    glBindBuffer(GL_ARRAY_BUFFER, leftArmVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &leftArmEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leftArmEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // RightArm buffers
    glGenVertexArrays(1, &rightArmVAO);
    glBindVertexArray(rightArmVAO);
    glGenBuffers(1, &rightArmVBO);
    glBindBuffer(GL_ARRAY_BUFFER, rightArmVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &rightArmEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rightArmEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Axis drawing VAO/VBO
    glGenVertexArrays(1, &axisVAO);
    glBindVertexArray(axisVAO);
    glGenBuffers(1, &axisVBO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Floor VAO/VBO/EBO
    glGenVertexArrays(1, &floorVAO);
    glBindVertexArray(floorVAO);

    glGenBuffers(1, &floorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &floorEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Wall buffers
    glGenVertexArrays(1, &wallVAO);
    glBindVertexArray(wallVAO);

    glGenBuffers(1, &wallVBO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // New: AABB wireframe buffers (lines, dynamic)
    glGenVertexArrays(1, &aabbVAO);
    glBindVertexArray(aabbVAO);
    glGenBuffers(1, &aabbVBO);
    glBindBuffer(GL_ARRAY_BUFFER, aabbVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
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

void UpdateModelBuffers(ExtendedModel* model, GLuint vao, GLuint vbo, GLuint ebo, glm::vec3 color) {
    std::vector<GLfloat> vertexData;

    // Calculate and store center
    model->center = CalculateModelCenter(&model->baseModel);

    // Generate vertex data (centered)
    for (size_t i = 0; i < model->baseModel.vertex_count; ++i) {
        Vertex v = model->baseModel.vertices[i];
        float x = v.x - model->center.x;
        float y = v.y - model->center.y;
        float z = v.z - model->center.z;
        vertexData.push_back(x);
        vertexData.push_back(y);
        vertexData.push_back(z);
        // Color
        vertexData.push_back(color.r);
        vertexData.push_back(color.g);
        vertexData.push_back(color.b);
    }

    // Generate indices
    model->indices.clear();
    for (size_t i = 0; i < model->baseModel.face_count; ++i) {
        model->indices.push_back(model->baseModel.faces[i].v1);
        model->indices.push_back(model->baseModel.faces[i].v2);
        model->indices.push_back(model->baseModel.faces[i].v3);
    }
    model->indexCount = static_cast<GLuint>(model->indices.size());

    // Upload buffers
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices.size() * sizeof(GLuint), model->indices.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
}

// AABB functions implementations
bool CheckAABBCollision(const AABB& a, const AABB& b) {
    return (a.max.x < b.min.x || a.min.x > b.max.x ||
        a.max.y < b.min.y || a.min.y > b.max.y ||
        a.max.z < b.min.z || a.min.z > b.max.z) ? false : true;
}

AABB GetCharacterAABB() {
    AABB charBox;
    // Character center: moveVec, size: body-based (X:±0.4, Y:±1.0 jump considered, Z:±0.2)
    charBox.min = moveVec + glm::vec3(-0.4f, -1.0f, -0.2f);
    charBox.max = moveVec + glm::vec3(0.4f, 1.0f, 0.2f);
    return charBox;
}

AABB GetObstacleAABB(int index) {
    AABB obsBox;
    // Obstacle center: obstaclePositions[index], size: 0.5 scale cube (half: ±0.5)  // ±0.25 -> ±0.5로 변경
    obsBox.min = obstaclePositions[index] + glm::vec3(-0.5f, -0.5f, -0.5f);
    obsBox.max = obstaclePositions[index] + glm::vec3(0.5f, 0.5f, 0.5f);
    return obsBox;
}

bool HasObstacleCollision() {
    AABB charBox = GetCharacterAABB();
    for (int i = 0; i < 3; ++i) {
        AABB obsBox = GetObstacleAABB(i);
        if (CheckAABBCollision(charBox, obsBox)) {
            return true;
        }
    }
    return false;
}

bool CheckJumpOverObstacle() {
    AABB charBox = GetCharacterAABB();
    for (int i = 0; i < 3; ++i) {
        AABB obsBox = GetObstacleAABB(i);
        // XZ overlap but character Y min > obstacle Y max (over it) then OK
        if ((charBox.max.x >= obsBox.min.x && charBox.min.x <= obsBox.max.x &&
            charBox.max.z >= obsBox.min.z && charBox.min.z <= obsBox.max.z) &&
            charBox.min.y > obsBox.max.y) {  // Y: character's bottom above obstacle top
            return true;  // Can pass over
        }
    }
    return false;
}

// New: Get highest top Y of colliding obstacles (for Y snap)
float GetHighestObstacleTop() {
    AABB charBox = GetCharacterAABB();
    float highestTop = -100.0f;
    for (int i = 0; i < 3; ++i) {
        AABB obsBox = GetObstacleAABB(i);
        if (CheckAABBCollision(charBox, obsBox)) {
            highestTop = std::max(highestTop, obsBox.max.y);
        }
    }
    return highestTop;
}


void DrawAABB(const AABB& box, glm::vec3 color) {
    std::vector<GLfloat> lineData;
    glm::vec3 vertices[8] = {
        box.min,  // 0: min
        glm::vec3(box.max.x, box.min.y, box.min.z),  // 1
        glm::vec3(box.max.x, box.min.y, box.max.z),  // 2
        glm::vec3(box.min.x, box.min.y, box.max.z),  // 3
        glm::vec3(box.min.x, box.max.y, box.min.z),  // 4
        glm::vec3(box.max.x, box.max.y, box.min.z),  // 5
        glm::vec3(box.max.x, box.max.y, box.max.z),  // 6
        box.max   // 7: max
    };

    // 12 edges: each edge is 2 vertices (position + color)
    // Bottom face
    lineData.insert(lineData.end(), { vertices[0].x, vertices[0].y, vertices[0].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[1].x, vertices[1].y, vertices[1].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[1].x, vertices[1].y, vertices[1].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[2].x, vertices[2].y, vertices[2].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[2].x, vertices[2].y, vertices[2].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[3].x, vertices[3].y, vertices[3].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[3].x, vertices[3].y, vertices[3].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[0].x, vertices[0].y, vertices[0].z, color.r, color.g, color.b });

    // Top face
    lineData.insert(lineData.end(), { vertices[4].x, vertices[4].y, vertices[4].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[5].x, vertices[5].y, vertices[5].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[5].x, vertices[5].y, vertices[5].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[6].x, vertices[6].y, vertices[6].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[6].x, vertices[6].y, vertices[6].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[7].x, vertices[7].y, vertices[7].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[7].x, vertices[7].y, vertices[7].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[4].x, vertices[4].y, vertices[4].z, color.r, color.g, color.b });

    // Vertical edges
    lineData.insert(lineData.end(), { vertices[0].x, vertices[0].y, vertices[0].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[4].x, vertices[4].y, vertices[4].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[1].x, vertices[1].y, vertices[1].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[5].x, vertices[5].y, vertices[5].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[2].x, vertices[2].y, vertices[2].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[6].x, vertices[6].y, vertices[6].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[3].x, vertices[3].y, vertices[3].z, color.r, color.g, color.b });
    lineData.insert(lineData.end(), { vertices[7].x, vertices[7].y, vertices[7].z, color.r, color.g, color.b });

    // Identity model matrix for AABB (no transformation needed as vertices are world-space)
    glm::mat4 modelMat = glm::mat4(1.0f);
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

    glBindVertexArray(aabbVAO);
    glBindBuffer(GL_ARRAY_BUFFER, aabbVBO);
    glBufferData(GL_ARRAY_BUFFER, lineData.size() * sizeof(GLfloat), lineData.data(), GL_DYNAMIC_DRAW);

    glDisable(GL_CULL_FACE);  // Lines don't need culling
    glDrawArrays(GL_LINES, 0, 24);  // 12 lines * 2 vertices = 24 vertices

    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);  // Re-enable if needed
}

void DrawWalls() {
    std::vector<GLfloat> wallData;

    // Left wall (red) - X = -ROOM_SIZE
    float leftWall[] = {
        -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // Left bottom back
        -ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // Left bottom front
        -ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // Left top front
        -ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // Left top front
        -ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 1.0f, 0.0f, 0.0f,  // Left top back
        -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 1.0f, 0.0f, 0.0f   // Left bottom back
    };

    // Right wall (green) - X = ROOM_SIZE
    float rightWall[] = {
        ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 1.0f, 0.0f,  // Right bottom back
        ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 1.0f, 0.0f,  // Right top back
        ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 0.0f, 1.0f, 0.0f,  // Right top front
        ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 0.0f, 1.0f, 0.0f,  // Right top front
        ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.0f, 1.0f, 0.0f,  // Right bottom front
        ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 1.0f, 0.0f   // Right bottom back
    };

    // Back wall (blue) - Z = -ROOM_SIZE
    float backWall[] = {
        -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // Left bottom back
        ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // Right bottom back
        ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // Right top back
        ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // Right top back
        -ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,  // Left top back
        -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 0.0f, 1.0f   // Left bottom back
    };

    // Front wall (magenta) - Z = ROOM_SIZE, floor Y = frontWallY (slide)
    GLfloat frontWall[36];
    int idx = 0;
    frontWall[idx++] = -ROOM_SIZE; frontWall[idx++] = frontWallY; frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = 1.0f; frontWall[idx++] = 0.0f; frontWall[idx++] = 1.0f;  // Left bottom front
    frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = frontWallY; frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = 1.0f; frontWall[idx++] = 0.0f; frontWall[idx++] = 1.0f;  // Right bottom front
    frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = 1.0f; frontWall[idx++] = 0.0f; frontWall[idx++] = 1.0f;  // Right top front
    frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = 1.0f; frontWall[idx++] = 0.0f; frontWall[idx++] = 1.0f;  // Right top front
    frontWall[idx++] = -ROOM_SIZE; frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = 1.0f; frontWall[idx++] = 0.0f; frontWall[idx++] = 1.0f;  // Left top front
    frontWall[idx++] = -ROOM_SIZE; frontWall[idx++] = frontWallY; frontWall[idx++] = ROOM_SIZE; frontWall[idx++] = 1.0f; frontWall[idx++] = 0.0f; frontWall[idx++] = 1.0f;  // Left bottom front

    // Top wall (yellow) - Y = ROOM_SIZE
    float topWall[] = {
        -ROOM_SIZE, ROOM_SIZE, -ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // Left top back
        -ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // Left top front
        ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // Right top front
        ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // Right top front
        ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 1.0f, 1.0f, 0.0f,  // Right top back
        -ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 1.0f, 1.0f, 0.0f   // Left top back
    };

    // Floor (gray, conditional)
    if (!floorOpen) {
        float bottomWall[] = {
            -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // Left floor back
            -ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // Left floor front
            ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // Right floor front
            ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // Right floor front
            ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.5f, 0.5f, 0.5f,  // Right floor back
            -ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.5f, 0.5f, 0.5f   // Left floor back
        };

        for (int i = 0; i < 36; i++) {
            wallData.push_back(bottomWall[i]);
        }
    }

    // Add all wall data
    for (int i = 0; i < 36; i++) {
        wallData.push_back(leftWall[i]);
    }
    for (int i = 0; i < 36; i++) {
        wallData.push_back(rightWall[i]);
    }
    for (int i = 0; i < 36; i++) {
        wallData.push_back(backWall[i]);
    }
    for (int i = 0; i < 36; i++) {
        wallData.push_back(frontWall[i]);
    }
    for (int i = 0; i < 36; i++) {
        wallData.push_back(topWall[i]);
    }

    glBindVertexArray(wallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, wallData.size() * sizeof(GLfloat), wallData.data(), GL_DYNAMIC_DRAW);

    // Apply rotation to walls
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, 4.0f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(wallRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));  // Z-axis rotation

    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

    // Disable culling to see inside
    glDisable(GL_CULL_FACE);

    int wallCount = floorOpen ? 5 : 6;
    glDrawArrays(GL_TRIANGLES, 0, wallCount * 6);

    glBindVertexArray(0);
}

void DrawObstacles() {
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");

    glm::vec3 obstacleColors[3] = {
        glm::vec3(0.7f, 0.3f, 0.3f),
        glm::vec3(0.3f, 0.7f, 0.3f),
        glm::vec3(0.3f, 0.3f, 0.7f)
    };

    for (int i = 0; i < 3; ++i) {
        UpdateModelBuffers(&Obstacle[i], obstacleVAO, obstacleVBO, obstacleEBO, obstacleColors[i]);

        glm::mat4 obstacleMat = glm::mat4(1.0f);
        obstacleMat = glm::translate(obstacleMat, obstaclePositions[i]);
        obstacleMat = glm::scale(obstacleMat, glm::vec3(1.0f));


        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obstacleMat));
        glBindVertexArray(obstacleVAO);
        glDrawElements(GL_TRIANGLES, Obstacle[i].indexCount, GL_UNSIGNED_INT, 0);

        // New: Draw obstacle AABB after each obstacle
        AABB obsBox = GetObstacleAABB(i);
        DrawAABB(obsBox, glm::vec3(0.0f, 1.0f, 0.0f));  // Green for obstacles
    }

    glBindVertexArray(0);
}

GLvoid DrawScene() {
    // Movement logic (with collision check)
    if (currentMoveDir == UP) {
        modelAngle = 180.0f;
        float prevZ = moveVec.z;  // Previous Z for collision rollback
        moveVec.z -= modelMoveSpeed;
        if (moveVec.z < -ROOM_SIZE + 0.5f) {
            moveVec.z = -ROOM_SIZE + 0.5f;
            currentMoveDir = DOWN;
        }
        else {
            
            if (!isJumping || !CheckJumpOverObstacle()) {  // If not jumping over
                if (HasObstacleCollision() && moveVec.y < 2.0f) {
                    moveVec.z = prevZ;  // Rollback on collision
                }
            }
        }
    }
    else if (currentMoveDir == DOWN) {
        modelAngle = 0.0f;
        float prevZ = moveVec.z;
        moveVec.z += modelMoveSpeed;
        if (moveVec.z > ROOM_SIZE - 0.5f) {
            moveVec.z = ROOM_SIZE - 0.5f;
			currentMoveDir = UP;
        }
        else {
            if (!isJumping || !CheckJumpOverObstacle()) {
                if (HasObstacleCollision() && moveVec.y < 2.0f) {
                    moveVec.z = prevZ;
                }
            }
        }
    }
    else if (currentMoveDir == LEFT) {
        modelAngle = -90.0f;
        float prevX = moveVec.x;
        moveVec.x -= modelMoveSpeed;
        if (moveVec.x < -ROOM_SIZE + 0.5f) {
            moveVec.x = -ROOM_SIZE + 0.5f;
			currentMoveDir = RIGHT;
        }
        else {
            if (!isJumping || !CheckJumpOverObstacle()) {
                if (HasObstacleCollision() && moveVec.y < 2.0f) {
                    moveVec.x = prevX;
                }
            }
        }
    }
    else if (currentMoveDir == RIGHT) {
        modelAngle = 90.0f;
        float prevX = moveVec.x;
        moveVec.x += modelMoveSpeed;
        if (moveVec.x > ROOM_SIZE - 0.5f) {
            moveVec.x = ROOM_SIZE - 0.5f;
			currentMoveDir = LEFT;
        }
        else {
            if (!isJumping || !CheckJumpOverObstacle()) {
                if (HasObstacleCollision() && moveVec.y < 2.0f) {
                    moveVec.x = prevX;
                }
            }
        }
    }

    float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z); // XZ plane distance
    cameraPos.x = r * glm::cos(cameraAngle);
    cameraPos.z = r * glm::sin(cameraAngle);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Dark background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);
    glm::vec3 pivotOffset(0.0f, 0.5f, 0.0f);
    glm::vec3 armPivotOffset(0.0f, 0.4f, 0.0f);
    // Uniform matrices
    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

    // Apply rotation
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 relativeDir = target - cameraPos;
    glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), cameraYaw, glm::vec3(0.0f, 1.0f, 0.0f));
    relativeDir = glm::mat3(yawRotation) * relativeDir;
    target = cameraPos + relativeDir;

    glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    // Draw walls
    DrawWalls();

    // Draw obstacles (includes their AABBs now)
    DrawObstacles();

    // New: Draw character AABB after movement update
    AABB charBox = GetCharacterAABB();
    DrawAABB(charBox, glm::vec3(1.0f, 0.0f, 0.0f));  // Red for character

    // World matrix (translation + rotation)
    glm::mat4 worldMat = glm::translate(glm::mat4(1.0f), moveVec);
    worldMat = worldMat * glm::rotate(glm::mat4(1.0f), glm::radians(modelAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Body local matrix
    glm::mat4 bodyLocalMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 1.2f, 0.4f)); // Body scale
    bodyLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));  // Y center up
    glm::mat4 bodyModelMat = worldMat * bodyLocalMat;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bodyModelMat));
    glBindVertexArray(bodyVAO);
    glDrawElements(GL_TRIANGLES, Body.indexCount, GL_UNSIGNED_INT, 0);

    // Head local matrix
    glm::mat4 headLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.7f, 0.0f));  // Above body
    headLocalMat = headLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.6f)); // Head scale
    glm::mat4 headModelMat = worldMat * headLocalMat;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(headModelMat));
    glBindVertexArray(headVAO);
    glDrawElements(GL_TRIANGLES, Head.indexCount, GL_UNSIGNED_INT, 0);

    // Nose local matrix
    glm::mat4 noseLocalMat = headLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.3f));  // Front of head
    noseLocalMat = noseLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 1.5f)); // Nose scale
    glm::mat4 noseModelMat = worldMat * noseLocalMat;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(noseModelMat));
    glBindVertexArray(noseVAO);
    glDrawElements(GL_TRIANGLES, Nose.indexCount, GL_UNSIGNED_INT, 0);

    // LeftLeg local matrix
    glm::mat4 leftLegLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(-0.3f, -1.0f, 0.0f));  // Below body left

    leftLegLocalMat = leftLegLocalMat * glm::translate(glm::mat4(1.0f), pivotOffset);  // Move pivot to origin
    leftLegLocalMat = leftLegLocalMat * glm::rotate(glm::mat4(1.0f), glm::radians(legSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // X rotation
    leftLegLocalMat = leftLegLocalMat * glm::translate(glm::mat4(1.0f), -pivotOffset);  // Return to original position
    leftLegLocalMat = leftLegLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 1.0f, 0.3f)); // Leg scale

    glm::mat4 leftLegModelMat = worldMat * leftLegLocalMat;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(leftLegModelMat));
    glBindVertexArray(leftLegVAO);
    glDrawElements(GL_TRIANGLES, LeftLeg.indexCount, GL_UNSIGNED_INT, 0);

    // RightLeg local matrix
    glm::mat4 rightLegLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, -1.0f, 0.0f));  // Below body right

    rightLegLocalMat = rightLegLocalMat * glm::translate(glm::mat4(1.0f), pivotOffset);  // Move pivot to origin
    rightLegLocalMat = rightLegLocalMat * glm::rotate(glm::mat4(1.0f), glm::radians(-legSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // X rotation
    rightLegLocalMat = rightLegLocalMat * glm::translate(glm::mat4(1.0f), -pivotOffset);  // Return to original position
    rightLegLocalMat = rightLegLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 1.0f, 0.3f)); // Leg scale

    glm::mat4 rightLegModelMat = worldMat * rightLegLocalMat;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rightLegModelMat));
    glBindVertexArray(rightLegVAO);
    glDrawElements(GL_TRIANGLES, RightLeg.indexCount, GL_UNSIGNED_INT, 0);

    // LeftArm local matrix
    glm::mat4 leftArmLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(-0.6f, 0.0f, 0.0f));  // Left of body
    leftArmLocalMat = leftArmLocalMat * glm::translate(glm::mat4(1.0f), armPivotOffset);  // Move pivot to origin
    leftArmLocalMat = leftArmLocalMat * glm::rotate(glm::mat4(1.0f), glm::radians(-armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // X rotation
    leftArmLocalMat = leftArmLocalMat * glm::translate(glm::mat4(1.0f), -armPivotOffset);  // Return to original position
    leftArmLocalMat = leftArmLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 1.0f, 0.3f)); // Arm scale

    glm::mat4 leftArmModelMat = worldMat * leftArmLocalMat;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(leftArmModelMat));
    glBindVertexArray(leftArmVAO);
    glDrawElements(GL_TRIANGLES, LeftArm.indexCount, GL_UNSIGNED_INT, 0);

    // RightArm local matrix
    glm::mat4 rightArmLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.6f, 0.0f, 0.0f));  // Right of body
    rightArmLocalMat = rightArmLocalMat * glm::translate(glm::mat4(1.0f), armPivotOffset);  // Move pivot to origin
    rightArmLocalMat = rightArmLocalMat * glm::rotate(glm::mat4(1.0f), glm::radians(armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // X rotation
    rightArmLocalMat = rightArmLocalMat * glm::translate(glm::mat4(1.0f), -armPivotOffset);  // Return to original position
    rightArmLocalMat = rightArmLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 1.0f, 0.3f)); // Arm scale

    glm::mat4 rightArmModelMat = worldMat * rightArmLocalMat;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rightArmModelMat));
    glBindVertexArray(rightArmVAO);
    glDrawElements(GL_TRIANGLES, RightArm.indexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    // Draw XYZ axes
    glm::mat4 AxisMat = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(AxisMat));
    std::vector<GLfloat> axesData = {
        // X-axis (red)
        -10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // Start
         10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // End

         // Y-axis (blue)
          0.0f,-10.0f, 0.0f, 0.0f, 0.0f, 1.0f,
          0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 1.0f,

          // Z-axis (green)
           0.0f, 0.0f,-10.0f, 0.0f, 1.0f, 0.0f,
           0.0f, 0.0f, 10.0f, 0.0f, 1.0f, 0.0f
    };

    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, axesData.size() * sizeof(GLfloat), axesData.data(), GL_STATIC_DRAW);

    glDrawArrays(GL_LINES, 0, 6);

    glBindVertexArray(0);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(
        ImVec2((float)width, 0.0f),
        ImGuiCond_FirstUseEver,
        ImVec2(1.0f, 0.0f)
    );
    ImGui::SetNextWindowSize(ImVec2(200, 150), ImGuiCond_FirstUseEver);

    // ImGui UI window
    ImGui::Begin("Human Model Controls");

    //ImGui::SliderFloat("World Y Offset", &moveVec.y, -5.0f, 5.0f);
    ImGui::SliderFloat("Gravity", &gravity, -30.0f, -5.0f);  // Gravity slider

    ImGui::Checkbox("Front Open", &frontOpen);
    ImGui::Text("Is Jumping: %s", isJumping ? "Yes" : "No");  // New: Debug
    ImGui::Text("Y Position: %.2f", moveVec.y);  // New: Debug

    ImGui::End();

    // ImGui rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    ImGui_ImplGLUT_ReshapeFunc(w, h);
}

void Keyboard(unsigned char key, int x, int y) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        glutPostRedisplay();
        return;
    }

    switch (key) {
    case 'a':
        cameraOrbitAnim = !cameraOrbitAnim;
        break;
    case 'r':
        targetCameraAngle += 0.1f;
        break;
    case 'R':
        targetCameraAngle -= 0.1f;
        break;
    case 'y':
        targetCameraYaw += 0.1f;
        break;
    case 'Y':
        targetCameraYaw -= 0.1f;
        break;
    case 'z':
        cameraPos.z += 0.5f;
        break;
    case 'j':  // Jump
        if (!isJumping) {  // Modified: Jump only if not airborne
            isJumping = true;
            jumpVelocity = jumpForce;
        }
        break;
    case 'i':  // Reset
        isJumping = false;
        jumpVelocity = 0.0f;
        moveVec.y = 1.1f;
		moveVec.x = 0.0f;
		moveVec.z = 0.0f;
        currentMoveDir = RIGHT;
        break;
    case '+':
        modelMoveSpeed += 0.01f;
        break;
    case '-':
        modelMoveSpeed -= 0.01f;
        break;
    case 'Z':
        cameraPos.z -= 0.5f;
        break;
    case 'x':
        cameraPos.x += 0.5f;
        break;
    case 'X':
        cameraPos.x -= 0.5f;
        break;
    case 'o':
    case 'O':
        frontOpen = !frontOpen;
        break;
	
    case 'q':
        exit(0);
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

void Timer(int value) {
    float delta = 1.0f / 60.0f;

    cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);

    targetFrontWallY = frontOpen ? ROOM_SIZE : FLOOR_Y;
    frontWallY = glm::mix(frontWallY, targetFrontWallY, interpSpeed * delta);

    // Leg animation
    if (legFlag) {
        if (legSwingAngle >= 30.0f) legFlag = false;
        else if (legSwingAngle <= -30.0f) legFlag = true;
        legSwingAngle += 180.0f * delta * (modelMoveSpeed * 100);
    }
    else {
        if (legSwingAngle >= 30.0f) legFlag = false;
        else if (legSwingAngle <= -30.0f) legFlag = true;
        legSwingAngle -= 180.0f * delta * (modelMoveSpeed * 100);
    }

    // Arm animation
    if (armFlag) {
        if (armSwingAngle >= 30.0f) armFlag = false;
        else if (armSwingAngle <= -30.0f) armFlag = true;
        armSwingAngle += 180.0f * delta * (modelMoveSpeed * 100);
    }
    else {
        if (armSwingAngle >= 30.0f) armFlag = false;
        else if (armSwingAngle <= -30.0f) armFlag = true;
        armSwingAngle -= 180.0f * delta * (modelMoveSpeed * 100);
    }

    cameraYaw = glm::mix(cameraYaw, targetCameraYaw, interpSpeed * delta);

    // Jump & Y-Axis AABB Collision Support logic (중력 후 충돌 체크)
    if (isJumping) {  // 점프 중 중력 적용
        jumpVelocity += gravity * delta;
        moveVec.y += jumpVelocity * delta;

        // 착지: 지면 or 장애물 Y축 충돌 스냅
        if (moveVec.y <= GROUND_Y) {
            moveVec.y = GROUND_Y;
            jumpVelocity = 0.0f;
            isJumping = false;
        }
        else {
            float highestTop = GetHighestObstacleTop();
            if (highestTop > -100.0f) {  // Y축 포함 AABB 충돌 → 장애물 top 스냅
                moveVec.y = highestTop + 1.0f;  // bottom = top, center = +1.0
                jumpVelocity = 0.0f;
                isJumping = false;
            }
        }
    }
    else {
        // 착지 후: 매 프레임 Y축 충돌 체크 (지지 유지 or 떨어짐)
        float highestTop = GetHighestObstacleTop();
        if (highestTop > -100.0f) {
            moveVec.y = highestTop + 1.0f;  // Y 고정 (위에서 걸어다님)
            jumpVelocity = 0.0f;
        }
        else if (moveVec.y > GROUND_Y) {
            // 공중 (지지/지면 없음) → 중력 시작
            isJumping = true;
            jumpVelocity = 0.0f;
        }
        else {
            // 지면 유지
            moveVec.y = GROUND_Y;
            jumpVelocity = 0.0f;
        }
    }

    if (cameraOrbitAnim) {
        targetCameraAngle += 1.0f * delta;
    }

    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}

void Mouse(int button, int state, int x, int y) {
    ImGui_ImplGLUT_MouseFunc(button, state, x, y);

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        // Mouse click logic
    }
}

void Motion(int x, int y) {
    ImGui_ImplGLUT_MotionFunc(x, y);

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        // Mouse motion logic
    }
}

void SpecialKeyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        currentMoveDir = UP;
        break;
    case GLUT_KEY_DOWN:
        currentMoveDir = DOWN;
        break;
    case GLUT_KEY_LEFT:
        currentMoveDir = LEFT;
        break;
    case GLUT_KEY_RIGHT:
        currentMoveDir = RIGHT;
        break;
    default:
        break;
    }

    glutPostRedisplay();
}