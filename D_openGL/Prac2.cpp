#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" // filetobuf 함수와 ObjRead.h 포함

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

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;
GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO 추가

Model model;
int facenum = 0;  // 초기값 0으로 변경 (-1에서 0으로)
int modelType = 1; // 0: Cube, 1: Cone
bool allFaceDraw = true; // true: 전체 면 그리기, false: 한 면씩 그리기

glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);
float cameraAngle = glm::atan(1.0f); // 초기 각도 (45도 라디안)
float targetCameraAngle = glm::atan(1.0f); // 타겟 카메라 각도

// 도형(모델) 이동 및 회전 관련 추가 전역 변수
glm::vec3 modelPos = glm::vec3(0.0f, 0.0f, 0.0f);  // 도형 위치 (X, Y, Z 이동량)
glm::vec3 targetModelPos = glm::vec3(0.0f, 0.0f, 0.0f); // 타겟 도형 위치
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
	}
	else {
		std::cout << "OBJ 로드 완료: " << model.vertex_count << " 버텍스, " << model.face_count << " 페이스" << std::endl;
	}
}

void MakeVertexShaders() {
	GLchar* vertexSource = filetobuf("vertex.glsl");
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
	// 모델 버퍼 초기화
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	const GLsizei stride = 3 * sizeof(GLfloat);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);  // 언바인드 추가

	// 축 버퍼 초기화 (별도 VAO/VBO)
	glGenVertexArrays(1, &axisVAO);
	glBindVertexArray(axisVAO);

	glGenBuffers(1, &axisVBO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);  // stride=0 (positions only)
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void DrawScene() {
	if (model.vertex_count == 0 || model.face_count == 0) {
		std::cerr << "OBJ 데이터가 로드되지 않았습니다." << std::endl;
		glutSwapBuffers();
		return;
	}

	// 카메라 위치 업데이트
	float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z); // XZ 평면 거리
	cameraPos.x = r * glm::cos(cameraAngle);
	cameraPos.z = r * glm::sin(cameraAngle);

	glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glEnable(GL_CULL_FACE); // 은면 제거 활성화
	//glCullFace(GL_BACK); // 뒷면 제거 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 깊이 버퍼 클리어 추가

	glUseProgram(shaderProgramID);

	// OBJ 버텍스 데이터: 위치 (x,y,z)만 (색상은 uniform으로 처리)
	std::vector<GLfloat> vertexData;
	for (size_t i = 0; i < model.vertex_count; ++i) {
		Vertex v = model.vertices[i];
		// OBJ 좌표를 OpenGL에 맞게 사용
		float x = v.x;
		float y = v.y;
		float z = v.z;
		vertexData.push_back(x);
		vertexData.push_back(y);
		vertexData.push_back(z);
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
	GLint faceColorLoc = glGetUniformLocation(shaderProgramID, "faceColor");

	// 모델 매트릭스: 이동 + 회전 적용 (회전은 이동 후 적용되도록 순서 주의)
	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, modelPos);  // 먼저 이동 적용
	modelMat = glm::rotate(modelMat, modelRotY, glm::vec3(0.0f, 1.0f, 0.0f));  // Y축 회전
	modelMat = glm::rotate(modelMat, modelRotX, glm::vec3(1.0f, 0.0f, 0.0f));  // X축 회전
	//--------------------------------------------------------------------------
	// Camera (View) 및 Projection 매트릭스 설정
	glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 뷰 매트릭스
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f); // 프로젝션 매트릭스

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	//--------------------------------------------------------------------------
	// 큐브 각 면 색상 설정 및 그리기
	glm::vec3 colors[6] = {
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.5f, 0.0f)
	};

	// 공통 그리기 로직: modelType에 따라 분기
	if (modelType == 0) {  // 큐브: 6 faces, 각 6 indices (2 tris)
		if (allFaceDraw) {
			for (int i = 0; i < 6; ++i) {
				int startIdx = i * 6;
				glUniform3f(faceColorLoc, colors[i].r, colors[i].g, colors[i].b);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
			}
		}
		else {
			if (facenum < 0 || facenum >= 6) facenum = 0;
			int startIdx = facenum * 6;
			glUniform3f(faceColorLoc, colors[facenum].r, colors[facenum].g, colors[facenum].b);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
		}
	}
	else if (modelType == 1) {  // 콘: face_count faces, 각 3 indices
		if (allFaceDraw) {
			for (size_t i = 0; i < model.face_count; ++i) {  // 9 faces
				int startIdx = i * 3;
				glm::vec3 color = colors[i % 6];  // 색상 6개 재사용
				glUniform3f(faceColorLoc, color.r, color.g, color.b);
				glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
			}
		}
		else {
			if (facenum < 0 || facenum >= static_cast<int>(model.face_count)) facenum = 0;
			int startIdx = facenum * 3;
			glm::vec3 color = colors[facenum % 6];
			glUniform3f(faceColorLoc, color.r, color.g, color.b);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
		}
	}

	glBindVertexArray(0);

	//---------------------------------------------------------------------------
	// XYZ 축 그리기 (별도 VAO 사용)
	glm::mat4 axisModelMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(axisModelMat));

	std::vector<GLfloat> axesData = {
		// X축
		-5.0f, 0.0f, 0.0f,  // 시작
		 5.0f, 0.0f, 0.0f,  // 끝

		 // Y축
		  0.0f,-5.0f, 0.0f,
		  0.0f, 5.0f, 0.0f,

		  // Z축
		   0.0f, 0.0f,-5.0f,
		   0.0f, 0.0f, 5.0f
	};

	glBindVertexArray(axisVAO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, axesData.size() * sizeof(GLfloat), axesData.data(), GL_STATIC_DRAW);

	// X축
	glUniform3f(faceColorLoc, 1.0f, 0.0f, 0.0f);
	glDrawArrays(GL_LINES, 0, 2);

	// Y축 
	glUniform3f(faceColorLoc, 0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_LINES, 2, 2);

	// Z축 
	glUniform3f(faceColorLoc, 0.0f, 1.0f, 0.0f);
	glDrawArrays(GL_LINES, 4, 2);

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
	case 'a': //카메라를 Y축 중심으로 회전 
		targetCameraAngle += 0.1f; // 타겟 각도 증가 (라디안)
		break;
	case 'd':
		targetCameraAngle -= 0.1f; // 타겟 각도 감소 (라디안)
		break;
	case 'w': // 도형을 Z축 방향으로 앞으로 이동 (큐브를 카메라 쪽으로)
		targetModelPos.z += 0.1f;  // 타겟 Z 증가 (앞으로)
		break;
	case 's': // 도형을 Z축 방향으로 뒤로 이동
		targetModelPos.z -= 0.1f;  // 타겟 Z 감소 (뒤로)
		break;
	case '1':
		facenum = 0;
		break;
	case '2':
		facenum = 1;
		break;
	case '3':
		facenum = 2;
		break;
	case '4':
		facenum = 3;
		break;
	case '5':
		facenum = 4;
		break;
	case '6':
		facenum = 5;
		break;
	case 'o': // 전체 면 그리기 모드
		allFaceDraw = !allFaceDraw;
		break;
	case 'g':
		cameraPos.x += 10.0f; // 카메라 X축으로 오른쪽 이동
		cameraPos.y += 10.0f;
		cameraPos.z += 10.0f;
		break;
	case 'p':
		modelType = (modelType + 1) % 2; // 0과 1 사이 토글
		LoadOBJ(modelType == 0 ? "cube.obj" : "cone.obj"); // 모델 재로드
		// 이전 버퍼 삭제 및 재초기화
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		InitBuffers();  // 모델 버퍼 재초기화 (축은 유지)
		break;
	case 'q':
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

// 특수 키(화살표) 처리 함수: 도형 회전
void SpecialKeyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:    // 위 화살표: X축으로 도형 회전 (위로 기울임)
		targetModelRotX += 0.1f;  // 타겟 X축 회전 증가
		break;
	case GLUT_KEY_DOWN:  // 아래 화살표: X축으로 도형 회전 (아래로 기울임)
		targetModelRotX -= 0.1f;
		break;
	case GLUT_KEY_LEFT:  // 왼쪽 화살표: Y축으로 도형 좌회전
		targetModelRotY += 0.1f;  // 타겟 Y축 회전 증가
		break;
	case GLUT_KEY_RIGHT: // 오른쪽 화살표: Y축으로 도형 우회전
		targetModelRotY -= 0.1f;  // 타겟 Y축 회전 감소
		break;
	default:
		break;
	}

	glutPostRedisplay();  // 재렌더링 요청
}

void Timer(int value) {
	float delta = 1.0f / 60.0f; // 고정 타임스텝 (~60 FPS)

	// 선형 보간으로 부드럽게 업데이트
	modelPos = glm::mix(modelPos, targetModelPos, interpSpeed * delta);
	modelRotX = glm::mix(modelRotX, targetModelRotX, interpSpeed * delta);
	modelRotY = glm::mix(modelRotY, targetModelRotY, interpSpeed * delta);
	cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0); // 다음 타이머 호출
}