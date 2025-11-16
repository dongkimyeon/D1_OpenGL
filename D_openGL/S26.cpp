#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include "CommonInclude.h" // filetobuf 함수와 ObjRead.h 포함
#include <vector>
#include <cmath>

using namespace std;

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void SpecialKeyboard(int key, int x, int y); // 특수 키(화살표) 처리 함수 선언
void LoadOBJ(const char* filename, Model* model);
void Timer(int value);
glm::vec3 CalculateModelCenter(Model& model); // 모델 중심 계산 함수 선언
void CalculateNormals(Model& model); // 노멀 계산 함수 선언
void DrawLightOrbit(); // 조명 궤도 그리기 함수 선언

bool reverseFlag = false; //도형 회전 방향
bool xRotateFlag = false; //X축 회전 여부
bool yRotateFlag = false; //Y축 회전 여부
bool stopFlag = false; //회전 정지 여부
bool lightEnabled = true; // 조명 켜기/끄기
bool lightRotate = false; // 조명 공전 여부
int lightRotateDirection = 1; // 조명 공전 방향 (1: 양, -1: 음)
float lightOrbitAngle = 0.0f; // 조명 공전 각도
float lightDistance = 3.0f; // 조명 거리

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;

GLuint model0VAO, model0VBO, model0EBO;
GLuint model1VAO, model1VBO, model1EBO;
GLuint model2VAO, model2VBO, model2EBO;


GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO 추가
GLuint lightVAO, lightVBO, lightEBO; // 조명 표시용 VAO/VBO/EBO
GLuint orbitVAO, orbitVBO; // 궤도 그리기용 VAO/VBO

Model model0;
Model model1;
Model model2;
Model LightModel;

std::vector<glm::vec3> normals; // 노멀 벡터 저장
glm::vec3 modelCenter = glm::vec3(0.0f); // 모델 중심 추가
glm::vec3 lightPos = glm::vec3(3.0f, 0.0f, 0.0f); // 조명 위치
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // 조명 색상 (흰색)
int facenum = 0;  // 초기값 0으로 변경 (-1에서 0으로)
int endfacenum = 0; // endfacenum 변수 추가
int modelType = 2; // 0: Cube, 1: Cone

bool backfaceCulling = false; // 뒷면 제거 모드 여부
glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);
float cameraAngle = glm::atan(1.0f); // 초기 각도 (45도 라디안)
float targetCameraAngle = glm::atan(1.0f); // 타겟 카메라 각도

// 도형(모델) 이동 및 회전 관련 추가 전역 변수
glm::vec3 modelPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 targetModelPos = glm::vec3(0.0f, 0.0f, 0.0f);
float modelRotY = 0.0f;  // Y축 회전 각도 (라디안)
float targetModelRotY = 0.0f; // 타겟 Y축 회전 각도
float modelRotX = 0.0f;  // X축 회전 각도 (라디안)
float targetModelRotX = 0.0f; // 타겟 X축 회전 각도

float interpSpeed = 10.0f; // 선형 보간 속도

int main(int argc, char** argv) {
	srand(static_cast<unsigned>(time(0))); // 랜덤 시드 초기화
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // GLUT_DEPTH 추가
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Lighting Practice");

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

	LoadOBJ("sphere.obj", &model0);
	LoadOBJ("sphere.obj", &model1);
	LoadOBJ("sphere.obj", &model2);
	LoadOBJ("cube.obj", &LightModel);

	InitBuffers();

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard); // 특수 키(화살표) 함수 등록
	glutTimerFunc(16, Timer, 0); // 타이머 등록 (~60 FPS)

	glutMainLoop();
	return 0;
}

void CalculateNormals(Model& model) {
	normals.clear();
	normals.resize(model.vertex_count, glm::vec3(0.0f));

	// 각 면의 노멀을 계산하여 해당 버텍스에 누적
	for (size_t i = 0; i < model.face_count; ++i) {
		GLuint idx0 = model.faces[i].v1;
		GLuint idx1 = model.faces[i].v2;
		GLuint idx2 = model.faces[i].v3;

		glm::vec3 v0(model.vertices[idx0].x, model.vertices[idx0].y, model.vertices[idx0].z);
		glm::vec3 v1(model.vertices[idx1].x, model.vertices[idx1].y, model.vertices[idx1].z);
		glm::vec3 v2(model.vertices[idx2].x, model.vertices[idx2].y, model.vertices[idx2].z);

		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;
		glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

		normals[idx0] += normal;
		normals[idx1] += normal;
		normals[idx2] += normal;
	}

	// 누적된 노멀을 정규화
	for (size_t i = 0; i < normals.size(); ++i) {
		normals[i] = glm::normalize(normals[i]);
	}
}

void LoadOBJ(const char* filename, Model* model) {
	read_obj_file(filename, model);
	if (model->vertex_count == 0 || model->face_count == 0) {
		std::cerr << "OBJ 로드 실패: " << filename << std::endl;
	}
	else {
		std::cout << "OBJ 로드 완료: " << model->vertex_count << " 버텍스, " << model->face_count << " 페이스" << std::endl;
		modelCenter = CalculateModelCenter(*model); // 모델 중심 계산
		CalculateNormals(*model); // 노멀 계산
		std::cout << "모델 중심: " << modelCenter.x << ", " << modelCenter.y << ", " << modelCenter.z << std::endl;
	}
}

glm::vec3 CalculateModelCenter(Model& model) {
	if (model.vertex_count == 0) return glm::vec3(0.0f);

	glm::vec3 center(0.0f);
	for (size_t i = 0; i < model.vertex_count; ++i) {
		Vertex v = model.vertices[i];
		center.x += v.x;
		center.y += v.y;
		center.z += v.z;
	}
	center /= static_cast<float>(model.vertex_count);
	return center;
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
	// Model 0 버퍼 초기화
	glGenVertexArrays(1, &model0VAO);
	glBindVertexArray(model0VAO);

	std::vector<GLfloat> model0Data;
	std::vector<GLuint> model0Indices;
	
	CalculateNormals(model0);
	for (size_t i = 0; i < model0.vertex_count; ++i) {
		Vertex v = model0.vertices[i];
		model0Data.push_back(v.x);
		model0Data.push_back(v.y);
		model0Data.push_back(v.z);
		model0Data.push_back(normals[i].x);
		model0Data.push_back(normals[i].y);
		model0Data.push_back(normals[i].z);
	}
	for (size_t i = 0; i < model0.face_count; ++i) {
		model0Indices.push_back(model0.faces[i].v1);
		model0Indices.push_back(model0.faces[i].v2);
		model0Indices.push_back(model0.faces[i].v3);
	}

	glGenBuffers(1, &model0VBO);
	glBindBuffer(GL_ARRAY_BUFFER, model0VBO);
	glBufferData(GL_ARRAY_BUFFER, model0Data.size() * sizeof(GLfloat), model0Data.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &model0EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model0EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model0Indices.size() * sizeof(GLuint), model0Indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Model 1 버퍼 초기화
	glGenVertexArrays(1, &model1VAO);
	glBindVertexArray(model1VAO);

	std::vector<GLfloat> model1Data;
	std::vector<GLuint> model1Indices;
	
	CalculateNormals(model1);
	for (size_t i = 0; i < model1.vertex_count; ++i) {
		Vertex v = model1.vertices[i];
		model1Data.push_back(v.x);
		model1Data.push_back(v.y);
		model1Data.push_back(v.z);
		model1Data.push_back(normals[i].x);
		model1Data.push_back(normals[i].y);
		model1Data.push_back(normals[i].z);
	}
	for (size_t i = 0; i < model1.face_count; ++i) {
		model1Indices.push_back(model1.faces[i].v1);
		model1Indices.push_back(model1.faces[i].v2);
		model1Indices.push_back(model1.faces[i].v3);
	}

	glGenBuffers(1, &model1VBO);
	glBindBuffer(GL_ARRAY_BUFFER, model1VBO);
	glBufferData(GL_ARRAY_BUFFER, model1Data.size() * sizeof(GLfloat), model1Data.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &model1EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model1EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model1Indices.size() * sizeof(GLuint), model1Indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Model 2 버퍼 초기화
	glGenVertexArrays(1, &model2VAO);
	glBindVertexArray(model2VAO);

	std::vector<GLfloat> model2Data;
	std::vector<GLuint> model2Indices;
	
	CalculateNormals(model2);
	for (size_t i = 0; i < model2.vertex_count; ++i) {
		Vertex v = model2.vertices[i];
		model2Data.push_back(v.x);
		model2Data.push_back(v.y);
		model2Data.push_back(v.z);
		model2Data.push_back(normals[i].x);
		model2Data.push_back(normals[i].y);
		model2Data.push_back(normals[i].z);
	}
	for (size_t i = 0; i < model2.face_count; ++i) {
		model2Indices.push_back(model2.faces[i].v1);
		model2Indices.push_back(model2.faces[i].v2);
		model2Indices.push_back(model2.faces[i].v3);
	}

	glGenBuffers(1, &model2VBO);
	glBindBuffer(GL_ARRAY_BUFFER, model2VBO);
	glBufferData(GL_ARRAY_BUFFER, model2Data.size() * sizeof(GLfloat), model2Data.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &model2EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model2EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model2Indices.size() * sizeof(GLuint), model2Indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// 축 버퍼 초기화 (별도 VAO/VBO)
	glGenVertexArrays(1, &axisVAO);
	glBindVertexArray(axisVAO);

	glGenBuffers(1, &axisVBO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// 조명 표시용 작은 큐브 버퍼 초기화
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glGenBuffers(1, &lightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);

	GLfloat lightCubeVertices[] = {
		// 위치
		-0.1f, -0.1f, -0.1f,
		 0.1f, -0.1f, -0.1f,
		 0.1f,  0.1f, -0.1f,
		-0.1f,  0.1f, -0.1f,
		-0.1f, -0.1f,  0.1f,
		 0.1f, -0.1f,  0.1f,
		 0.1f,  0.1f,  0.1f,
		-0.1f,  0.1f,  0.1f
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(lightCubeVertices), lightCubeVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &lightEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);

	GLuint lightCubeIndices[] = {
		0, 1, 2, 2, 3, 0,  // 앞면
		1, 5, 6, 6, 2, 1,  // 오른쪽
		5, 4, 7, 7, 6, 5,  // 뒷면
		4, 0, 3, 3, 7, 4,  // 왼쪽
		3, 2, 6, 6, 7, 3,  // 위
		4, 5, 1, 1, 0, 4   // 아래
	};

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lightCubeIndices), lightCubeIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// 궤도 그리기용 버퍼 초기화
	glGenVertexArrays(1, &orbitVAO);
	glBindVertexArray(orbitVAO);

	glGenBuffers(1, &orbitVBO);
	glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void DrawLightOrbit() {
	// 원형 궤도 그리기
	const int segments = 100;
	std::vector<GLfloat> orbitVertices;

	for (int i = 0; i <= segments; ++i) {
		float angle = 2.0f * M_PI * i / segments;
		float x = modelCenter.x + lightDistance * cos(angle);
		float y = modelCenter.y;
		float z = modelCenter.z + lightDistance * sin(angle);
		orbitVertices.push_back(x);
		orbitVertices.push_back(y);
		orbitVertices.push_back(z);
	}

	// Uniform 위치 가져오기
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint objectColorLoc = glGetUniformLocation(shaderProgramID, "objectColor");

	// 모델 매트릭스 초기화
	glm::mat4 orbitModel = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(orbitModel));

	// 조명 효과 비활성화하고 회색으로 표시
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(objectColorLoc, 0.7f, 0.7f, 0.7f); // 밝은 회색

	glBindVertexArray(orbitVAO);
	glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
	glBufferData(GL_ARRAY_BUFFER, orbitVertices.size() * sizeof(GLfloat), orbitVertices.data(), GL_DYNAMIC_DRAW);

	glDrawArrays(GL_LINE_LOOP, 0, segments + 1);

	glBindVertexArray(0);
}

void DrawScene() {
	glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	if (backfaceCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	// 조명 공전 업데이트
	if (lightRotate) {
		lightPos.x = modelCenter.x + lightDistance * cos(lightOrbitAngle);
		lightPos.z = modelCenter.z + lightDistance * sin(lightOrbitAngle);
		lightPos.y = modelCenter.y;
	}

	// Uniform 매트릭스 매핑
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
	GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
	GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint objectColorLoc = glGetUniformLocation(shaderProgramID, "objectColor");
	GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");

	// 뷰와 투영 매트릭스 설정
	glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// 조명 uniform 설정
	if (lightEnabled) {
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
	}
	else {
		glUniform3f(lightColorLoc, 0.0f, 0.0f, 0.0f); 
	}
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

	// Model 0 그리기 (왼쪽)
	glm::mat4 model0Mat = glm::mat4(1.0f);
	model0Mat = glm::translate(model0Mat, glm::vec3(-2.0f, 0.0f, 0.0f));
	model0Mat = glm::rotate(model0Mat, glm::radians(modelRotY * 180.0f / 3.14159f), glm::vec3(0.0f, 1.0f, 0.0f));
	model0Mat = glm::scale(model0Mat, glm::vec3(3.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model0Mat));
	glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f); 
	
	glBindVertexArray(model0VAO);
	glDrawElements(GL_TRIANGLES, model0.face_count * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Model 1 그리기 (중앙)
	glm::mat4 model1Mat = glm::mat4(1.0f);
	model1Mat = glm::translate(model1Mat, glm::vec3(0.0f, 0.0f, 0.0f));
	model1Mat = glm::rotate(model1Mat, glm::radians(modelRotY * 180.0f / 3.14159f), glm::vec3(0.0f, 1.0f, 0.0f));
	model1Mat = glm::scale(model1Mat, glm::vec3(3.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1Mat));
	glUniform3f(objectColorLoc, 0.0f, 1.0f, 0.0f); // 초록색
	
	glBindVertexArray(model1VAO);
	glDrawElements(GL_TRIANGLES, model1.face_count * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Model 2 그리기 (오른쪽)
	glm::mat4 model2Mat = glm::mat4(1.0f);
	model2Mat = glm::translate(model2Mat, glm::vec3(2.0f, 0.0f, 0.0f));
	model2Mat = glm::rotate(model2Mat, glm::radians(modelRotY * 180.0f / 3.14159f), glm::vec3(0.0f, 1.0f, 0.0f));
	model2Mat = glm::scale(model2Mat, glm::vec3(3.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2Mat));
	glUniform3f(objectColorLoc, 0.0f, 0.0f, 1.0f); // 파란색
	
	glBindVertexArray(model2VAO);
	glDrawElements(GL_TRIANGLES, model2.face_count * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// 조명 위치에 Light Model 그리기
	glm::mat4 lightModelMat = glm::mat4(1.0f);
	lightModelMat = glm::translate(lightModelMat, lightPos);
	lightModelMat = glm::scale(lightModelMat, glm::vec3(0.2f)); // 작은 크기
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lightModelMat));
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // 조명 효과 비활성화
	glUniform3f(objectColorLoc, 1.0f, 1.0f, 0.0f); // 노란색 (조명 표시)

	glBindVertexArray(lightVAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	

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

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'd':
		targetCameraAngle -= glm::radians(5.0f);
		break;
	case 'a':
		targetCameraAngle += glm::radians(5.0f);
		break;
	case 'r': // 조명 공전 방향 토글
		lightRotate = !lightRotate;
		if (lightRotate) {
			lightRotateDirection *= -1;
		}
		std::cout << "조명 공전 " << (lightRotate ? "시작" : "정지")
			<< " (방향: " << (lightRotateDirection > 0 ? "양" : "음") << ")" << std::endl;
		break;
	case 'c':
		lightColor = glm::vec3(static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
			static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
			static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		break;
	case 'q':
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void SpecialKeyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		targetModelPos.x += 0.1f;
		break;
	case GLUT_KEY_DOWN:
		targetModelPos.x -= 0.1f;
		break;
	case GLUT_KEY_LEFT:
		targetModelPos.y += 0.1f;
		break;
	case GLUT_KEY_RIGHT:
		targetModelPos.y -= 0.1f;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void Timer(int value) {
	float delta = 1.0f / 60.0f;

	cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);
	float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z);
	cameraPos.x = r * glm::cos(cameraAngle);
	cameraPos.z = r * glm::sin(cameraAngle);

	if (stopFlag) {
		xRotateFlag = false;
		yRotateFlag = false;
	}

	if (yRotateFlag) {
		targetModelRotY += 0.05f;
	}

	if (lightRotate) {
		lightOrbitAngle += lightRotateDirection * 0.02f;
	}

	modelPos = glm::mix(modelPos, targetModelPos, interpSpeed * delta);
	modelRotX = glm::mix(modelRotX, targetModelRotX, interpSpeed * delta);
	modelRotY = glm::mix(modelRotY, targetModelRotY, interpSpeed * delta);

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}