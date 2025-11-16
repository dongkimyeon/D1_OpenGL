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
GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
float interpSpeed = 10.0f;

GLuint lowVAO, lowVBO, lowEBO;
GLuint midVAO, midVBO, midEBO;
GLuint topVAO[2], topVBO[2], topEBO[2];
GLuint flagVAO[2], flagVBO[2], flagEBO[2];
GLuint barrelVAO[2], barrelVBO[2], barrelEBO[2];
GLuint axisVAO, axisVBO; 
GLuint floorVAO, floorVBO, floorEBO;
GLuint lightVAO, lightVBO, lightEBO;

struct ExtendedModel {
	Model baseModel;
	glm::vec3 center;
	GLuint indexCount = 0;
	std::vector<GLuint> indices;
};

void UpdateModelBuffers(ExtendedModel* model, GLuint vao, GLuint vbo, GLuint ebo, glm::vec3 color);

ExtendedModel lowBody;
ExtendedModel midBody;
ExtendedModel topBody[2];
ExtendedModel flagModel[2];
ExtendedModel barrelModel[2];
ExtendedModel lightModel;

float cameraAngle = glm::atan(1.0f);
float targetCameraAngle = glm::atan(1.0f);

glm::vec3 cameraPos = glm::vec3(10.0f, 10.0f, 10.0f);
glm::vec3 moveVec(0.0f, 2.0f, 0.0f);
bool cameraOrbitAnim = false;

float cameraYaw = 0.0f;
float targetCameraYaw = 0.0f;

float midBodyYRotation = 0.0f;
float barrelYRotation = 0.0f;
float flagXRotation = 0.0f;
float topSwapProgress = 0.0f;

bool tFlag = false;
int lFlag = -1;
bool gFlag = false;
bool pFlag = false;


glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);
glm::vec3 lightColors[3] = {
	glm::vec3(1.0f, 1.0f, 1.0f),  // 흰색
	glm::vec3(1.0f, 0.0f, 0.0f),  // 빨강
	glm::vec3(0.0f, 0.0f, 1.0f)   // 파랑
};
int currentLightColor = 0;
bool ambientEnabled = true;
bool lightRotating = false;
float lightAngle = 0.0f;

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	width = 1280;
	height = 720;
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("S27_Lighting");

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
	read_obj_file("cube.obj", &lowBody.baseModel);
	read_obj_file("cube.obj", &midBody.baseModel);
	read_obj_file("cube.obj", &topBody[0].baseModel);
	read_obj_file("cube.obj", &topBody[1].baseModel);
	read_obj_file("cube.obj", &flagModel[0].baseModel);
	read_obj_file("cube.obj", &flagModel[1].baseModel);
	read_obj_file("cube.obj", &barrelModel[0].baseModel);
	read_obj_file("cube.obj", &barrelModel[1].baseModel);
	read_obj_file("cube.obj", &lightModel.baseModel);

	InitBuffers();

	UpdateModelBuffers(&lowBody, lowVAO, lowVBO, lowEBO, glm::vec3(1.0f, 0.0f, 0.0f));
	UpdateModelBuffers(&midBody, midVAO, midVBO, midEBO, glm::vec3(0.0f, 1.0f, 0.0f));
	UpdateModelBuffers(&topBody[0], topVAO[0], topVBO[0], topEBO[0], glm::vec3(1.0f, 1.0f, 0.0f));
	UpdateModelBuffers(&topBody[1], topVAO[1], topVBO[1], topEBO[1], glm::vec3(1.0f, 0.0f, 1.0f));
	UpdateModelBuffers(&flagModel[0], flagVAO[0], flagVBO[0], flagEBO[0], glm::vec3(0.5f, 0.5f, 0.5f));
	UpdateModelBuffers(&flagModel[1], flagVAO[1], flagVBO[1], flagEBO[1], glm::vec3(0.5f, 0.5f, 0.5f));
	UpdateModelBuffers(&barrelModel[0], barrelVAO[0], barrelVBO[0], barrelEBO[0], glm::vec3(1.0f, 0.7f, 0.7f));
	UpdateModelBuffers(&barrelModel[1], barrelVAO[1], barrelVBO[1], barrelEBO[1], glm::vec3(1.0f, 0.7f, 0.7f));
	UpdateModelBuffers(&lightModel, lightVAO, lightVBO, lightEBO, glm::vec3(1.0f, 1.0f, 1.0f));

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutTimerFunc(16, Timer, 0);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);

	glutMainLoop();

	return 0;
}

void MakeVertexShaders() {
	GLchar* vertexSource = filetobuf("lightVertex.glsl");
	if (!vertexSource) {
		std::cerr << "ERROR: lightVertex.glsl 파일을 읽지 못했습니다." << std::endl;
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
	GLchar* fragmentSource = filetobuf("lightFragment.glsl");
	if (!fragmentSource) {
		std::cerr << "ERROR: lightFragment.glsl 파일을 읽지 못했습니다." << std::endl;
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
	const GLsizei stride = 6 * sizeof(GLfloat);

	// LowBody 버퍼
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

	// MidBody 버퍼
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

	// TopBody[0] 버퍼
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

	// TopBody[1] 버퍼
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

	// Flag[0] 버퍼
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

	// Flag[1] 버퍼
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

	// Barrel[0] 버퍼
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

	// Barrel[1] 버퍼
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

	// Light 버퍼
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &lightEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// 축 버퍼
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

	// 바닥 버퍼
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

	model->center = CalculateModelCenter(&model->baseModel);

	// 노멀 벡터 계산을 위한 데이터 (간단하게 face normal 사용)
	for (size_t i = 0; i < model->baseModel.vertex_count; ++i) {
		Vertex v = model->baseModel.vertices[i];
		float x = v.x - model->center.x;
		float y = v.y - model->center.y;
		float z = v.z - model->center.z;
		
		// 위치
		vertexData.push_back(x);
		vertexData.push_back(y);
		vertexData.push_back(z);
		
		// 노멀 (간단하게 정규화된 위치 벡터 사용 - 구체에 가까운 모양에서 작동)
		glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
		vertexData.push_back(normal.x);
		vertexData.push_back(normal.y);
		vertexData.push_back(normal.z);
	}

	model->indices.clear();
	for (size_t i = 0; i < model->baseModel.face_count; ++i) {
		model->indices.push_back(model->baseModel.faces[i].v1);
		model->indices.push_back(model->baseModel.faces[i].v2);
		model->indices.push_back(model->baseModel.faces[i].v3);
	}
	model->indexCount = static_cast<GLuint>(model->indices.size());

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices.size() * sizeof(GLuint), model->indices.data(), GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
}

void DrawScene() {
	float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z);
	cameraPos.x = r * glm::cos(cameraAngle);
	cameraPos.z = r * glm::sin(cameraAngle);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	// Uniform 위치
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
	GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
	GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint objectColorLoc = glGetUniformLocation(shaderProgramID, "objectColor");
	GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");

	// 조명 회전
	if (lightRotating) {
		float radius = glm::length(glm::vec2(lightPos.x, lightPos.z));
		lightPos.x = radius * glm::cos(lightAngle);
		lightPos.z = radius * glm::sin(lightAngle);
	}

	// 뷰 및 프로젝션 매트릭스
	glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 relativeDir = target - cameraPos;
	glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), cameraYaw, glm::vec3(0.0f, 1.0f, 0.0f));
	relativeDir = glm::mat3(yawRotation) * relativeDir;
	target = cameraPos + relativeDir;

	glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// 조명 설정
	glm::vec3 finalLightColor = ambientEnabled ? lightColors[currentLightColor] : glm::vec3(0.0f);
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(finalLightColor));
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

	// 바닥 그리기
	std::vector<GLfloat> floorData = {
		-10.0f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f,
		 10.0f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f,
		 10.0f, 0.0f,  10.0f, 0.0f, 1.0f, 0.0f,
		-10.0f, 0.0f,  10.0f, 0.0f, 1.0f, 0.0f
	};

	std::vector<GLuint> floorIndices = {
		0, 3, 2, 0, 2, 1
	};

	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, floorData.size() * sizeof(GLfloat), floorData.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, floorIndices.size() * sizeof(GLuint), floorIndices.data(), GL_STATIC_DRAW);

	glm::mat4 floorMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(floorMat));
	glUniform3f(objectColorLoc, 0.2f, 0.2f, 0.2f);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	glm::mat4 worldMat = glm::translate(glm::mat4(1.0f), moveVec);

	// LowBody
	glm::mat4 lowLocalMat = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 1.0f, 1.0f));
	lowLocalMat = lowLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glm::mat4 lowModelMat = worldMat * lowLocalMat;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lowModelMat));
	glUniform3f(objectColorLoc, 1.0f, 0.0f, 0.0f);
	glBindVertexArray(lowVAO);
	glDrawElements(GL_TRIANGLES, lowBody.indexCount, GL_UNSIGNED_INT, 0);

	// MidBody
	glm::mat4 midLocalMat = glm::mat4(1.0f);
	midLocalMat = glm::translate(midLocalMat, glm::vec3(0.0f, 0.0f, 0.0f));
	midLocalMat = glm::rotate(midLocalMat, glm::radians(midBodyYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 midModelMat = worldMat * midLocalMat;
	midModelMat = glm::scale(midModelMat, glm::vec3(1.7f, 0.8f, 1.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(midModelMat));
	glUniform3f(objectColorLoc, 0.0f, 1.0f, 0.0f);
	glBindVertexArray(midVAO);
	glDrawElements(GL_TRIANGLES, midBody.indexCount, GL_UNSIGNED_INT, 0);

	// TopBody[0]
	float leftX = glm::mix(-1.0f, 1.0f, topSwapProgress);
	glm::mat4 topLocalMat0 = midLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(leftX, 1.0f, 0.0f));
	topLocalMat0 = topLocalMat0 * glm::scale(glm::mat4(1.0f), glm::vec3(0.8f));
	glm::mat4 topModelMat0 = worldMat * topLocalMat0;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(topModelMat0));
	glUniform3f(objectColorLoc, 1.0f, 1.0f, 0.0f);
	glBindVertexArray(topVAO[0]);
	glDrawElements(GL_TRIANGLES, topBody[0].indexCount, GL_UNSIGNED_INT, 0);

	// TopBody[1]
	float rightX = glm::mix(1.0f, -1.0f, topSwapProgress);
	glm::mat4 topLocalMat1 = midLocalMat * glm::translate(glm::mat4(1.0f), glm::vec3(rightX, 1.0f, 0.0f));
	topLocalMat1 = topLocalMat1 * glm::scale(glm::mat4(1.0f), glm::vec3(0.8f));
	glm::mat4 topModelMat1 = worldMat * topLocalMat1;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(topModelMat1));
	glUniform3f(objectColorLoc, 1.0f, 0.0f, 1.0f);
	glBindVertexArray(topVAO[1]);
	glDrawElements(GL_TRIANGLES, topBody[1].indexCount, GL_UNSIGNED_INT, 0);

	// Flag[0]
	glm::mat4 flagLocalMat0 = topLocalMat0 *
		glm::rotate(glm::mat4(1.0f), glm::radians(flagXRotation), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.7f, 0.0f));
	glm::mat4 flagModelMat0 = worldMat * flagLocalMat0;
	flagModelMat0 = flagModelMat0 * glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 2.0, 0.3));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(flagModelMat0));
	glUniform3f(objectColorLoc, 0.5f, 0.5f, 0.5f);
	glBindVertexArray(flagVAO[0]);
	glDrawElements(GL_TRIANGLES, flagModel[0].indexCount, GL_UNSIGNED_INT, 0);

	// Flag[1]
	glm::mat4 flagLocalMat1 = topLocalMat1 *
		glm::rotate(glm::mat4(1.0f), glm::radians(-flagXRotation), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.7f, 0.0f));
	glm::mat4 flagModelMat1 = worldMat * flagLocalMat1;
	flagModelMat1 = flagModelMat1 * glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 2.0, 0.3));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(flagModelMat1));
	glUniform3f(objectColorLoc, 0.5f, 0.5f, 0.5f);
	glBindVertexArray(flagVAO[1]);
	glDrawElements(GL_TRIANGLES, flagModel[1].indexCount, GL_UNSIGNED_INT, 0);

	// Barrel[0]
	glm::mat4 barrelLocalMat0 = topLocalMat0 *
		glm::rotate(glm::mat4(1.0f), glm::radians(barrelYRotation), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.7f));
	glm::mat4 barrelModelMat0 = worldMat * barrelLocalMat0;
	barrelModelMat0 = barrelModelMat0 * glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 0.3, 2.0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(barrelModelMat0));
	glUniform3f(objectColorLoc, 1.0f, 0.7f, 0.7f);
	glBindVertexArray(barrelVAO[0]);
	glDrawElements(GL_TRIANGLES, barrelModel[0].indexCount, GL_UNSIGNED_INT, 0);

	// Barrel[1]
	glm::mat4 barrelLocalMat1 = topLocalMat1 *
		glm::rotate(glm::mat4(1.0f), glm::radians(-barrelYRotation), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.7f));
	glm::mat4 barrelModelMat1 = worldMat * barrelLocalMat1;
	barrelModelMat1 = barrelModelMat1 * glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 0.3, 2.0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(barrelModelMat1));
	glUniform3f(objectColorLoc, 1.0f, 0.7f, 0.7f);
	glBindVertexArray(barrelVAO[1]);
	glDrawElements(GL_TRIANGLES, barrelModel[1].indexCount, GL_UNSIGNED_INT, 0);

	// 조명 객체 그리기 (작은 큐브)
	glm::mat4 lightModelMat = glm::translate(glm::mat4(1.0f), lightPos);
	lightModelMat = glm::scale(lightModelMat, glm::vec3(0.3f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lightModelMat));
	glUniform3fv(objectColorLoc, 1, glm::value_ptr(lightColors[currentLightColor]));
	glBindVertexArray(lightVAO);
	glDrawElements(GL_TRIANGLES, lightModel.indexCount, GL_UNSIGNED_INT, 0);

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
		cameraOrbitAnim = !cameraOrbitAnim;
		break;
	case 'r':
		targetCameraAngle += 0.1f;
		break;
	case 'R':
		targetCameraAngle -= 0.1f;
		break;
	case 'y':
	case 'Y':
		lightRotating = !lightRotating;
		break;
	case 'z':
		cameraPos.z += 0.5f;
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
	case 'm':
		ambientEnabled = !ambientEnabled;
		std::cout << "주변 조명: " << (ambientEnabled ? "켜짐" : "꺼짐") << std::endl;
		break;
	case 'c':
		currentLightColor = (currentLightColor + 1) % 3;
		std::cout << "조명 색상 변경: " << currentLightColor << std::endl;
		break;
	case 's':
		lightRotating = false;
		std::cout << "조명 회전 멈춤" << std::endl;
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

	if (cameraOrbitAnim) {
		targetCameraAngle += 1.0f * delta;
	}

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

	// 조명 회전
	if (lightRotating) {
		lightAngle += 1.5f * delta;  // 초당 1.5 라디안 회전
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}

void Mouse(int button, int state, int x, int y) {
}

void Motion(int x, int y) {
}

void SpecialKeyboard(int key, int x, int y) {
	switch (key) {
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

	glutPostRedisplay();
}
