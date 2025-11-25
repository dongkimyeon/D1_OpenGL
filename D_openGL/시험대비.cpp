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
void Mouse(int button, int state, int x, int y);
void SpecialKeyboard(int key, int x, int y);
void Motion(int x, int y);
glm::vec3 CalculateModelCenter(Model* model);
void DrawOrbit(float radius, glm::vec3 center, glm::vec3 color, int segments = 100); // 궤도 그리기 함수

// 초기값 정의 (리셋 기능용)
bool initialCameraOrbitAnim = false;
glm::vec3 initialMoveVec = glm::vec3(0.0f, 0.0f, 0.0f);
float initialCameraAngle = glm::atan(1.0f);
float initialTargetCameraAngle = glm::atan(1.0f);
float initialCameraYaw = 0.0f;
float initialTargetCameraYaw = 0.0f;
glm::vec3 initialCameraPos = glm::vec3(10.0f, 10.0f, 10.0f);

// 전역 변수
GLint width = 1280, height = 720;
GLuint shaderProgramID, vertexShader, fragmentShader;

// 모델별 VAO/VBO/EBO
GLuint cubeVAO, cubeVBO, cubeEBO;         // 중앙 큐브
GLuint planeVAO[4], planeVBO[4], planeEBO[4];  // 좌우상하 면 4개
GLuint sphere1VAO, sphere1VBO, sphere1EBO;     // 큐브 주위 공전하는 공
GLuint sphere2VAO, sphere2VBO, sphere2EBO;     // 공을 공전하는 작은 공
GLuint axisVAO, axisVBO;
GLuint floorVAO, floorVBO, floorEBO;
GLuint orbitVAO, orbitVBO;  // 궤도용 VAO/VBO 추가

bool allAnimationsPaused = false; // 모든 애니메이션 일시정지 플래그
bool showOrbits = true;  // 궤도 표시 여부

struct ExtendedModel {
	Model baseModel;
	glm::vec3 center;
	GLuint indexCount = 0;  // 인덱스 개수 저장
	std::vector<GLuint> indices;  // 모델별 인덱스 벡터
};

void UpdateModelBuffers(ExtendedModel* model, GLuint vao, GLuint vbo, GLuint ebo, glm::vec3 color);

ExtendedModel centerCube;       // 중앙 큐브
ExtendedModel planes[4];         // 좌우상하 면 4개
ExtendedModel sphere1;           // 큐브 주위 공전하는 공
ExtendedModel sphere2;           // 공을 공전하는 작은 공

float cameraAngle = glm::atan(1.0f); // 초기 각도 (45도 라디안)
float targetCameraAngle = glm::atan(1.0f); // 타겟 카메라 각도

// 카메라 위치 관련
glm::vec3 cameraPos = glm::vec3(10.0f, 10.0f, 10.0f);
float interpSpeed = 10.0f; // 선형 보간 속도
glm::vec3 moveVec(0.0f, 0.0f, 0.0f);
bool cameraOrbitAnim = false;

// 카메라 자전 관련 
float cameraYaw = 0.0f;
float targetCameraYaw = 0.0f;

// 공전 각도
float sphere1OrbitAngle = 0.0f;  // 큐브 주위 공전 각도
float sphere2OrbitAngle = 0.0f;  // 공 주위 공전 각도

// 명령어 
bool tFlag = false;

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Cube with Orbiting Spheres");

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
	read_obj_file("cube.obj", &centerCube.baseModel);  // 중앙 큐브
	
	// 4개의 면 (큐브를 사용하여 납작하게 만들 예정)
	for (int i = 0; i < 4; ++i) {
		read_obj_file("cube.obj", &planes[i].baseModel);
	}

	// 공 모델 로드
	read_obj_file("sphere.obj", &sphere1.baseModel);  // 큐브 주위 공전하는 공
	read_obj_file("sphere.obj", &sphere2.baseModel);  // 공을 공전하는 작은 공

	InitBuffers();

	// 버퍼 업데이트
	UpdateModelBuffers(&centerCube, cubeVAO, cubeVBO, cubeEBO, glm::vec3(1.0f, 0.0f, 0.0f));  // 빨간색 큐브
	UpdateModelBuffers(&planes[0], planeVAO[0], planeVBO[0], planeEBO[0], glm::vec3(0.0f, 1.0f, 0.0f));  // 초록색 (위)
	UpdateModelBuffers(&planes[1], planeVAO[1], planeVBO[1], planeEBO[1], glm::vec3(0.0f, 0.0f, 1.0f));  // 파란색 (아래)
	UpdateModelBuffers(&planes[2], planeVAO[2], planeVBO[2], planeEBO[2], glm::vec3(1.0f, 1.0f, 0.0f));  // 노란색 (왼쪽)
	UpdateModelBuffers(&planes[3], planeVAO[3], planeVBO[3], planeEBO[3], glm::vec3(1.0f, 0.0f, 1.0f));  // 마젠타 (오른쪽)
	UpdateModelBuffers(&sphere1, sphere1VAO, sphere1VBO, sphere1EBO, glm::vec3(0.0f, 1.0f, 1.0f));  // 시안색 (큐브 주위 공)
	UpdateModelBuffers(&sphere2, sphere2VAO, sphere2VBO, sphere2EBO, glm::vec3(1.0f, 1.0f, 1.0f));  // 흰색 (작은 공)

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutTimerFunc(16, Timer, 0); // 타이머 등록 (~60 FPS)

	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);

	glutMainLoop();

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

	// 중앙 큐브 버퍼 초기화
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &cubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// 4개의 면 버퍼 초기화
	for (int i = 0; i < 4; ++i) {
		glGenVertexArrays(1, &planeVAO[i]);
		glBindVertexArray(planeVAO[i]);
		glGenBuffers(1, &planeVBO[i]);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO[i]);
		glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
		glGenBuffers(1, &planeEBO[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}

	// 큐브 주위 공전하는 공 버퍼 초기화
	glGenVertexArrays(1, &sphere1VAO);
	glBindVertexArray(sphere1VAO);
	glGenBuffers(1, &sphere1VBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphere1VBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &sphere1EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere1EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// 공을 공전하는 작은 공 버퍼 초기화
	glGenVertexArrays(1, &sphere2VAO);
	glBindVertexArray(sphere2VAO);
	glGenBuffers(1, &sphere2VBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphere2VBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &sphere2EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere2EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// 궤도용 VAO/VBO 초기화
	glGenVertexArrays(1, &orbitVAO);
	glBindVertexArray(orbitVAO);
	glGenBuffers(1, &orbitVBO);
	glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
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

// 궤도 그리기 함수
void DrawOrbit(float radius, glm::vec3 center, glm::vec3 color, int segments) {
	std::vector<GLfloat> orbitData;
	
	// 원형 궤도 생성 (XZ 평면)
	for (int i = 0; i <= segments; ++i) {
		float angle = 2.0f * 3.14159265359f * i / segments;
		float x = center.x + radius * glm::cos(angle);
		float y = center.y;
		float z = center.z + radius * glm::sin(angle);
		
		orbitData.push_back(x);
		orbitData.push_back(y);
		orbitData.push_back(z);
		orbitData.push_back(color.r);
		orbitData.push_back(color.g);
		orbitData.push_back(color.b);
	}

	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	glm::mat4 modelMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	glBindVertexArray(orbitVAO);
	glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
	glBufferData(GL_ARRAY_BUFFER, orbitData.size() * sizeof(GLfloat), orbitData.data(), GL_DYNAMIC_DRAW);
	
	glDrawArrays(GL_LINE_LOOP, 0, segments + 1);
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
		// 위치 (x, y, z) + 색상 (gray: 0.2, 0.2, 0.2)
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

	// ========== 궤도 그리기 (모델보다 먼저 그려서 뒤에 표시) ==========
	if (showOrbits) {
		// sphere1의 궤도 (큐브 중심 기준)
		float sphere1Radius = 3.0f;
		DrawOrbit(sphere1Radius, moveVec, glm::vec3(0.5f, 0.5f, 0.5f), 100);  // 회색 궤도

		// sphere2의 궤도 (sphere1 위치 기준)
		float sphere2Radius = 1.0f;
		glm::vec3 sphere1Pos = moveVec + glm::vec3(
			sphere1Radius * glm::cos(sphere1OrbitAngle),
			0.0f,
			sphere1Radius * glm::sin(sphere1OrbitAngle)
		);
		
		// sphere2 궤도를 그리기 위해 XY 평면에서 원 생성
		std::vector<GLfloat> orbit2Data;
		int segments = 100;
		for (int i = 0; i <= segments; ++i) {
			float angle = 2.0f * 3.14159265359f * i / segments;
			float x = sphere1Pos.x + sphere2Radius * glm::cos(angle);
			float y = sphere1Pos.y + sphere2Radius * glm::sin(angle);
			float z = sphere1Pos.z;
			
			orbit2Data.push_back(x);
			orbit2Data.push_back(y);
			orbit2Data.push_back(z);
			orbit2Data.push_back(0.7f);  // 밝은 회색
			orbit2Data.push_back(0.7f);
			orbit2Data.push_back(0.7f);
		}

		glm::mat4 orbitMat = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(orbitMat));
		glBindVertexArray(orbitVAO);
		glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
		glBufferData(GL_ARRAY_BUFFER, orbit2Data.size() * sizeof(GLfloat), orbit2Data.data(), GL_DYNAMIC_DRAW);
		glDrawArrays(GL_LINE_LOOP, 0, segments + 1);
		glBindVertexArray(0);
	}

	// ========== 중앙 큐브 그리기 ==========
	glm::mat4 cubeModelMat = worldMat * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModelMat));
	glBindVertexArray(cubeVAO);
	glDrawElements(GL_TRIANGLES, centerCube.indexCount, GL_UNSIGNED_INT, 0);

	// ========== 좌우상하 면 4개 그리기 ==========
	// 위쪽 면 (Y축 위)
	glm::mat4 topPlane = worldMat * 
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f)) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 0.1f, 2.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(topPlane));
	glBindVertexArray(planeVAO[0]);
	glDrawElements(GL_TRIANGLES, planes[0].indexCount, GL_UNSIGNED_INT, 0);

	// 아래쪽 면 (Y축 아래)
	glm::mat4 bottomPlane = worldMat * 
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f)) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 0.1f, 2.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bottomPlane));
	glBindVertexArray(planeVAO[1]);
	glDrawElements(GL_TRIANGLES, planes[1].indexCount, GL_UNSIGNED_INT, 0);

	// 왼쪽 면 (X축 음)
	glm::mat4 leftPlane = worldMat * 
		glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 2.0f, 2.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(leftPlane));
	glBindVertexArray(planeVAO[2]);
	glDrawElements(GL_TRIANGLES, planes[2].indexCount, GL_UNSIGNED_INT, 0);

	// 오른쪽 면 (X축 양)
	glm::mat4 rightPlane = worldMat * 
		glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f)) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 2.0f, 2.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rightPlane));
	glBindVertexArray(planeVAO[3]);
	glDrawElements(GL_TRIANGLES, planes[3].indexCount, GL_UNSIGNED_INT, 0);

	// ========== 큐브 주위를 공전하는 공 그리기 ==========
	float sphere1Radius = 3.0f;  // 공전 반지름
	glm::vec3 sphere1Pos = glm::vec3(
		sphere1Radius * glm::cos(sphere1OrbitAngle),
		0.0f,
		sphere1Radius * glm::sin(sphere1OrbitAngle)
	);
	glm::mat4 sphere1Mat = worldMat * 
		glm::translate(glm::mat4(1.0f), sphere1Pos) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sphere1Mat));
	glBindVertexArray(sphere1VAO);
	glDrawElements(GL_TRIANGLES, sphere1.indexCount, GL_UNSIGNED_INT, 0);

	// ========== 공을 공전하는 작은 공 그리기 ==========
	float sphere2Radius = 1.0f;  // 공전 반지름 (sphere1 기준)
	glm::vec3 sphere2Pos = sphere1Pos + glm::vec3(
		sphere2Radius * glm::cos(sphere2OrbitAngle),
		sphere2Radius * glm::sin(sphere2OrbitAngle),
		0.0f
	);
	glm::mat4 sphere2Mat = worldMat * 
		glm::translate(glm::mat4(1.0f), sphere2Pos) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sphere2Mat));
	glBindVertexArray(sphere2VAO);
	glDrawElements(GL_TRIANGLES, sphere2.indexCount, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	//---------------------------------------------------------------------------
	// XYZ 축 그리기
	glm::mat4 AxisMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(AxisMat));
	std::vector<GLfloat> axesData = {
		// X축 (red)
		-10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

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

	glutSwapBuffers();
}

void Reshape(int w, int h) {
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

void Keyboard(unsigned char key, int x, int y) {
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
	case 'm':  // 궤도 표시 토글
		showOrbits = !showOrbits;
		break;
	case 'o': // 일시정지 토글
		allAnimationsPaused = !allAnimationsPaused;
		break;
	case 'c': // 초기 상태로 리셋
		allAnimationsPaused = false;
		cameraOrbitAnim = initialCameraOrbitAnim;
		moveVec = initialMoveVec;
		cameraAngle = initialCameraAngle;
		targetCameraAngle = initialTargetCameraAngle;
		cameraYaw = initialCameraYaw;
		targetCameraYaw = initialTargetCameraYaw;
		cameraPos = initialCameraPos;
		sphere1OrbitAngle = 0.0f;
		sphere2OrbitAngle = 0.0f;
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

		// 카메라 공전 애니메이션
		if (cameraOrbitAnim) {
			targetCameraAngle += 1.0f * delta;
		}

		// 공 공전 애니메이션
		sphere1OrbitAngle += 1.0f * delta;  // 큐브 주위 공전 (초당 약 57도)
		sphere2OrbitAngle += 2.0f * delta;  // 공 주위 공전 (더 빠르게, 초당 약 114도)
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}

void Mouse(int button, int state, int x, int y) {
	// 마우스 클릭 로직이 있다면 여기에 추가
}

void Motion(int x, int y) {
	// 마우스 이동 로직이 있다면 여기에 추가
}

void SpecialKeyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		moveVec.y += 0.1f;
		break;
	case GLUT_KEY_DOWN:
		moveVec.y -= 0.1f;
		break;
	case GLUT_KEY_LEFT:
		moveVec.x -= 0.1f;
		break;
	case GLUT_KEY_RIGHT:
		moveVec.x += 0.1f;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}