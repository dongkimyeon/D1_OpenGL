#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" // filetobuf 함수와 ObjRead.h 포함
#include <random>
#include <cmath>

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

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;
GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO 추가

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
float cameraAngle = glm::atan(1.0f); // 초기 각도 (45도 라디안)
float targetCameraAngle = glm::atan(1.0f); // 타겟 카메라 각도
float interpSpeed = 10.0f; // 선형 보간 속도

//명령어 관련 변수

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

	// 위성들 로드 (예: 각 행성당 1-2개 위성, 총 5개)
	for (int i = 0; i < 5; ++i) {
		Model s;
		LoadOBJ("sphere.obj", &s);
		satellite.push_back(s);
		modelCenters.push_back(CalculateModelCenter(&satellite.back()));
		sphereScales.push_back(glm::vec3(0.5f)); // 위성 스케일 작게
		vertexColorsList.push_back(std::vector<glm::vec3>());
		vertexColorsList.back().resize(satellite.back().vertex_count);
		for (size_t j = 0; j < satellite.back().vertex_count; ++j) {
			float r = colorDist(gen);
			float g = colorDist(gen);
			float b = colorDist(gen);
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
}

void DrawScene() {
	// 카메라 위치 업데이트
	float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z); // XZ 평면 거리
	cameraPos.x = r * glm::cos(cameraAngle);
	cameraPos.z = r * glm::sin(cameraAngle);

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
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f); // 프로젝션 매트릭스

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// 모든 모델 그리기 (메인 + 행성 + 위성)
	std::vector<Model*> allModels = { &model };
	for (auto& p : planet) allModels.push_back(&p);
	for (auto& s : satellite) allModels.push_back(&s);

	for (size_t modelIdx = 0; modelIdx < allModels.size(); ++modelIdx) {
		Model* currentModel = allModels[modelIdx];
		if (currentModel->vertex_count == 0 || currentModel->face_count == 0) continue;

		// OBJ 버텍스 데이터: 위치 (x,y,z) + 색상 (r,g,b)
		std::vector<GLfloat> vertexData;
		for (size_t i = 0; i < currentModel->vertex_count; ++i) {
			Vertex v = currentModel->vertices[i];
			// OBJ 좌표를 OpenGL에 맞게 사용, 중심을 원점으로 이동
			float x = v.x - modelCenters[modelIdx].x;
			float y = v.y - modelCenters[modelIdx].y;
			float z = v.z - modelCenters[modelIdx].z;
			glm::vec3 col = vertexColorsList[modelIdx][i];
			vertexData.push_back(x);
			vertexData.push_back(y);
			vertexData.push_back(z);
			vertexData.push_back(col.r);
			vertexData.push_back(col.g);
			vertexData.push_back(col.b);
		}

		// 인덱스 데이터
		std::vector<GLuint> indices;
		for (size_t i = 0; i < currentModel->face_count; ++i) {
			indices.push_back(currentModel->faces[i].v1);
			indices.push_back(currentModel->faces[i].v2);
			indices.push_back(currentModel->faces[i].v3);
		}

		// 모델 VAO 바인드 및 데이터 업로드
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

		// 각 모델의 modelMat 계산: 전체 회전 + 위치/스케일
		glm::mat4 localModelMat = glm::mat4(1.0f);
		
		if (modelIdx == 0) {
			// 메인 모델: 원점
			localModelMat = glm::scale(localModelMat, sphereScales[modelIdx]);
		}
		else if (modelIdx < 1 + planet.size()) {
			// 행성
			int planetIdx = modelIdx;
			float dist =  planetIdx * 2.0f;
			
			localModelMat = glm::rotate(localModelMat, glm::radians(45.f), glm::vec3(0.0f, 0.0f, 1.0f)); // 기울기 추가
			localModelMat = glm::translate(localModelMat, glm::vec3(dist * glm::cos(rotateAngle * 0.5f), 0.0f, dist * glm::sin(rotateAngle * 0.5f)));
			localModelMat = glm::scale(localModelMat, sphereScales[modelIdx]);
		}
		else {
	
		}

		// Uniform 업데이트
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(localModelMat));

		// 각 면 그리기 (기존 루프 유지, 하지만 전체 인덱스로 변경)
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
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
	glutSwapBuffers();
}

void Timer(int value) {
	rotateAngle += 0.05f; // 회전 각도 증가 (전체 시스템 회전)
	float delta = 1.0f / 60.0f; // 고정 타임스텝 (~60 FPS)

	cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);
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
		targetCameraAngle -= glm::radians(5.0f); // 카메라 각도 5도 감소
		break;
	case GLUT_KEY_LEFT:
		targetCameraAngle += glm::radians(5.0f); // 카메라 각도 5도 증가
		break;
	}
	glutPostRedisplay();
}