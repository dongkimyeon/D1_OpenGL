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
void Motion(int x, int y);

glm::vec3 CalculateModelCenter(Model* model);

// 전역 변수
GLint width = 1280, height = 720;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO 추가


struct ExtendedModel {
	Model baseModel;
	glm::vec3 center;
};

ExtendedModel cube;


float cameraAngle = glm::atan(1.0f); // 초기 각도 (45도 라디안)
float targetCameraAngle = glm::atan(1.0f); // 타겟 카메라 각도

// 카메라 위치 관련
glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);
float interpSpeed = 10.0f; // 선형 보간 속도

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

	read_obj_file("cube.obj", &cube.baseModel);

	InitBuffers();

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
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
	// 사각형 버퍼 초기화
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	// 위치 속성 (location 0): x,y,z
	const GLsizei stride = 6 * sizeof(GLfloat);  // 위치(3) + 색상(3)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	// 색상 속성 (location 1): r,g,b
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);  // 언바인드


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

void DrawScene() {
	if (cube.baseModel.vertex_count == 0 || cube.baseModel.face_count == 0) {
		std::cerr << "OBJ 데이터가 로드되지 않았습니다." << std::endl;
		glutSwapBuffers();
		return;
	}

	float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z); // XZ 평면 거리
	cameraPos.x = r * glm::cos(cameraAngle);
	cameraPos.z = r * glm::sin(cameraAngle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	std::vector<GLfloat> vertexData;
	glm::vec3 colors[4] = {
		glm::vec3(1.0f, 0.0f, 0.0f),  // 왼쪽 아래 - 빨강
		glm::vec3(0.0f, 1.0f, 0.0f),  // 오른쪽 아래 - 초록
		glm::vec3(0.0f, 0.0f, 1.0f),  // 오른쪽 위 - 파랑
		glm::vec3(1.0f, 1.0f, 0.0f)   // 왼쪽 위 - 노랑
	};

	for (size_t i = 0; i < cube.baseModel.vertex_count; ++i) {
		Vertex v = cube.baseModel.vertices[i];
		// 위치
		float x = v.x - CalculateModelCenter(&cube.baseModel).x;
		float y = v.y - CalculateModelCenter(&cube.baseModel).y;
		float z = v.z - CalculateModelCenter(&cube.baseModel).z;
		vertexData.push_back(x);
		vertexData.push_back(y);
		vertexData.push_back(z);
		// 색상 
		vertexData.push_back(colors[i % 4].r);
		vertexData.push_back(colors[i % 4].g);
		vertexData.push_back(colors[i % 4].b);
	}

	// 인덱스 데이터
	std::vector<GLuint> indices;
	for (size_t i = 0; i < cube.baseModel.face_count; ++i) {
		indices.push_back(cube.baseModel.faces[i].v1);
		indices.push_back(cube.baseModel.faces[i].v2);
		indices.push_back(cube.baseModel.faces[i].v3);
	}

	// VAO 바인드 및 데이터 업로드
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

	// Uniform 매트릭스 설정
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

	// 모델 매트릭스
	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::rotate(modelMat, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축 회전
	modelMat = glm::scale(modelMat, glm::vec3(1.5f)); // 스케일링 (필요시 조정)


	glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));


	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	//---------------------------------------------------------------------------
	// XYZ 축 그리기
	glm::mat4 AxisMat = glm::mat4(1.0f);  // 축은 로컬 변환 없음
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

	if (ImGui::Button("test"))
	{
		std::cout << "Test button clicked!" << std::endl;
	}

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
	// --- ImGui 키보드 입력 전달 ---
	ImGui_ImplGLUT_KeyboardFunc(key, x, y);

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) // ImGui가 키보드를 사용 중이면
	{
		glutPostRedisplay(); // ImGui 입력을 반영하기 위해 다시 그리기
		return;              // 프로그램의 키보드 로직은 무시
	}
	// --- ImGui 처리 끝 ---


	switch (key) {
	case 'a': // 카메라를 Y축 중심으로 왼쪽 회전
		targetCameraAngle += 0.1f; // 타겟 각도 증가 (라디안)
		break;
	case 'd': // 카메라를 Y축 중심으로 오른쪽 회전
		targetCameraAngle -= 0.1f; // 타겟 각도 감소 (라디안)
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
	float delta = 1.0f / 60.0f; // 고정 타임스텝 (~60 FPS)

	// 선형 보간으로 카메라 각도 부드럽게 업데이트
	cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0); // 다음 타이머 호출
}

// --- ImGui를 위한 마우스/모션 함수 추가 (Default_GL.cpp에서 가져옴) ---

void Mouse(int button, int state, int x, int y) {
	ImGui_ImplGLUT_MouseFunc(button, state, x, y);

	// ImGui가 마우스를 사용하지 않을 때만 사용자의 마우스 로직 처리
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		// (S20의 마우스 클릭 로직이 있다면 여기에 추가)
	}
	// ImGui는 Timer에서 어차피 다시 그리므로 여기서 PostRedisplay 불필요
}


void Motion(int x, int y) {
	ImGui_ImplGLUT_MotionFunc(x, y);

	// ImGui가 마우스를 사용하지 않을 때만 사용자의 마우스 로직 처리
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		// (S20의 마우스 이동 로직이 있다면 여기에 추가)
	}
	// ImGui는 Timer에서 어차피 다시 그리므로 여기서 PostRedisplay 불필요
}