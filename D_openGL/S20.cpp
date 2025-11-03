#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" 

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);
void LoadOBJ(const char* filename);
void Mouse(int button, int state, int x, int y);
void SpecialKeyboard(int key, int x, int y);
void Motion(int x, int y);
glm::vec3 CalculateModelCenter(Model* model);


// 초기값 정의 (리셋 기능용)
bool initialCameraOrbitAnim = false;
float initialMidBodyYRotation = 0.0f;
float initialBarrelYRotation = 0.0f;
float initialFlagXRotation = 0.0f;
float initialTopSwapProgress = 0.0f;
glm::vec3 initialMoveVec = glm::vec3(0.0f, 2.0f, 0.0f);
float initialCameraAngle = glm::atan(1.0f);
float initialTargetCameraAngle = glm::atan(1.0f);
float initialCameraYaw = 0.0f;
float initialTargetCameraYaw = 0.0f;
glm::vec3 initialCameraPos = glm::vec3(10.0f, 10.0f, 10.0f);


// 전역 변수
GLint width = 1280, height = 720;
GLuint shaderProgramID, vertexShader, fragmentShader;

// 모델별 VAO/VBO/EBO
GLuint lowVAO, lowVBO, lowEBO;
GLuint midVAO, midVBO, midEBO;
GLuint topVAO[2], topVBO[2], topEBO[2];
GLuint flagVAO[2], flagVBO[2], flagEBO[2];
GLuint barrelVAO[2], barrelVBO[2], barrelEBO[2];

GLuint axisVAO, axisVBO; 


GLuint floorVAO, floorVBO, floorEBO;

bool allAnimationsPaused = false; // 모든 애니메이션 일시정지 플래그

struct ExtendedModel {
	Model baseModel;
	glm::vec3 center;
	GLuint indexCount = 0;  // 인덱스 개수 저장
	std::vector<GLuint> indices;  // 모델별 인덱스 벡터
};

void UpdateModelBuffers(ExtendedModel* model, GLuint vao, GLuint vbo, GLuint ebo, glm::vec3 color);

ExtendedModel lowBody;
ExtendedModel midBody;
ExtendedModel topBody[2];
ExtendedModel flagModel[2];
ExtendedModel barrelModel[2];

float cameraAngle = glm::atan(1.0f); // 초기 각도 (45도 라디안)
float targetCameraAngle = glm::atan(1.0f); // 타겟 카메라 각도

// 카메라 위치 관련
glm::vec3 cameraPos = glm::vec3(10.0f, 10.0f, 10.0f);
float interpSpeed = 10.0f; // 선형 보간 속도
glm::vec3 moveVec(0.0f, 2.0f, 0.0f);
bool cameraOrbitAnim = false;

// 카메라 자전 관련 
float cameraYaw = 0.0f;  
float targetCameraYaw = 0.0f;  

// 탱크 회전 관련
float midBodyYRotation = 0.0f;
float barrelYRotation = 0.0f;
float flagXRotation = 0.0f;
//상부 몸체 이동 
float topSwapProgress = 0.0f; 


// 명령어 
bool tFlag = false;
int lFlag = -1;
bool gFlag = false;
bool pFlag = false;

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("S20_with_ImGui"); // 창 이름 변경

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패" << std::endl;
		return -1;
	}

	// --- ImGui 초기화 (Default_GL.cpp에서 가져옴) ---
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGLUT_Init();
	ImGui_ImplOpenGL3_Init("#version 330");
	// --- ImGui 초기화 끝 ---

	MakeVertexShaders();
	MakeFragmentShaders();
	shaderProgramID = MakeShaderProgram();
	if (shaderProgramID == 0) {
		std::cerr << "셰이더 프로그램 생성 실패" << std::endl;
		return -1;
	}

	// OBJ 파일 로드
	read_obj_file("cube.obj", &lowBody.baseModel);  // lowBody OBJ
	read_obj_file("cube.obj", &midBody.baseModel);  // midBody OBJ
	read_obj_file("cube.obj", &topBody[0].baseModel);  // topBody[0] OBJ
	read_obj_file("cube.obj", &topBody[1].baseModel);  // topBody[1] OBJ

	
	read_obj_file("cube.obj", &flagModel[0].baseModel);  // flagModel[0] OBJ
	read_obj_file("cube.obj", &flagModel[1].baseModel);  // flagModel[1] OBJ
	read_obj_file("cube.obj", &barrelModel[0].baseModel);  // barrelModel[0] OBJ
	read_obj_file("cube.obj", &barrelModel[1].baseModel);  // barrelModel[1] OBJ

	InitBuffers();


	UpdateModelBuffers(&lowBody, lowVAO, lowVBO, lowEBO, glm::vec3(1.0f, 0.0f, 0.0f));
	UpdateModelBuffers(&midBody, midVAO, midVBO, midEBO , glm::vec3(0.0f, 1.0f, 0.0f));
	UpdateModelBuffers(&topBody[0], topVAO[0], topVBO[0], topEBO[0] , glm::vec3(1.0f, 1.0f, 0.0f));
	UpdateModelBuffers(&topBody[1], topVAO[1], topVBO[1], topEBO[1] , glm::vec3(1.0f, 0.0f, 1.0f));

	// Flag와 Barrel 버퍼 업데이트 추가
	UpdateModelBuffers(&flagModel[0], flagVAO[0], flagVBO[0], flagEBO[0], glm::vec3(0.5f, 0.5f, 0.5f));
	UpdateModelBuffers(&flagModel[1], flagVAO[1], flagVBO[1], flagEBO[1], glm::vec3(0.5f, 0.5f, 0.5f));
	UpdateModelBuffers(&barrelModel[0], barrelVAO[0], barrelVBO[0], barrelEBO[0], glm::vec3(1.0f, 7.0f, 7.0f));
	UpdateModelBuffers(&barrelModel[1], barrelVAO[1], barrelVBO[1], barrelEBO[1], glm::vec3(1.0f, 7.0f, 7.0f));

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutTimerFunc(16, Timer, 0); // 타이머 등록 (~60 FPS)

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



	// LowBody 버퍼 초기화
	glGenVertexArrays(1, &lowVAO);
	glBindVertexArray(lowVAO);
	glGenBuffers(1, &lowVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lowVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &lowEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lowEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// MidBody 버퍼 초기화
	glGenVertexArrays(1, &midVAO);
	glBindVertexArray(midVAO);
	glGenBuffers(1, &midVBO);
	glBindBuffer(GL_ARRAY_BUFFER, midVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &midEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, midEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// TopBody[0] 버퍼 초기화
	glGenVertexArrays(1, &topVAO[0]);
	glBindVertexArray(topVAO[0]);
	glGenBuffers(1, &topVBO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, topVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &topEBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topEBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// TopBody[1] 버퍼 초기화
	glGenVertexArrays(1, &topVAO[1]);
	glBindVertexArray(topVAO[1]);
	glGenBuffers(1, &topVBO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, topVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &topEBO[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topEBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// Flag[0] 버퍼 초기화
	glGenVertexArrays(1, &flagVAO[0]);
	glBindVertexArray(flagVAO[0]);
	glGenBuffers(1, &flagVBO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, flagVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &flagEBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flagEBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// Flag[1] 버퍼 초기화
	glGenVertexArrays(1, &flagVAO[1]);
	glBindVertexArray(flagVAO[1]);
	glGenBuffers(1, &flagVBO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, flagVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &flagEBO[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flagEBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// Barrel[0] 버퍼 초기화
	glGenVertexArrays(1, &barrelVAO[0]);
	glBindVertexArray(barrelVAO[0]);

	glGenBuffers(1, &barrelVBO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, barrelVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &barrelEBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, barrelEBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// Barrel[1] 버퍼 초기화
	glGenVertexArrays(1, &barrelVAO[1]);
	glBindVertexArray(barrelVAO[1]);
	glGenBuffers(1, &barrelVBO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, barrelVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &barrelEBO[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, barrelEBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// 축 그리기용 별도 VAO/VBO 초기화
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

	// 바닥 그리기용 별도 VAO/VBO/EBO 초기화 (floorEBO 추가)
	glGenVertexArrays(1, &floorVAO);
	glBindVertexArray(floorVAO);

	glGenBuffers(1, &floorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

	glGenBuffers(1, &floorEBO);  // ← 추가: EBO 생성
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO); 
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);  

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
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

	// 중심 계산 및 저장
	model->center = CalculateModelCenter(&model->baseModel);

	// 버텍스 데이터 생성 (중심 중심으로 이동)
	for (size_t i = 0; i < model->baseModel.vertex_count; ++i) {
		Vertex v = model->baseModel.vertices[i];
		float x = v.x - model->center.x;
		float y = v.y - model->center.y;
		float z = v.z - model->center.z;
		vertexData.push_back(x);
		vertexData.push_back(y);
		vertexData.push_back(z);
		vertexData.push_back(color.r);
		vertexData.push_back(color.g);
		vertexData.push_back(color.b);
	}

	// 인덱스 생성 (모델별 로컬 벡터)
	model->indices.clear();
	for (size_t i = 0; i < model->baseModel.face_count; ++i) {
		model->indices.push_back(model->baseModel.faces[i].v1);
		model->indices.push_back(model->baseModel.faces[i].v2);
		model->indices.push_back(model->baseModel.faces[i].v3);
	}
	model->indexCount = static_cast<GLuint>(model->indices.size());

	// 버퍼 업로드 (매개변수 사용)
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices.size() * sizeof(GLuint), model->indices.data(), GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
}

void DrawScene() {
	float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z); // XZ 평면 거리
	cameraPos.x = r * glm::cos(cameraAngle);
	cameraPos.z = r * glm::sin(cameraAngle);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);


	// Uniform 매트릭스 설정
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

	

	// 자전 적용: 원점 방향을 카메라 중심 Y축으로 회전
	glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);  // 기본 타겟 (원점)
	glm::vec3 relativeDir = target - cameraPos;  // 카메라에서 타겟까지 상대 벡터
	// Y축으로 자전 회전 적용
	glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), cameraYaw, glm::vec3(0.0f, 1.0f, 0.0f));
	relativeDir = glm::mat3(yawRotation) * relativeDir;  // 벡터 회전 
	target = cameraPos + relativeDir; 

	glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// 바닥 그리기 
	std::vector<GLfloat> floorData = {
		// 위치 (x, y, z) + 색상 (gray: 0.5, 0.5, 0.5)  → 4 vertices (코너만)
		-10.0f,  0.0f, -10.0f, 0.2f, 0.2f, 0.2f,  // v0
		 10.0f, 0.0f, -10.0f, 0.2f, 0.2f, 0.2f,  // v1
		 10.0f, 0.0f,  10.0f, 0.2f, 0.2f, 0.2f,  // v2
		-10.0f, 0.0f,  10.0f, 0.2f, 0.2f, 0.2f   // v3
	};

	std::vector<GLuint> floorIndices = {
		0, 3, 2, 0, 2, 1
	};

	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, floorData.size() * sizeof(GLfloat), floorData.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, floorIndices.size() * sizeof(GLuint), floorIndices.data(), GL_STATIC_DRAW);


	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	

	glm::mat4 worldMat = glm::translate(glm::mat4(1.0f), moveVec);

	// ========== LowBody 로컬 매트릭스 계산 ==========
	glm::mat4 lowLocalMat = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f,1.0f,1.0f));
	lowLocalMat = lowLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));  // 아래 위치
	glm::mat4 lowModelMat = worldMat * lowLocalMat;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lowModelMat));
	glBindVertexArray(lowVAO);
	glDrawElements(GL_TRIANGLES, lowBody.indexCount, GL_UNSIGNED_INT, 0);

	// ========== MidBody 로컬 매트릭스 계산 ==========
	glm::mat4 midLocalMat = glm::mat4(1.0f);
	midLocalMat = glm::translate(midLocalMat, glm::vec3(0.0f, 0.0f, 0.0f));  // 중앙 위치
	midLocalMat = glm::rotate(midLocalMat, glm::radians(midBodyYRotation), glm::vec3(0.0f, 1.0f, 0.0f));  // Y축으로 회전

	glm::mat4 midModelMat = worldMat * midLocalMat;
	midModelMat = glm::scale(midModelMat, glm::vec3(1.7f, 0.8f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(midModelMat));
	glBindVertexArray(midVAO);
	glDrawElements(GL_TRIANGLES, midBody.indexCount, GL_UNSIGNED_INT, 0);

	// ========== TopBody[0] 로컬 매트릭스  ==========
	float leftX = glm::mix(-1.0f, 1.0f, topSwapProgress);  // 왼쪽 → 오른쪽으로 이동
	glm::mat4 topLocalMat0 = midLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(leftX, 1.0f, 0.0f));  // X 오프셋 동적
	topLocalMat0 = topLocalMat0 * glm::scale(glm::mat4(1.0f), glm::vec3(0.8f));
	glm::mat4 topModelMat0 = worldMat * topLocalMat0;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(topModelMat0));
	glBindVertexArray(topVAO[0]);
	glDrawElements(GL_TRIANGLES, topBody[0].indexCount, GL_UNSIGNED_INT, 0);

	// ========== TopBody[1] 로컬 매트릭스 계산  ==========
	float rightX = glm::mix(1.0f, -1.0f, topSwapProgress);  // 오른쪽 → 왼쪽으로 이동
	glm::mat4 topLocalMat1 = midLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(rightX, 1.0f, 0.0f));  // X 오프셋 동적
	topLocalMat1 = topLocalMat1 * glm::scale(glm::mat4(1.0f), glm::vec3(0.8f));
	glm::mat4 topModelMat1 = worldMat * topLocalMat1;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(topModelMat1));
	glBindVertexArray(topVAO[1]);
	glDrawElements(GL_TRIANGLES, topBody[1].indexCount, GL_UNSIGNED_INT, 0);

	// ========== Flag[0] 로컬 매트릭스 계산  ==========
	glm::mat4 flagLocalMat0 = topLocalMat0 *
		glm::rotate(glm::mat4(1.0f), glm::radians(flagXRotation), glm::vec3(1.0f, 0.0f, 0.0f)) *  // TopBody 주위 Y축 공전
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.7f, 0.0f)); 
	glm::mat4 flagModelMat0 = worldMat * flagLocalMat0;
	flagModelMat0 = flagModelMat0 * glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 2.0, 0.3));  // Flag 크기 스케일
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(flagModelMat0));
	glBindVertexArray(flagVAO[0]);
	glDrawElements(GL_TRIANGLES, flagModel[0].indexCount, GL_UNSIGNED_INT, 0);

	// ========== Flag[1] 로컬 매트릭스 계산 (TopBody[1] 기준) ==========
	glm::mat4 flagLocalMat1 = topLocalMat1 *
		glm::rotate(glm::mat4(1.0f), glm::radians(-flagXRotation), glm::vec3(1.0f, 0.0f, 0.0f)) *  // TopBody 주위 Y축 공전
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.7f, 0.0f));

	glm::mat4 flagModelMat1 = worldMat * flagLocalMat1;
	flagModelMat1 = flagModelMat1 * glm::scale(glm::mat4(1.0f), glm::vec3(0.3,2.0,0.3));  // Flag 크기 스케일
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(flagModelMat1));
	glBindVertexArray(flagVAO[1]);
	glDrawElements(GL_TRIANGLES, flagModel[1].indexCount, GL_UNSIGNED_INT, 0);

	// ========== Barrel[0] 로컬 매트릭스 계산 (TopBody[0] 기준) ==========
	glm::mat4 barrelLocalMat0 = topLocalMat0 *
	glm::rotate(glm::mat4(1.0f), glm::radians(barrelYRotation), glm::vec3(0.0f, 1.0f, 0.0f)) *  // TopBody 주위 Y축 공전
	glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.7f));  // Top 앞쪽으로 이동

	glm::mat4 barrelModelMat0 = worldMat * barrelLocalMat0;
	barrelModelMat0 = barrelModelMat0 * glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 0.3, 2.0));  // Barrel 크기 스케일
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(barrelModelMat0));
	glBindVertexArray(barrelVAO[0]);
	glDrawElements(GL_TRIANGLES, barrelModel[0].indexCount, GL_UNSIGNED_INT, 0);

	// ========== Barrel[1] 로컬 매트릭스 계산 (TopBody[1] 기준) ==========
	glm::mat4 barrelLocalMat1 = topLocalMat1 *
	glm::rotate(glm::mat4(1.0f), glm::radians(-barrelYRotation), glm::vec3(0.0f, 1.0f, 0.0f)) *  // TopBody 주위 Y축 공전
	glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.7f));  // Top 앞쪽으로 이동
	
	glm::mat4 barrelModelMat1 = worldMat * barrelLocalMat1;
	barrelModelMat1 = barrelModelMat1 * glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 0.3, 2.0));  // Barrel 크기 스케일
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(barrelModelMat1));
	glBindVertexArray(barrelVAO[1]);
	glDrawElements(GL_TRIANGLES, barrelModel[1].indexCount, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	//---------------------------------------------------------------------------
	// XYZ 축 그리기
	glm::mat4 AxisMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(AxisMat));
	std::vector<GLfloat> axesData = {
		// X축 (red)
		-10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // 시작
		 10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // 끝

		 // Y축 (blue)
		  0.0f,-10.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		  0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 1.0f,

		  // Z축 (green)
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
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);

	// ImGui UI 윈도우
	ImGui::Begin("TEST");

	ImGui::SliderFloat("world", &moveVec.y, 0.0f, 5.0f);

	ImGui::End();

	// ImGui 렌더링
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	// --- ImGui 렌더링 끝 ---

	glutSwapBuffers();
}
void Reshape(int w, int h) {
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	ImGui_ImplGLUT_ReshapeFunc(w, h);  // --- ImGui 리셰이프 호출 추가 ---
}

void Keyboard(unsigned char key, int x, int y) {
	// --- ImGui 키보드 입력 전달 --- (기존)
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) {
		glutPostRedisplay();
		return;
	}

	switch (key) {
	case 'a':
		cameraOrbitAnim = !cameraOrbitAnim;  // 카메라 공전 애니메이션 토글
		break;
	case 'r':
		targetCameraAngle += 0.1f;  // 중점 y축 공전 (시계 방향)
		break;
	case 'R':
		targetCameraAngle -= 0.1f;  // 중점 y축 공전 (반시계 방향)
		break;
	case 'y':
		targetCameraYaw += 0.1f;  // 카메라 Y축 자전 (시계 방향)
		break;
	case 'Y':
		targetCameraYaw -= 0.1f;  // 카메라 Y축 자전 (반시계 방향)
		break;
	case 'z':
		cameraPos.z += 0.5f;  // z축 양 방향 이동
		break;
	case 'Z':
		cameraPos.z -= 0.5f;  // z축 음 방향 이동
		break;
	case 'x':
		cameraPos.x += 0.5f;  // x축 양 방향 이동
		break;
	case 'X':
		cameraPos.x -= 0.5f;  // x축 음 방향 이동
		break;
	case 't':
		tFlag = !tFlag;
		break;
	case 'l':
		lFlag++;
		break;
	case 'g':
		gFlag = !gFlag;
		break;
	case 'p':
		pFlag = !pFlag;
		break;
	case 'o': // 일시정지 토글
		allAnimationsPaused = !allAnimationsPaused;
		break;
	case 'c': // 초기 상태로 리셋
		// 애니메이션 플래그들 초기화
		tFlag = false;
		lFlag = -1;
		gFlag = false;
		pFlag = false;
		allAnimationsPaused = false;
		cameraOrbitAnim = initialCameraOrbitAnim;

		// 회전 및 이동 값들 초기화
		midBodyYRotation = initialMidBodyYRotation;
		barrelYRotation = initialBarrelYRotation;
		flagXRotation = initialFlagXRotation;
		topSwapProgress = initialTopSwapProgress;
		moveVec = initialMoveVec;

		// 카메라 관련 값들 초기화
		cameraAngle = initialCameraAngle;
		targetCameraAngle = initialTargetCameraAngle;
		cameraYaw = initialCameraYaw;
		targetCameraYaw = initialTargetCameraYaw;
		cameraPos = initialCameraPos;
		break;

	case 'q': // 종료
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void Timer(int value) {
	float delta = 1.0f / 60.0f;

	// 애니메이션이 정지되어 있지 않을 때만 업데이트
	if (!allAnimationsPaused) {
		cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);
		cameraYaw = glm::mix(cameraYaw, targetCameraYaw, interpSpeed * delta);

		if (tFlag) {
			midBodyYRotation += 90.0f * delta;
		}
		if (gFlag) {
			barrelYRotation += 90.0f * delta;
		}
		if (pFlag) {
			flagXRotation += 90.0f * delta;
		}

		// 카메라 공전 애니메이션
		if (cameraOrbitAnim) {
			targetCameraAngle += 1.0f * delta;  // 초당 30도 공전 속도 (조정 가능)
		}

		// topBody 스왑 애니메이션 (기존)
		if (lFlag % 2 == 0) {
			topSwapProgress += 1.0f * delta;
			if (topSwapProgress >= 1.0f) {
				topSwapProgress = 1.0f;
			}
		}
		else {
			topSwapProgress -= 1.0f * delta;
			if (topSwapProgress <= 0.0f) {
				topSwapProgress = 0.0f;
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}

void Mouse(int button, int state, int x, int y) {
	ImGui_ImplGLUT_MouseFunc(button, state, x, y);


	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		// 마우스 클릭 로직이 있다면 여기에 추가
	}

}

void Motion(int x, int y) {
	ImGui_ImplGLUT_MotionFunc(x, y);

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		
	}

}

void SpecialKeyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		moveVec.x += 0.1f;
		break;
	case GLUT_KEY_DOWN:
		moveVec.x -= 0.1f;
		break;
	case GLUT_KEY_LEFT:
		moveVec.z += 0.1f;
		break;
	case GLUT_KEY_RIGHT:
		moveVec.z -= 0.1f;
		break;
	default:
		break;
	}

	glutPostRedisplay();  // 재렌더링 요청
}