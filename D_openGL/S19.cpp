#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" // filetobuf 함수와 ObjRead.h 포함
#include <random>
#include <cmath>
#include <vector>

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
void LoadOBJ(const char* filename, Model* targetModel);
void Timer(int value);
glm::vec3 CalculateModelCenter(Model* model); // 모델 중심 계산 함수 선언 (Model* 추가)
std::vector<GLfloat> PrepareVertexData(Model* model, size_t modelIdx);
std::vector<GLuint> PrepareIndices(Model* model);
void RenderModel(Model* model, size_t modelIdx, const glm::mat4& modelMat, GLint modelLoc);

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;
GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO 추가
GLuint orbitVAO, orbitVBO; // 궤도 그리기용 VAO/VBO

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

Model model; // 메인 모델 (중심 행성)
std::vector<Model> planet; // 행성들 (sphere.obj로 로드)
std::vector<Model> satellite; // 위성들 (sphere.obj로 로드)
std::vector<glm::vec3> modelCenters; // 각 모델의 중심 저장 (model + planet + satellite)
std::vector<glm::vec3> sphereScales; // 각 모델의 스케일 (기본 4.0f, 행성/위성은 조정 가능)
std::vector<std::vector<glm::vec3>> vertexColorsList; // 각 모델별 버텍스 색상 리스트

float rotateAngle = 0.0f;
bool backfaceCulling = false; // 뒷면 제거 모드 여부
glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);
float cameraRadius = 0.0f; // 카메라 반지름 추가
float cameraAngle = 0.0f; // 초기 X-Z 각도 (45도 라디안)
float cameraYAngle = 0.0f; // 초기 Y 각도 (고도각)
float targetCameraXAngle = 0.0f; // 타겟 X-Z 카메라 각도
float targetCameraYAngle = 0.0f; // 타겟 Y 카메라 각도
float interpSpeed = 10.0f; // 선형 보간 속도

glm::vec3 moveVec(0.0f ,0.0f, 0.0f);
glm::vec3 targetMoveVec(0.0f, 0.0f, 0.0f);

float tiltAngle0 = 0.0f;
float tiltAngle1 = 45.0f;
float tiltAngle2 = 135.0f;
// 명령어 변수

float planetDist = 3.0f;
float targetPlanetDist = 3.0f;
bool mFlag = true;
bool pFlag = true;
bool zFlag = true;
bool xFlag = true;
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // GLUT_DEPTH 추가
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Prac");

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

	// 카메라 초기 설정
	cameraRadius = glm::length(cameraPos);
	float r_xz_init = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z);
	cameraAngle = atan2(cameraPos.z, cameraPos.x);
	cameraYAngle = atan2(cameraPos.y, r_xz_init);
	targetCameraXAngle = cameraAngle;
	targetCameraYAngle = cameraYAngle;

	// 중심 행성
	LoadOBJ("sphere.obj", &model);
	modelCenters.push_back(CalculateModelCenter(&model));
	sphereScales.push_back(glm::vec3(5.0f)); // 메인 스케일
	vertexColorsList.push_back(std::vector<glm::vec3>());
	vertexColorsList.back().resize(model.vertex_count);
	for (size_t i = 0; i < model.vertex_count; ++i) {
		float r = 0.0f;
		float g = 1.0f;
		float b = 1.0f;
		vertexColorsList.back()[i] = glm::vec3(r, g, b);
	}

	// 행성
	for (int i = 0; i < 3; ++i) {
		Model p;
		LoadOBJ("sphere.obj", &p);
		planet.push_back(p);
		modelCenters.push_back(CalculateModelCenter(&planet.back()));
		sphereScales.push_back(glm::vec3(2.0f)); // 행성 스케일 점차 증가
		vertexColorsList.push_back(std::vector<glm::vec3>());
		vertexColorsList.back().resize(planet.back().vertex_count);
		for (size_t j = 0; j < planet.back().vertex_count; ++j) {
			float r = 0.0f;
			float g = 0.0f;
			float b = 1.0f;
			vertexColorsList.back()[j] = glm::vec3(r, g, b);
		}
	}

	// 위성
	for (int i = 0; i < 3; ++i) {
		Model s;
		LoadOBJ("sphere.obj", &s);
		satellite.push_back(s);
		modelCenters.push_back(CalculateModelCenter(&satellite.back()));
		sphereScales.push_back(glm::vec3(0.8f)); // 위성 스케일 작게
		vertexColorsList.push_back(std::vector<glm::vec3>());
		vertexColorsList.back().resize(satellite.back().vertex_count);
		for (size_t j = 0; j < satellite.back().vertex_count; ++j) {
			float r = 0.0f;
			float g = 1.0f;
			float b = 0.0f;
			vertexColorsList.back()[j] = glm::vec3(r, g, b);
		}
	}

	InitBuffers();

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard); // 특수 키(화살표) 함수 등록
	glutTimerFunc(16, Timer, 0); // 타이머 등록 (~60 FPS)


	// *** 추가: 프로그램 시작 시 명령어 목록 출력 (디버그용) ***
	std::cout << "=== OpenGL 행성 시뮬레이션 명령어 목록 ===" << std::endl;
	std::cout << "m: 솔리드/와이어프레임 모드 토글 (mFlag)" << std::endl;
	std::cout << "p: 원근투영(Perspective)/직각투영(Orthographic) 토글 (pFlag)" << std::endl;
	std::cout << "z: 회전 방향 반전 및 틸트 각도 초기화 (zFlag = false, tilt=0)" << std::endl;
	std::cout << "Z: 회전 방향 정상 및 틸트 각도 복원 (zFlag = true, tilt=0/45/135)" << std::endl;
	std::cout << "y: 행성 거리 증가 (targetPlanetDist +0.5, max 6.0)" << std::endl;
	std::cout << "Y: 행성 거리 감소 (targetPlanetDist -0.5, min 1.5)" << std::endl;
	std::cout << "w: 월드 Y축 앞으로 이동 (targetMoveVec.y +0.05)" << std::endl;
	std::cout << "s: 월드 Y축 뒤로 이동 (targetMoveVec.y -0.05)" << std::endl;
	std::cout << "a: 월드 X축 왼쪽 이동 (targetMoveVec.x -0.05)" << std::endl;
	std::cout << "d: 월드 X축 오른쪽 이동 (targetMoveVec.x +0.05)" << std::endl;
	std::cout << "-: 월드 Z축 앞으로 이동 (targetMoveVec.z -0.05)" << std::endl;
	std::cout << "=: 월드 Z축 뒤로 이동 (targetMoveVec.z +0.05)" << std::endl;
	std::cout << "h: 백페이스 컬링 토글 (backfaceCulling)" << std::endl;
	std::cout << "x: 축(XYZ) 및 궤도 표시 토글 (xFlag)" << std::endl;
	std::cout << "q: 프로그램 종료" << std::endl;
	std::cout << "화살표 키: 카메라 회전 (좌/우: X-Z, 상/하: Y 각도)" << std::endl;
	std::cout << "================================" << std::endl;


	glutMainLoop();
	return 0;
}

void LoadOBJ(const char* filename, Model* targetModel) {
	read_obj_file(filename, targetModel);
	if (targetModel->vertex_count == 0 || targetModel->face_count == 0) {
		std::cerr << "OBJ 로드 실패: " << filename << std::endl;
		return;
	}
	std::cout << "OBJ 로드 완료: " << targetModel->vertex_count << " 버텍스, " << targetModel->face_count << " 페이스" << std::endl;
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

std::vector<GLfloat> PrepareVertexData(Model* model, size_t modelIdx) {
	std::vector<GLfloat> vertexData;
	glm::vec3 center = modelCenters[modelIdx];
	for (size_t i = 0; i < model->vertex_count; ++i) {
		Vertex v = model->vertices[i];
		// 중심으로 이동
		float x = v.x - center.x;
		float y = v.y - center.y;
		float z = v.z - center.z;
		glm::vec3 col = vertexColorsList[modelIdx][i];
		vertexData.push_back(x);
		vertexData.push_back(y);
		vertexData.push_back(z);
		vertexData.push_back(col.r);
		vertexData.push_back(col.g);
		vertexData.push_back(col.b);
	}
	return vertexData;
}

std::vector<GLuint> PrepareIndices(Model* model) {
	std::vector<GLuint> indices;
	for (size_t i = 0; i < model->face_count; ++i) {
		indices.push_back(model->faces[i].v1);
		indices.push_back(model->faces[i].v2);
		indices.push_back(model->faces[i].v3);
	}
	return indices;
}

void RenderModel(Model* model, size_t modelIdx, const glm::mat4& modelMat, GLint modelLoc) {
	if (model->vertex_count == 0 || model->face_count == 0) return;

	auto vertexData = PrepareVertexData(model, modelIdx);
	auto indices = PrepareIndices(model);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	if (mFlag)
	{
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawElements(GL_LINE_LOOP, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
	}

}

void MakeVertexShaders() {
	GLchar* vertexSource = filetobuf("vertex3D.glsl");
	if (!vertexSource) {
		std::cerr << "ERROR: vertex.glsl 파일을 읽지 못했습니다." << std::endl;
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
	const GLsizei stride = 6 * sizeof(GLfloat);

	// 모델 버퍼 초기화 (공통 VAO/VBO 사용, DrawScene에서 동적 업데이트)
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);  

	// 축 버퍼 초기화 
	glGenVertexArrays(1, &axisVAO);
	glBindVertexArray(axisVAO);

	glGenBuffers(1, &axisVBO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// 궤도 버퍼 초기화
	glGenVertexArrays(1, &orbitVAO);
	glGenBuffers(1, &orbitVBO);
	glBindVertexArray(orbitVAO);
	glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);  
}

void DrawScene() {
	// 카메라 위치 업데이트 (구면 좌표계 사용)
	float theta = cameraAngle;
	float beta = cameraYAngle; // 고도각 
	float r_xz = cameraRadius * glm::cos(beta);
	cameraPos.x = r_xz * glm::cos(theta);
	cameraPos.z = r_xz * glm::sin(theta);
	cameraPos.y = cameraRadius * glm::sin(beta);

	if (pFlag) {
		glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
	}
	else {
		glDisable(GL_DEPTH_TEST); // 깊이 테스트 비활성화
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	if (backfaceCulling) { //리버스 
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 깊이 버퍼 클리어 추가

	glUseProgram(shaderProgramID);

	// Uniform 매트릭스 매핑
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");
	 
	// Camera (View) 및 Projection 매트릭스 설정
	glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 뷰 매트릭스

	glm::mat4 proj;
	if (pFlag) {
		//원근
		proj = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 100.0f);
		
	}
	else {
		//직교
		proj = glm::ortho(-10.0f, 10.0f, -10.0f * (float)height / (float)width, 10.0f * (float)height / (float)width, 0.1f, 100.0f);
		
	}
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// 추가: 전체 월드 이동 매트릭스 (모든 객체에 공통 적용)
	glm::mat4 worldMat = glm::translate(glm::mat4(1.0f), moveVec);

	// 현재 행성 위치들 계산 (원점 기준, moveVec는 worldMat으로 적용)
	std::vector<glm::vec3> planetPositions(3);
	float planetTheta = rotateAngle * 0.5f;
	for (int p = 0; p < 3; ++p) {
		int modelPlanetIdx = 1 + p;
		float tiltAngle = 0.0f;
		float offsetAngle = 120.0f * p; // 행성 간 120도 간격
		if (modelPlanetIdx == 1) tiltAngle = tiltAngle0;
		else if (modelPlanetIdx == 2) tiltAngle = tiltAngle1;
		else if (modelPlanetIdx == 3) tiltAngle = tiltAngle2;

		glm::mat4 tiltMat = glm::rotate(glm::mat4(1.0f), glm::radians(tiltAngle), glm::vec3(1.0f, 0.0f, 1.0f));
		glm::vec3 transVec(planetDist * glm::cos(planetTheta + offsetAngle), 0.0f, planetDist * glm::sin(planetTheta + offsetAngle));
		glm::vec4 transVec4 = tiltMat * glm::vec4(transVec, 1.0f);
		planetPositions[p] = glm::vec3(transVec4);
	}

	// 메인 모델 그리기 (로컬: scale → 월드: translate(moveVec))
	{
		size_t modelIdx = 0;
		glm::mat4 localModelMat = glm::mat4(1.0f);
		localModelMat = glm::scale(localModelMat, sphereScales[modelIdx]);  // 로컬 변환
		glm::mat4 finalMat = worldMat * localModelMat;  // 추가: worldMat 곱셈
		RenderModel(&model, modelIdx, finalMat, modelLoc);
	}

	// 행성들 그리기 (로컬: rotate(tilt) → translate(relative) → scale → 월드)
	for (size_t pIdx = 0; pIdx < planet.size(); ++pIdx) {
		size_t modelIdx = 1 + pIdx;

		int planetIdx = static_cast<int>(modelIdx);
		float angle = 0.0f;
		float offsetAngle = 120.0f * pIdx; // 행성 간 120도 간격
		if (planetIdx == 1) {
			angle = tiltAngle0;
		}
		else if (planetIdx == 2) {
			angle = tiltAngle1;
		}
		else if (planetIdx == 3) {
			angle = tiltAngle2;
		}

		glm::mat4 localModelMat = glm::mat4(1.0f);
		localModelMat = glm::rotate(localModelMat, glm::radians(angle), glm::vec3(1.0f, 0.0f, 1.0f));  // 로컬: tilt
		localModelMat = glm::translate(localModelMat, glm::vec3(planetDist * glm::cos(planetTheta + offsetAngle), 0.0f, planetDist * glm::sin(planetTheta + offsetAngle)));  // 로컬: relative 위치
		localModelMat = glm::scale(localModelMat, sphereScales[modelIdx]);  // 로컬: scale

		glm::mat4 finalMat = worldMat * localModelMat;  // 추가: worldMat 곱셈 (moveVec 적용)
		RenderModel(&planet[pIdx], modelIdx, finalMat, modelLoc);
	}

	// 위성들 그리기 (로컬: translate(planetPos + satRel) → scale → 월드)
	for (size_t sIdx = 0; sIdx < satellite.size(); ++sIdx) {
		size_t modelIdx = 1 + planet.size() + sIdx;

		// 위성: 해당 행성 주위 공전
		int pIdx = static_cast<int>(sIdx);
		int modelPlanetIdx = 1 + pIdx;
		float tiltAngle = 0.0f;
		if (modelPlanetIdx == 1) tiltAngle = tiltAngle0;
		else if (modelPlanetIdx == 2) tiltAngle = tiltAngle1;
		else if (modelPlanetIdx == 3) tiltAngle = tiltAngle2;

		glm::mat4 tiltMat = glm::rotate(glm::mat4(1.0f), glm::radians(tiltAngle), glm::vec3(1.0f, 0.0f, 1.0f));
		glm::vec3 planetPos = planetPositions[pIdx];

		float satDist = 0.8f;
		float satSpeed = 1.5f; // 행성보다 빠른 공전 속도
		float satTheta = rotateAngle * satSpeed + glm::radians(90.0f * sIdx);

		glm::vec4 satRelLocal(satDist * glm::cos(satTheta), 0.0f, satDist * glm::sin(satTheta), 1.0f);
		glm::vec4 satRelTilted = tiltMat * satRelLocal;
		glm::vec3 satRel = glm::vec3(satRelTilted);

		glm::mat4 localModelMat = glm::mat4(1.0f);
		localModelMat = glm::translate(localModelMat, planetPos + satRel);  // 로컬: 행성 + 상대 위치
		localModelMat = glm::scale(localModelMat, sphereScales[modelIdx]);  // 로컬: scale

		glm::mat4 finalMat = worldMat * localModelMat;  // 추가: worldMat 곱셈
		RenderModel(&satellite[sIdx], modelIdx, finalMat, modelLoc);
	}

	glBindVertexArray(0);

	//---------------------------------------------------------------------------
	// XYZ 축 그리기 (로컬: identity → 월드)
	{
		glm::mat4 localAxisMat = glm::mat4(1.0f);  // 축은 로컬 변환 없음
		glm::mat4 finalAxisMat = localAxisMat;  // 추가: worldMat 곱셈
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(finalAxisMat));
		std::vector<GLfloat> axesData = {
			// X축 (red)
			-5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // 시작
			 5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // 끝

			 // Y축 (blue)
			  0.0f,-5.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			  0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 1.0f,

			  // Z축 (green)
			   0.0f, 0.0f,-5.0f, 0.0f, 1.0f, 0.0f,
			   0.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f
		};

		glBindVertexArray(axisVAO);
		glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
		glBufferData(GL_ARRAY_BUFFER, axesData.size() * sizeof(GLfloat), axesData.data(), GL_STATIC_DRAW);
		if (xFlag)
			glDrawArrays(GL_LINES, 0, 6);

		glBindVertexArray(0);
	}

	//---------------------------------------------------------------------------
	// 궤도 그리기 (로컬: rotate(tilt) → 월드)
	glBindVertexArray(orbitVAO);
	float orbitRadius = planetDist;
	int segments = 100;

	for (int j = 0; j < 3; j++) {
		std::vector<GLfloat> orbitVertices;
		for (int i = 0; i <= segments; ++i) {
			float theta = 2.0f * glm::pi<float>() * float(i) / float(segments);
			float x = orbitRadius * glm::cos(theta);
			float z = orbitRadius * glm::sin(theta);
			orbitVertices.push_back(x);
			orbitVertices.push_back(0.0f);
			orbitVertices.push_back(z);
			orbitVertices.push_back(1.0f);
			orbitVertices.push_back(1.0f);
			orbitVertices.push_back(1.0f);
		}

		float tilt = 0.0f;
		if (j == 0) tilt = tiltAngle0;
		else if (j == 1) tilt = tiltAngle1;
		else if (j == 2) tilt = tiltAngle2;

		glm::mat4 localOrbitMat = glm::rotate(glm::mat4(1.0f), glm::radians(tilt), glm::vec3(1.0f, 0.0f, 1.0f));  // 로컬: tilt
		glm::mat4 finalOrbitMat = worldMat * localOrbitMat;  // 추가: worldMat 곱셈

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(finalOrbitMat));

		glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
		glBufferData(GL_ARRAY_BUFFER, orbitVertices.size() * sizeof(GLfloat), orbitVertices.data(), GL_DYNAMIC_DRAW);
		if (xFlag)
			glDrawArrays(GL_LINE_LOOP, 0, segments + 1);
	
	}

	glBindVertexArray(0);
	glutSwapBuffers();
}
void Timer(int value) 
{
	if (zFlag)
		rotateAngle += 0.05f; // 회전 각도 증가 
	else
		rotateAngle -= 0.05f; // 회전 각도 증가
	float delta = 1.0f / 60.0f; // 고정 타임스텝 (~60 FPS)


	// X 각도 보간
	cameraAngle = glm::mix(cameraAngle, targetCameraXAngle, interpSpeed * delta);

	// Y 각도 보간 추가
	cameraYAngle = glm::mix(cameraYAngle, targetCameraYAngle, interpSpeed * delta);

	planetDist = glm::mix(planetDist, targetPlanetDist, interpSpeed * delta);

	// 이동 벡터 보간
	moveVec = glm::mix(moveVec, targetMoveVec, interpSpeed * delta);
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0); // 다음 타이머 호출
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
	case 'm':
		mFlag = !mFlag;
		break;
	case 'p':
		pFlag = !pFlag;
		break;
	case 'z':
		zFlag = false;
		tiltAngle0 = 0.0f;
		tiltAngle1 = 0.0f;
		tiltAngle2 = 0.0f;
		break;
	case 'Z':
		zFlag = true;
		tiltAngle0 = 0.0f;
		tiltAngle1 = 45.0f;
		tiltAngle2 = 135.0f;
		break;
	case 'y':
		if (targetPlanetDist < 6.0f)
			targetPlanetDist += 0.5f;
		break;
	case 'Y':
		if (targetPlanetDist > 1.5f)
			targetPlanetDist -= 0.5f;
		break;
	case '-':
		targetMoveVec.z -= 0.05f;
		break;
	case '=':
		targetMoveVec.z += 0.05f;
		break;
	case 'w':
		targetMoveVec.y += 0.05f;
		break;
	case 's':
		targetMoveVec.y -= 0.05f;
		break;
	case 'a':
		targetMoveVec.x -= 0.05f;
		break;
	case 'd':
		targetMoveVec.x += 0.05f;
		break;
	case 'h':
		backfaceCulling = !backfaceCulling;
		break;
	case 'x':
		xFlag = !xFlag;
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
	glutPostRedisplay();
}