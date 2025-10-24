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
void LoadOBJ(const char* filename);
void Timer(int value);
glm::vec3 CalculateModelCenter(); // 모델 중심 계산 함수 선언

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;
GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO 추가

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

bool w_Flag = false;
Model model;
glm::vec3 modelCenter = glm::vec3(0.0f); // 모델 중심 추가
int modelType = 0; // 0: Cube, 1: Cone

float rotateAngle = 0.0f;
bool backfaceCulling = false; // 뒷면 제거 모드 여부
glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);
float cameraAngle = glm::atan(1.0f); // 초기 각도 (45도 라디안)
float targetCameraAngle = glm::atan(1.0f); // 타겟 카메라 각도
float interpSpeed = 10.0f; // 선형 보간 속도
std::vector<glm::vec3> vertexColors; // 버텍스별 고정 랜덤 색상

std::vector<glm::mat4> faceRotationMatrices;

//명령어 관련 변수
bool modelRoatateFlag = false; // 모델 회전 여부
bool f_Flag = false;
float faceOpenAngle = 0.0f; // 누적 회전 각도 (f_Flag 토글 시 증가/정지)
glm::vec3 hingePosition = glm::vec3(0.0f, -0.5f, 0.5f); // 큐브 앞면의 아래쪽 가장자리 힌지 위치 예시 (큐브가 -0.5~0.5 범위라고 가정, 실제 OBJ에 맞게 조정)

bool s_Flag = false;
float faceRotateAngle = 0.0f; // 누적 회전 각도 (s_Flag 토글 시 증가/정지)

bool b_Flag = false;
float scale = 1.0f; // 누적 스케일 값 (b_Flag 토글 시 증가/정지)
bool scaleDecreasing = true; // true: 1->0 감소, false: 0->1 증가

bool t_Flag = false;
float faceZRotateAngle = 0.0f; // 누적 회전 각도 (t_Flag 토글 시 증가/감소)
bool r_Flag = false;
bool o_Flag = false;
float faceOpenAngle2 = 0.0f; // 누적 회전 각도 (r_Flag 토글 시 증가/감소)
float r_time = 0.0f;



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

	// OBJ 파일 로드
	if (modelType == 0)
		LoadOBJ("cube.obj");
	else
		LoadOBJ("cone.obj");

	// 버텍스 색상 초기화 (한 번만)
	vertexColors.resize(model.vertex_count);
	for (size_t i = 0; i < model.vertex_count; ++i) {
		float r = colorDist(gen);
		float g = colorDist(gen);
		float b = colorDist(gen);
		vertexColors[i] = glm::vec3(r, g, b);
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
void LoadOBJ(const char* filename) {
	read_obj_file(filename, &model);
	if (model.vertex_count == 0 || model.face_count == 0) {
		std::cerr << "OBJ 로드 실패: " << filename << std::endl;
		return;
	}
	std::cout << "OBJ 로드 완료: " << model.vertex_count << " 버텍스, " << model.face_count << " 페이스" << std::endl;
	modelCenter = CalculateModelCenter();
	std::cout << "모델 중심: " << modelCenter.x << ", " << modelCenter.y << ", " << modelCenter.z << std::endl;

	// 각 면 회전 매트릭스 초기화 (추가)
	faceRotationMatrices.resize(model.face_count);
	for (size_t i = 0; i < model.face_count; ++i) {
		faceRotationMatrices[i] = glm::mat4(1.0f); // identity matrix (회전 없음)
	}
}
glm::vec3 CalculateModelCenter() {
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

	// 모델 버퍼 초기화
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
	if (model.vertex_count == 0 || model.face_count == 0) {
		std::cerr << "OBJ 데이터가 로드되지 않았습니다." << std::endl;
		glutSwapBuffers();
		return;
	}

	// r_Flag 로직: active_face와 active_angle 계산
	int active_face = -1;
	float active_angle = 0.0f;
	if (r_Flag) {
		float cycle_time = r_time / 2.0f; // 각 면당 2초 (1초 열기, 1초 닫기)
		active_face = static_cast<int>(std::floor(cycle_time)) % 4;
		float local_time = std::fmod(cycle_time, 1.0f);
		if (local_time < 0.5f) {
			active_angle = 116.5f * (local_time / 0.5f); // 0 -> 90
		}
		else {
			active_angle = 116.5f * (1.0f - (local_time - 0.5f) / 0.5f); // 90 -> 0
		}
	}

	// 카메라 위치 업데이트
	float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z); // XZ 평면 거리
	cameraPos.x = r * glm::cos(cameraAngle);
	cameraPos.z = r * glm::sin(cameraAngle);

	if (w_Flag)
	{
		glDisable(GL_DEPTH_TEST);
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
	}
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	if (backfaceCulling) //리버스 
	{
		glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 깊이 버퍼 클리어 추가

	glUseProgram(shaderProgramID);

	// OBJ 버텍스 데이터: 위치 (x,y,z) + 색상 (r,g,b)
	std::vector<GLfloat> vertexData;
	for (size_t i = 0; i < model.vertex_count; ++i) {
		Vertex v = model.vertices[i];
		// OBJ 좌표를 OpenGL에 맞게 사용, 중심을 원점으로 이동
		float x = v.x - modelCenter.x;
		float y = v.y - modelCenter.y;
		float z = v.z - modelCenter.z;
		glm::vec3 col = vertexColors[i];
		vertexData.push_back(x);
		vertexData.push_back(y);
		vertexData.push_back(z);
		vertexData.push_back(col.r);
		vertexData.push_back(col.g);
		vertexData.push_back(col.b);
	}

	// 인덱스 데이터
	std::vector<GLuint> indices;
	for (size_t i = 0; i < model.face_count; ++i) {
		indices.push_back(model.faces[i].v1);
		indices.push_back(model.faces[i].v2);
		indices.push_back(model.faces[i].v3);
	}

	// 모델 VAO 바인드 및 데이터 업로드
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

	// Uniform 매트릭스 매핑
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

	// Camera (View) 및 Projection 매트릭스 설정
	glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 뷰 매트릭스
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f); // 프로젝션 매트릭스

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// DrawScene 내, Uniform 매트릭스 매핑 부분 아래에 루프 수정
	// (기존 modelMat은 전체 회전용으로 유지, 각 면 루프에서 곱함)

	if (modelType == 0) {  // 큐브: 6 faces
		for (int i = 0; i < 6; ++i) {
			// 각 면의 modelMat 계산: 전체 회전 + 면 개별 회전
			glm::mat4 localModelMat = glm::mat4(1.0f);
			localModelMat = glm::rotate(localModelMat, glm::radians(rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // 전체 Y축 회전

			if (i == 2) //면의 z축으로 회전
			{
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
				localModelMat = localModelMat * glm::rotate(glm::mat4(1.0f), glm::radians(faceZRotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), -glm::vec3(0.0f, 0.5f, 0.0f));
			}

			if (i == 5) {
				// 힌지 회전: translate(hinge) * rotate * translate(-hinge)
				// 주의: 매트릭스 곱셈 순서 (오른쪽에서 왼쪽으로 적용)
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), hingePosition);
				localModelMat = localModelMat * glm::rotate(glm::mat4(1.0f), glm::radians(faceOpenAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // X축 회전 (문처럼 아래서 열림, 축 변경 가능)
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), -hingePosition);
			}

			if (i == 1 || i == 3) {

				localModelMat = localModelMat * glm::rotate(glm::mat4(1.0f), glm::radians(faceRotateAngle), glm::vec3(1.0f, 0.0f, 0.0f));

			}

			if (i == 0)
			{
				localModelMat = localModelMat * glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));
			}

			// Uniform 업데이트
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(localModelMat));

			int startIdx = i * 6;
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
		}
	}
	else if (modelType == 1) {
		for (size_t i = 0; i < model.face_count; ++i) {
			// 각 면의 modelMat 계산
			glm::mat4 localModelMat = glm::mat4(1.0f);
			localModelMat = glm::rotate(localModelMat, glm::radians(rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // 전체
			localModelMat *= faceRotationMatrices[i];  // 개별

			// r_Flag: 한 면씩 번갈아 열기 (측면 i < 4만)
			if (r_Flag && i < 4 && static_cast<size_t>(active_face) == i) {
				glm::vec3 hinge_mid;
				glm::vec3 axis_dir;
				float rot_sign = 1.0f;
				if (i == 0) {
					hinge_mid = glm::vec3(0.0f, 0.0f - modelCenter.y, 0.5f);
					axis_dir = glm::vec3(1.0f, 0.0f, 0.0f);
					rot_sign = 1.0f;
				}
				else if (i == 1) {
					hinge_mid = glm::vec3(0.5f, 0.0f - modelCenter.y, 0.0f);
					axis_dir = glm::vec3(0.0f, 0.0f, 1.0f);
					rot_sign = -1.0f;
				}
				else if (i == 2) {
					hinge_mid = glm::vec3(0.0f, 0.0f - modelCenter.y, -0.5f);
					axis_dir = glm::vec3(1.0f, 0.0f, 0.0f);
					rot_sign = -1.0f;
				}
				else if (i == 3) {
					hinge_mid = glm::vec3(-0.5f, 0.0f - modelCenter.y, 0.0f);
					axis_dir = glm::vec3(0.0f, 0.0f, 1.0f);
					rot_sign = 1.0f;
				}
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), hinge_mid);
				localModelMat = localModelMat * glm::rotate(glm::mat4(1.0f), glm::radians(rot_sign * active_angle), axis_dir);
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), -hinge_mid);
			}

			if (i == 0)
			{
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f - modelCenter.y, 0.5f));
				localModelMat = localModelMat * glm::rotate(glm::mat4(1.0f), glm::radians(faceOpenAngle2), glm::vec3(1.0f, 0.0f, 0.0f));
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), -glm::vec3(0.0f, 0.0f - modelCenter.y, 0.5f));
			}
			if (i == 1)
			{
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f - modelCenter.y, 0.0f));
				localModelMat = localModelMat * glm::rotate(glm::mat4(1.0f), glm::radians(-faceOpenAngle2), glm::vec3(0.0f, 0.0f, 1.0f));
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), -glm::vec3(0.5f, 0.0f - modelCenter.y, 0.0f));
			}
			if (i == 2)
			{
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f - modelCenter.y, -0.5f));
				localModelMat = localModelMat * glm::rotate(glm::mat4(1.0f), glm::radians(-faceOpenAngle2), glm::vec3(1.0f, 0.0f, 0.0f));
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), -glm::vec3(0.0f, 0.0f - modelCenter.y, -0.5f));
			}
			if (i == 3)
			{
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f - modelCenter.y, 0.0f));
				localModelMat = localModelMat * glm::rotate(glm::mat4(1.0f), glm::radians(faceOpenAngle2), glm::vec3(0.0f, 0.0f, 1.0f));
				localModelMat = localModelMat * glm::translate(glm::mat4(1.0f), -glm::vec3(-0.5f, 0.0f - modelCenter.y, 0.0f));
			}


			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(localModelMat));

			int startIdx = i * 3;
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
		}
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
	float delta = 1.0f / 60.0f; // 고정 타임스텝 (~60 FPS)

	if (modelRoatateFlag) {
		rotateAngle += 90.0f * delta; // 초당 90도 회전 (기존 90으로 변경, 속도 조절 가능)
		if (rotateAngle >= 360.0f)
			rotateAngle -= 360.0f;
	}

	if (t_Flag)
	{
		faceZRotateAngle += 90.0f * delta;
		if (faceZRotateAngle >= 360.0f)
			faceZRotateAngle -= 360.0f;
	}

	if (f_Flag)
	{
		if (faceOpenAngle < 90.0f)
			faceOpenAngle += 90.0f * delta;
	}
	else
	{
		if (faceOpenAngle > 0.0f)
			faceOpenAngle -= 90.0f * delta;
	}

	if (s_Flag)
	{
		faceRotateAngle += 90.0f * delta;
		if (faceRotateAngle >= 360.0f)
			faceRotateAngle -= 360.0f;
	}

	if (b_Flag)
	{
		float scaleSpeed = 0.5f;
		if (scaleDecreasing) {
			scale -= scaleSpeed * delta;
			if (scale <= 0.0f) {
				scale = 0.0f;
				scaleDecreasing = false;
			}
		}
		else {
			scale += scaleSpeed * delta;
			if (scale >= 1.0f) {
				scale = 1.0f;
				scaleDecreasing = true;
			}
		}
	}

	if (o_Flag)
	{
		if (faceOpenAngle2 < 233.2f)
		{
			faceOpenAngle2 += 180.0f * delta;
			faceOpenAngle2 = glm::min(faceOpenAngle2, 233.2f);

		}
	}
	else
	{
		faceOpenAngle2 -= 180.0f * delta;
		if (faceOpenAngle2 < 0.0f)
			faceOpenAngle2 = 0.0f;
	}

	if (r_Flag) {
		r_time += delta;
	}


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
	case 'y':
		modelRoatateFlag = !modelRoatateFlag;
		break;
	case 't':
		t_Flag = !t_Flag;
		break;
	case 'f':
		f_Flag = !f_Flag;
		break;
	case 's':
		s_Flag = !s_Flag;
		break;
	case 'b':
		b_Flag = !b_Flag;
		break;
		break;
	case 'h':
		backfaceCulling = !backfaceCulling;
		break;
	case 'o':
		if(r_Flag)
		{
			break;
		}
		o_Flag = !o_Flag;
		break;
	case 'r':
		if (o_Flag)
		{
			break;
		}
		r_time = 0.0f;
		r_Flag = !r_Flag;
		break;
	case 'c':
		LoadOBJ("cube.obj");
		// 색상 재초기화
		vertexColors.resize(model.vertex_count);
		for (size_t i = 0; i < model.vertex_count; ++i) {
			float r = colorDist(gen);
			float g = colorDist(gen);
			float b = colorDist(gen);
			vertexColors[i] = glm::vec3(r, g, b);
		}
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		InitBuffers();
		break;
	case 'p':
		modelType = (modelType + 1) % 2; // 0과 1 사이 토글
		LoadOBJ(modelType == 0 ? "cube.obj" : "cone.obj"); // 모델 재로드
		InitBuffers();
		break;
	case 'w':
		w_Flag = !w_Flag;
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