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

	// 모든 모델을 sphere.obj로 로드
	// 메인 모델 (중심 행성)
	LoadOBJ("sphere.obj", &model);
	modelCenters.push_back(CalculateModelCenter(&model));
	sphereScales.push_back(glm::vec3(5.0f)); // 메인 스케일
	vertexColorsList.push_back(std::vector<glm::vec3>());
	vertexColorsList.back().resize(model.vertex_count);
	for (size_t i = 0; i < model.vertex_count; ++i) {
		float r = colorDist(gen);
		float g = colorDist(gen);
		float b = colorDist(gen);
		vertexColorsList.back()[i] = glm::vec3(r, g, b);
	}

	// 행성들 로드 (예: 3개의 행성)
	for (int i = 0; i < 3; ++i) {
		Model p;
		LoadOBJ("sphere.obj", &p);
		planet.push_back(p);
		modelCenters.push_back(CalculateModelCenter(&planet.back()));
		sphereScales.push_back(glm::vec3(2.0f)); // 행성 스케일 점차 증가
		vertexColorsList.push_back(std::vector<glm::vec3>());
		vertexColorsList.back().resize(planet.back().vertex_count);
		for (size_t j = 0; j < planet.back().vertex_count; ++j) {
			float r = colorDist(gen);
			float g = colorDist(gen);
			float b = colorDist(gen);
			vertexColorsList.back()[j] = glm::vec3(r, g, b);
		}
	}

	// 위성들 로드 
	for (int i = 0; i < 3; ++i) {
		Model s;
		LoadOBJ("sphere.obj", &s);
		satellite.push_back(s);
		modelCenters.push_back(CalculateModelCenter(&satellite.back()));
		sphereScales.push_back(glm::vec3(0.8f)); // 위성 스케일 작게
		vertexColorsList.push_back(std::vector<glm::vec3>());
		vertexColorsList.back().resize(satellite.back().vertex_count);
		for (size_t j = 0; j < satellite.back().vertex_count; ++j) {
			float r = colorDist(gen);
			float g = colorDist(gen);
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

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
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

	glBindVertexArray(0);  // 언바인드 추가

	// 축 버퍼 초기화 (별도 VAO/VBO)
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

	// 궤도 버퍼 초기화 (별도 VAO/VBO)
	glGenVertexArrays(1, &orbitVAO);
	glGenBuffers(1, &orbitVBO);
	glBindVertexArray(orbitVAO);
	glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);

	// 속성 포인터 설정 (이게 핵심!)
	// - 위치 (location 0): 3 floats, stride=6*sizeof(float)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// - 색상 (location 1): 3 floats, offset=3 floats 후
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);  // unbind
}

void DrawScene() {
	// 카메라 위치 업데이트 (구면 좌표계 사용)
	float theta = cameraAngle;
	float beta = cameraYAngle; // 고도각 
	float r_xz = cameraRadius * glm::cos(beta);
	cameraPos.x = r_xz * glm::cos(theta);
	cameraPos.z = r_xz * glm::sin(theta);
	cameraPos.y = cameraRadius * glm::sin(beta);

	glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	if (backfaceCulling) //리버스 
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}
	else
	{
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
	glm::mat4 proj = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 100.0f); // 프로젝션 매트릭스

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// 현재 행성 위치들 계산
	std::vector<glm::vec3> planetPositions(3);
	float planetDist = 3.0f;
	float planetTheta = rotateAngle * 0.5f;
	for (int p = 0; p < 3; ++p) {
		int modelPlanetIdx = 1 + p;
		float tiltAngle = 0.0f;
		if (modelPlanetIdx == 1) tiltAngle = 0.0f;
		else if (modelPlanetIdx == 2) tiltAngle = 45.0f;
		else if (modelPlanetIdx == 3) tiltAngle = 135.0f;

		glm::mat4 tiltMat = glm::rotate(glm::mat4(1.0f), glm::radians(tiltAngle), glm::vec3(1.0f, 0.0f, 1.0f));
		glm::vec3 transVec(planetDist * glm::cos(planetTheta), 0.0f, planetDist * glm::sin(planetTheta));
		glm::vec4 transVec4 = tiltMat * glm::vec4(transVec, 1.0f);
		planetPositions[p] = glm::vec3(transVec4);
	}

	// 메인 모델 그리기
	{
		size_t modelIdx = 0;
		glm::mat4 localModelMat = glm::scale(glm::mat4(1.0f), sphereScales[modelIdx]);
		RenderModel(&model, modelIdx, localModelMat, modelLoc);
	}

	// 행성들 그리기
	for (size_t pIdx = 0; pIdx < planet.size(); ++pIdx) {
		size_t modelIdx = 1 + pIdx;

		int planetIdx = static_cast<int>(modelIdx);
		float angle = 0.0f;
		if (planetIdx == 1) {
			angle = 0.0f;
		}
		else if (planetIdx == 2) {
			angle = 45.0f;
		}
		else if (planetIdx == 3) {
			angle = 135.0f;
		}

		glm::mat4 localModelMat = glm::mat4(1.0f);
		localModelMat = glm::rotate(localModelMat, glm::radians(angle), glm::vec3(1.0f, 0.0f, 1.0f)); // 기울기 추가
		localModelMat = glm::translate(localModelMat, glm::vec3(planetDist * glm::cos(planetTheta), 0.0f, planetDist * glm::sin(planetTheta)));
		localModelMat = glm::scale(localModelMat, sphereScales[modelIdx]);

		RenderModel(&planet[pIdx], modelIdx, localModelMat, modelLoc);
	}

	// 위성들 그리기
	for (size_t sIdx = 0; sIdx < satellite.size(); ++sIdx) {
		size_t modelIdx = 1 + planet.size() + sIdx;

		// 위성: 해당 행성 주위 공전
		int pIdx = static_cast<int>(sIdx);
		int modelPlanetIdx = 1 + pIdx;
		float tiltAngle = 0.0f;
		if (modelPlanetIdx == 1) tiltAngle = 0.0f;
		else if (modelPlanetIdx == 2) tiltAngle = 45.0f;
		else if (modelPlanetIdx == 3) tiltAngle = 135.0f;

		glm::mat4 tiltMat = glm::rotate(glm::mat4(1.0f), glm::radians(tiltAngle), glm::vec3(1.0f, 0.0f, 1.0f));
		glm::vec3 planetPos = planetPositions[pIdx];

		float satDist = 0.8f;
		float satSpeed = 1.5f; // rhdw
		float satTheta = rotateAngle * satSpeed + glm::radians(90.0f * sIdx);

		glm::vec4 satRelLocal(satDist * glm::cos(satTheta), 0.0f, satDist * glm::sin(satTheta), 1.0f);
		glm::vec4 satRelTilted = tiltMat * satRelLocal;
		glm::vec3 satRel = glm::vec3(satRelTilted);

		glm::mat4 localModelMat = glm::mat4(1.0f);
		localModelMat = glm::translate(localModelMat, planetPos + satRel);
		localModelMat = glm::scale(localModelMat, sphereScales[modelIdx]);

		RenderModel(&satellite[sIdx], modelIdx, localModelMat, modelLoc);
	}

	glBindVertexArray(0);

	//---------------------------------------------------------------------------
	// XYZ 축 그리기 (별도 VAO 사용)
	glm::mat4 axisModelMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(axisModelMat));
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

	glDrawArrays(GL_LINES, 0, 6);

	glBindVertexArray(0);

	//---------------------------------------------------------------------------
	// 궤도 그리기 
	glBindVertexArray(orbitVAO);
	glm::mat4 orbitModelMat = glm::mat4(1.0f);
	float orbitRadius = 3.0f;
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

		orbitModelMat = glm::mat4(1.0f);
		if (j == 1) orbitModelMat = glm::rotate(orbitModelMat, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 1.0f));
		if (j == 2) orbitModelMat = glm::rotate(orbitModelMat, glm::radians(135.0f), glm::vec3(1.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(orbitModelMat));

		glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
		glBufferData(GL_ARRAY_BUFFER, orbitVertices.size() * sizeof(GLfloat), orbitVertices.data(), GL_DYNAMIC_DRAW);

		glDrawArrays(GL_LINE_LOOP, 0, segments + 1);
	}

	glBindVertexArray(0);
	glutSwapBuffers();
}

void Timer(int value) {
	rotateAngle += 0.05f; // 회전 각도 증가 (전체 시스템 회전)
	float delta = 1.0f / 60.0f; // 고정 타임스텝 (~60 FPS)


	// X 각도 보간
	cameraAngle = glm::mix(cameraAngle, targetCameraXAngle, interpSpeed * delta);

	// Y 각도 보간 추가
	cameraYAngle = glm::mix(cameraYAngle, targetCameraYAngle, interpSpeed * delta);

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
	case 'h':
		backfaceCulling = !backfaceCulling;
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