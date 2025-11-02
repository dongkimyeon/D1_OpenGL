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

// 전역 변수
GLint width = 1280, height = 720;
GLuint shaderProgramID, vertexShader, fragmentShader;

// 사람 부위별 VAO/VBO/EBO
GLuint bodyVAO, bodyVBO, bodyEBO;
GLuint headVAO, headVBO, headEBO;
GLuint leftLegVAO, leftLegVBO, leftLegEBO;
GLuint rightLegVAO, rightLegVBO, rightLegEBO;
GLuint leftArmVAO, leftArmVBO, leftArmEBO;
GLuint rightArmVAO, rightArmVBO, rightArmEBO;

GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO

// 바닥 그리기
GLuint floorVAO, floorVBO, floorEBO;

struct ExtendedModel {
	Model baseModel;
	glm::vec3 center;
	GLuint indexCount = 0;  // 인덱스 개수 저장
	std::vector<GLuint> indices;  // 모델별 인덱스 벡터
};

void UpdateModelBuffers(ExtendedModel* model, GLuint vao, GLuint vbo, GLuint ebo, glm::vec3 color);

ExtendedModel Body;
ExtendedModel Head;
ExtendedModel LeftLeg;
ExtendedModel RightLeg;
ExtendedModel LeftArm;
ExtendedModel RightArm;

float cameraAngle = glm::atan(1.0f); // 초기 각도 (45도 라디안)
float targetCameraAngle = glm::atan(1.0f); // 타겟 카메라 각도

// 카메라 위치 관련
glm::vec3 cameraPos = glm::vec3(10.0f, 10.0f, 10.0f);
float interpSpeed = 10.0f; // 선형 보간 속도
glm::vec3 moveVec(0.0f, 0.0f, 0.0f); // 사람 전체 이동 벡터 (Y=0으로 초기화)
bool cameraOrbitAnim = false;

// 카메라 자전 관련 (Y축 yaw 회전)
float cameraYaw = 0.0f;  // 초기 자전 각도 (라디안)
float targetCameraYaw = 0.0f;  // 타겟 자전 각도 (부드러운 보간용, 옵션)

// 사람 애니메이션 관련
float bodyYRotation = 0.0f; // 몸통 Y축 회전
float armSwingAngle = 0.0f; // 팔 흔들기 각도
float legSwingAngle = 0.0f; // 다리 흔들기 각도

// 명령어 플래그
bool rotateFlag = false; // 몸통 회전 토글
bool armFlag = false; // 팔 흔들기 토글
bool legFlag = false; // 다리 흔들기 토글

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Human Model with ImGui"); // 창 이름 변경

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

	// OBJ 파일 로드 (모든 부위에 cube.obj 사용)
	read_obj_file("cube.obj", &Body.baseModel);
	read_obj_file("cube.obj", &Head.baseModel);
	read_obj_file("cube.obj", &LeftLeg.baseModel);
	read_obj_file("cube.obj", &RightLeg.baseModel);
	read_obj_file("cube.obj", &LeftArm.baseModel);
	read_obj_file("cube.obj", &RightArm.baseModel);

	InitBuffers();

	// 버퍼 업데이트 (각 부위별 색상 지정)
	UpdateModelBuffers(&Body, bodyVAO, bodyVBO, bodyEBO, glm::vec3(0.8f, 0.5f, 0.3f)); // 갈색 몸통
	UpdateModelBuffers(&Head, headVAO, headVBO, headEBO, glm::vec3(1.0f, 0.8f, 0.6f)); // 베이지 머리
	UpdateModelBuffers(&LeftLeg, leftLegVAO, leftLegVBO, leftLegEBO, glm::vec3(0.2f, 0.2f, 0.8f)); // 파랑 왼다리
	UpdateModelBuffers(&RightLeg, rightLegVAO, rightLegVBO, rightLegEBO, glm::vec3(0.2f, 0.2f, 0.8f)); // 파랑 오른다리
	UpdateModelBuffers(&LeftArm, leftArmVAO, leftArmVBO, leftArmEBO, glm::vec3(0.6f, 0.4f, 0.2f)); // 갈색 왼팔
	UpdateModelBuffers(&RightArm, rightArmVAO, rightArmVBO, rightArmEBO, glm::vec3(0.6f, 0.4f, 0.2f)); // 갈색 오른팔

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutTimerFunc(16, Timer, 0); // 타이머 등록 (~60 FPS)

	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);

	std::cout << "r: 몸통 회전 토글, a: 팔 흔들기 토글, l: 다리 흔들기 토글" << std::endl;
	std::cout << "방향키: 전체 모델 이동, q: 종료" << std::endl;
	std::cout << "카메라: a(공전 토글), r/R(공전 각도), y/Y(자전 각도)" << std::endl;

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

	// Body 버퍼 초기화
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

	// Head 버퍼 초기화
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

	// LeftLeg 버퍼 초기화
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

	// RightLeg 버퍼 초기화
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

	// LeftArm 버퍼 초기화
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

	// RightArm 버퍼 초기화
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

	// 바닥 그리기용 별도 VAO/VBO/EBO 초기화
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
		// 색상 (i % 4로 사이클링)
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

	// 전체 월드 매트릭스 (이동 + 회전)
	glm::mat4 worldMat = glm::translate(glm::mat4(1.0f), moveVec);
	worldMat = worldMat * glm::rotate(glm::mat4(1.0f), glm::radians(bodyYRotation), glm::vec3(0.0f, 1.0f, 0.0f));

	// ========== Body 로컬 매트릭스 계산 ==========
	glm::mat4 bodyLocalMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 1.2f, 0.4f)); // 몸통 스케일 (높이 길게)
	bodyLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));  // Y축 중앙 위로
	glm::mat4 bodyModelMat = worldMat * bodyLocalMat;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bodyModelMat));
	glBindVertexArray(bodyVAO);
	glDrawElements(GL_TRIANGLES, Body.indexCount, GL_UNSIGNED_INT, 0);

	// ========== Head 로컬 매트릭스 계산 (Body 위) ==========
	glm::mat4 headLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0f));  // Body 위로
	headLocalMat = headLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.6f)); // 머리 스케일 (작게)
	glm::mat4 headModelMat = worldMat * headLocalMat;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(headModelMat));
	glBindVertexArray(headVAO);
	glDrawElements(GL_TRIANGLES, Head.indexCount, GL_UNSIGNED_INT, 0);

	// ========== LeftLeg 로컬 매트릭스 계산 (Body 아래 왼쪽, 애니메이션 적용) ==========
	float leftLegSwing = legFlag ? glm::sin(armSwingAngle) * 0.5f : 0.0f; // 다리 흔들기
	glm::mat4 leftLegLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(-0.3f, -1.2f, 0.0f));  // Body 아래 왼쪽
	leftLegLocalMat = leftLegLocalMat * glm::rotate(glm::mat4(1.0f), glm::radians(leftLegSwing), glm::vec3(1.0f, 0.0f, 0.0f)); // X축 회전
	leftLegLocalMat = leftLegLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 1.0f, 0.3f)); // 다리 스케일
	glm::mat4 leftLegModelMat = worldMat * leftLegLocalMat;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(leftLegModelMat));
	glBindVertexArray(leftLegVAO);
	glDrawElements(GL_TRIANGLES, LeftLeg.indexCount, GL_UNSIGNED_INT, 0);

	// ========== RightLeg 로컬 매트릭스 계산 (Body 아래 오른쪽, 애니메이션 적용) ==========
	float rightLegSwing = legFlag ? -glm::sin(armSwingAngle) * 0.5f : 0.0f; // 반대 방향
	glm::mat4 rightLegLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, -1.2f, 0.0f));  // Body 아래 오른쪽
	rightLegLocalMat = rightLegLocalMat * glm::rotate(glm::mat4(1.0f), glm::radians(rightLegSwing), glm::vec3(1.0f, 0.0f, 0.0f)); // X축 회전
	rightLegLocalMat = rightLegLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 1.0f, 0.3f)); // 다리 스케일
	glm::mat4 rightLegModelMat = worldMat * rightLegLocalMat;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rightLegModelMat));
	glBindVertexArray(rightLegVAO);
	glDrawElements(GL_TRIANGLES, RightLeg.indexCount, GL_UNSIGNED_INT, 0);

	// ========== LeftArm 로컬 매트릭스 계산 (Body 옆 왼쪽, 애니메이션 적용) ==========
	float leftArmSwing = armFlag ? glm::sin(armSwingAngle) * 0.8f : 0.0f; // 팔 흔들기
	glm::mat4 leftArmLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.5f, 0.0f));  // Body 왼쪽
	leftArmLocalMat = leftArmLocalMat * glm::rotate(glm::mat4(1.0f), glm::radians(leftArmSwing), glm::vec3(0.0f, 0.0f, 1.0f)); // Z축 회전 (앞뒤)
	leftArmLocalMat = leftArmLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.8f, 0.3f)); // 팔 스케일
	glm::mat4 leftArmModelMat = worldMat * leftArmLocalMat;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(leftArmModelMat));
	glBindVertexArray(leftArmVAO);
	glDrawElements(GL_TRIANGLES, LeftArm.indexCount, GL_UNSIGNED_INT, 0);

	// ========== RightArm 로컬 매트릭스 계산 (Body 옆 오른쪽, 애니메이션 적용) ==========
	float rightArmSwing = armFlag ? -glm::sin(armSwingAngle) * 0.8f : 0.0f; // 반대 방향
	glm::mat4 rightArmLocalMat = bodyLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.5f, 0.0f));  // Body 오른쪽
	rightArmLocalMat = rightArmLocalMat * glm::rotate(glm::mat4(1.0f), glm::radians(rightArmSwing), glm::vec3(0.0f, 0.0f, 1.0f)); // Z축 회전
	rightArmLocalMat = rightArmLocalMat * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.8f, 0.3f)); // 팔 스케일
	glm::mat4 rightArmModelMat = worldMat * rightArmLocalMat;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rightArmModelMat));
	glBindVertexArray(rightArmVAO);
	glDrawElements(GL_TRIANGLES, RightArm.indexCount, GL_UNSIGNED_INT, 0);

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
	ImGui::Begin("Human Model Controls");

	ImGui::SliderFloat("World Y Offset", &moveVec.y, -5.0f, 5.0f);

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

	cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);

	cameraYaw = glm::mix(cameraYaw, targetCameraYaw, interpSpeed * delta);

	if (rotateFlag) {
		bodyYRotation += 90.0f * delta; // 몸통 회전
	}

	// 팔/다리 흔들기 애니메이션 (공통 각도 업데이트)
	armSwingAngle += 2.0f * delta; // 속도 조정 가능
	legSwingAngle = armSwingAngle; // 팔과 다리 동기화 (간단히)

	// 카메라 공전 애니메이션
	if (cameraOrbitAnim) {
		targetCameraAngle += 1.0f * delta;  // 초당 30도 공전 속도 (조정 가능)
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}

void Mouse(int button, int state, int x, int y) {
	ImGui_ImplGLUT_MouseFunc(button, state, x, y);

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse) {
		// 마우스 클릭 로직이 있다면 여기에 추가
	}
}

void Motion(int x, int y) {
	ImGui_ImplGLUT_MotionFunc(x, y);

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse) {
		// 마우스 모션 로직이 있다면 여기에 추가
	}
}

void SpecialKeyboard(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		moveVec.z -= 0.1f; // 앞으로 이동 (Z 음의 방향)
		break;
	case GLUT_KEY_DOWN:
		moveVec.z += 0.1f; // 뒤로 이동
		break;
	case GLUT_KEY_LEFT:
		moveVec.x -= 0.1f; // 왼쪽 이동
		break;
	case GLUT_KEY_RIGHT:
		moveVec.x += 0.1f; // 오른쪽 이동
		break;
	default:
		break;
	}

	glutPostRedisplay();  // 재렌더링 요청
}