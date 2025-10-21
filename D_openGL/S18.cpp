#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES  // M_PI 사용을 위한 정의
#include "CommonInclude.h" // filetobuf 함수와 ObjRead.h 포함

// 정규 직교 기저를 위한 구조체
struct OrthonormalBasis {
	glm::vec3 right;   // X축 (우측 벡터)
	glm::vec3 up;      // Y축 (위쪽 벡터)  
	glm::vec3 forward; // Z축 (앞쪽 벡터)
	glm::vec3 origin;  // 원점 (모델의 중심점)
};

// 확장된 모델 구조체 (기존 Model에 정규 직교 기저 추가)
struct ExtendedModel {
	Model baseModel;
	OrthonormalBasis basis;
};

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void LoadOBJ(const char* filename, Model* model);
void Timer(int value);
glm::vec3 CalculateModelCenter(const Model& model); // 모델 중심 계산 함수 선언
void CenterModelPivot(Model* model); // 모델 pivot을 중심으로 이동하는 함수 선언

// 애니메이션 관련 함수들
void UpdateAnimation(float deltaTime);
void StartSwapThroughOrigin();
void StartSwapVertical();
void StartScaleAndRotate();
void SaveCurrentState();
void ResetToInitialState(); // 초기화 함수 추가
float EaseInOutQuad(float t); // 애니메이션 이징 함수

// 정규 직교 기저 관련 함수들
void InitializeOrthonormalBasis(ExtendedModel* extModel);
void DrawBasis(const OrthonormalBasis& basis, float length = 1.0f);

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint cubeVAO, cubeVBO, cubeEBO;
GLuint sphereVAO, sphereVBO, sphereEBO;
GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO

ExtendedModel cubeModel;
ExtendedModel sphereModel;

// 객체 선택 및 변환 상태
int selectedObject = 3; // 1: 큐브만, 2: 구만, 3: 둘 다
float cubeXRotation = 0.0f;
float cubeYRotation = 0.0f;
float sphereXRotation = 0.0f;
float sphereYRotation = 0.0f;
float globalYRotation = 0.0f; // 공전
glm::vec3 cubePosition = glm::vec3(2.0f, 0.0f, 0.0f);
glm::vec3 spherePosition = glm::vec3(-2.0f, 0.0f, 0.0f);
glm::vec3 cubeScale = glm::vec3(0.75f, 0.75f, 0.75f);
glm::vec3 sphereScale = glm::vec3(3.0f, 3.0f, 3.0f);

// 애니메이션 상태 변수들
enum AnimationState {
	ANIM_NONE,
	ANIM_SWAP_THROUGH_ORIGIN,    // t: 원점을 통과하며 자리 바꾸기
	ANIM_SWAP_VERTICAL,          // u: 수직으로 이동하며 자리 바꾸기  
	ANIM_SCALE_AND_ROTATE        // v: 확대/축소 + 자전/공전
};

AnimationState currentAnimation = ANIM_NONE;
float animationTime = 0.0f;
const float ANIMATION_DURATION = 3.0f; // 3초 애니메이션

// 애니메이션 시작 시 초기값 저장
glm::vec3 cubeStartPos;
glm::vec3 sphereStartPos;
glm::vec3 cubeStartScale;
glm::vec3 sphereStartScale;
float startCubeXRotation, startCubeYRotation;
float startSphereXRotation, startSphereYRotation;
float startGlobalYRotation;

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Two Shapes with Orthonormal Basis");

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
	LoadOBJ("cube.obj", &cubeModel.baseModel);
	LoadOBJ("sphere.obj", &sphereModel.baseModel);

	// 정규 직교 기저 초기화
	InitializeOrthonormalBasis(&cubeModel);
	InitializeOrthonormalBasis(&sphereModel);

	InitBuffers();

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, Timer, 0);

	glutMainLoop();
	return 0;
}

void LoadOBJ(const char* filename, Model* model) {
	read_obj_file(filename, model);
	if (model->vertex_count == 0 || model->face_count == 0) {
		std::cerr << "OBJ 로드 실패: " << filename << std::endl;
		return;
	}

	// 모델의 pivot을 중심점으로 조정
	CenterModelPivot(model);

	std::cout << "OBJ 로드 완료: " << model->vertex_count << " 버텍스, " << model->face_count << " 페이스" << std::endl;
}

glm::vec3 CalculateModelCenter(const Model& model) {
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

void CenterModelPivot(Model* model) {
	if (model->vertex_count == 0) return;

	// 모델의 중심점 계산
	glm::vec3 center = CalculateModelCenter(*model);

	// 모든 정점을 중심점만큼 이동시켜 pivot을 원점(0,0,0)으로 설정
	for (size_t i = 0; i < model->vertex_count; ++i) {
		model->vertices[i].x -= center.x;
		model->vertices[i].y -= center.y;
		model->vertices[i].z -= center.z;
	}

	std::cout << "모델 Pivot을 중심점으로 조정 완료: 기존 중심(" << center.x << ", " << center.y << ", " << center.z << ") -> (0, 0, 0)" << std::endl;
}

void MakeVertexShaders() {
	GLchar* vertexSource = filetobuf("S18Vertex.glsl");
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
	// 큐브 버퍼 초기화
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &cubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	const GLsizei stride = 3 * sizeof(GLfloat);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	// 구 버퍼 초기화
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);

	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &sphereEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	// 축 버퍼 초기화
	glGenVertexArrays(1, &axisVAO);
	glBindVertexArray(axisVAO);

	glGenBuffers(1, &axisVBO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void DrawModel(const Model& model, GLuint vao, GLuint vbo, GLuint ebo, glm::vec3 position, glm::vec3 color, glm::vec3 scale = glm::vec3(1.0f)) {
	if (model.vertex_count == 0 || model.face_count == 0) return;

	// 버텍스 데이터 준비
	std::vector<GLfloat> vertexData;
	for (size_t i = 0; i < model.vertex_count; ++i) {
		Vertex v = model.vertices[i];
		vertexData.push_back(v.x);
		vertexData.push_back(v.y);
		vertexData.push_back(v.z);
	}

	// 인덱스 데이터 준비
	std::vector<GLuint> indices;
	for (size_t i = 0; i < model.face_count; ++i) {
		indices.push_back(model.faces[i].v1);
		indices.push_back(model.faces[i].v2);
		indices.push_back(model.faces[i].v3);
	}

	// VAO 바인드 및 데이터 업로드
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

	// 모델 매트릭스 설정 (스케일 -> 위치 이동 순서)
	glm::mat4 modelMat = glm::mat4(1.0f);
	modelMat = glm::translate(modelMat, position);
	modelMat = glm::scale(modelMat, scale);

	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint faceColorLoc = glGetUniformLocation(shaderProgramID, "faceColor");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
	glUniform3f(faceColorLoc, color.r, color.g, color.b);

	// 모델 그리기
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void DrawScene() {
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	// Uniform 매트릭스 매핑
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

	// 기본 카메라 설정
	glm::vec3 cameraPos = glm::vec3(5.0f, 5.0f, 5.0f);
	glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// 큐브 그리기 (공전 -> 이동 -> 자전 -> 스케일 순서)
	glm::mat4 cubeModelMat = glm::mat4(1.0f);
	cubeModelMat = glm::rotate(cubeModelMat, glm::radians(globalYRotation), glm::vec3(0.0f, 1.0f, 0.0f)); // 1. 공전
	cubeModelMat = glm::translate(cubeModelMat, cubePosition); // 2. 위치 이동
	cubeModelMat = glm::rotate(cubeModelMat, glm::radians(cubeYRotation), glm::vec3(0.0f, 1.0f, 0.0f)); // 3. Y축 자전
	cubeModelMat = glm::rotate(cubeModelMat, glm::radians(cubeXRotation), glm::vec3(1.0f, 0.0f, 0.0f)); // 4. X축 자전
	cubeModelMat = glm::scale(cubeModelMat, cubeScale); // 5. 스케일링

	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint faceColorLoc = glGetUniformLocation(shaderProgramID, "faceColor");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModelMat));
	glUniform3f(faceColorLoc, 1.0f, 1.0f, 0.0f);

	// 큐브 렌더링
	if (cubeModel.baseModel.vertex_count > 0 && cubeModel.baseModel.face_count > 0) {
		std::vector<GLfloat> vertexData;
		for (size_t i = 0; i < cubeModel.baseModel.vertex_count; ++i) {
			Vertex v = cubeModel.baseModel.vertices[i];
			vertexData.push_back(v.x);
			vertexData.push_back(v.y);
			vertexData.push_back(v.z);
		}

		std::vector<GLuint> indices;
		for (size_t i = 0; i < cubeModel.baseModel.face_count; ++i) {
			indices.push_back(cubeModel.baseModel.faces[i].v1);
			indices.push_back(cubeModel.baseModel.faces[i].v2);
			indices.push_back(cubeModel.baseModel.faces[i].v3);
		}

		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}

	// 구 그리기 (공전 -> 이동 -> 자전 -> 스케일 순서)
	glm::mat4 sphereModelMat = glm::mat4(1.0f);
	sphereModelMat = glm::rotate(sphereModelMat, glm::radians(globalYRotation), glm::vec3(0.0f, 1.0f, 0.0f)); // 1. 공전
	sphereModelMat = glm::translate(sphereModelMat, spherePosition); // 2. 위치 이동
	sphereModelMat = glm::rotate(sphereModelMat, glm::radians(sphereYRotation), glm::vec3(0.0f, 1.0f, 0.0f)); // 3. Y축 자전
	sphereModelMat = glm::rotate(sphereModelMat, glm::radians(sphereXRotation), glm::vec3(1.0f, 0.0f, 0.0f)); // 4. X축 자전
	sphereModelMat = glm::scale(sphereModelMat, sphereScale); // 5. 스케일링

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sphereModelMat));
	glUniform3f(faceColorLoc, 0.0f, 1.0f, 0.0f);

	// 구 렌더링
	if (sphereModel.baseModel.vertex_count > 0 && sphereModel.baseModel.face_count > 0) {
		std::vector<GLfloat> vertexData;
		for (size_t i = 0; i < sphereModel.baseModel.vertex_count; ++i) {
			Vertex v = sphereModel.baseModel.vertices[i];
			vertexData.push_back(v.x);
			vertexData.push_back(v.y);
			vertexData.push_back(v.z);
		}

		std::vector<GLuint> indices;
		for (size_t i = 0; i < sphereModel.baseModel.face_count; ++i) {
			indices.push_back(sphereModel.baseModel.faces[i].v1);
			indices.push_back(sphereModel.baseModel.faces[i].v2);
			indices.push_back(sphereModel.baseModel.faces[i].v3);
		}

		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}

	// 정규 직교 기저 그리기 - 객체의 중심에 위치
	// 큐브의 정규 직교 기저 (큐브 내부 중심에 표시)
	glm::vec3 cubeWorldPos = glm::vec3(cubeModelMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	OrthonormalBasis cubeBasis = cubeModel.basis;

	// 큐브의 정규 직교 기저를 자전에 따라 회전시키기
	glm::mat4 cubeRotationMat4 = glm::mat4(1.0f);
	cubeRotationMat4 = glm::rotate(cubeRotationMat4, glm::radians(cubeYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
	cubeRotationMat4 = glm::rotate(cubeRotationMat4, glm::radians(cubeXRotation), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat3 cubeRotationMat = glm::mat3(cubeRotationMat4);

	cubeBasis.right = cubeRotationMat * cubeBasis.right;
	cubeBasis.up = cubeRotationMat * cubeBasis.up;
	cubeBasis.forward = cubeRotationMat * cubeBasis.forward;
	cubeBasis.origin = cubeWorldPos; // 큐브의 실제 월드 좌표 중심
	DrawBasis(cubeBasis, 1.0f); // 큐브 내부에 맞게 크기 조정

	// 구의 정규 직교 기저 (구 내부 중심에 표시)
	glm::vec3 sphereWorldPos = glm::vec3(sphereModelMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	OrthonormalBasis sphereBasis = sphereModel.basis;

	// 구의 정규 직교 기저를 자전에 따라 회전시키기
	glm::mat4 sphereRotationMat4 = glm::mat4(1.0f);
	sphereRotationMat4 = glm::rotate(sphereRotationMat4, glm::radians(sphereYRotation), glm::vec3(0.0f, 1.0f, 0.0f));
	sphereRotationMat4 = glm::rotate(sphereRotationMat4, glm::radians(sphereXRotation), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat3 sphereRotationMat = glm::mat3(sphereRotationMat4);

	sphereBasis.right = sphereRotationMat * sphereBasis.right;
	sphereBasis.up = sphereRotationMat * sphereBasis.up;
	sphereBasis.forward = sphereRotationMat * sphereBasis.forward;
	sphereBasis.origin = sphereWorldPos; // 구의 실제 월드 좌표 중심
	DrawBasis(sphereBasis, 1.0f); // 구 내부에 맞게 크기 조정

	// 전역 XYZ 축 그리기
	glm::mat4 axisModelMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(axisModelMat));

	std::vector<GLfloat> axesData = {
		-5.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f,
		0.0f, -5.0f, 0.0f, 0.0f, 5.0f, 0.0f,
		0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 5.0f
	};

	glBindVertexArray(axisVAO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, axesData.size() * sizeof(GLfloat), axesData.data(), GL_STATIC_DRAW);

	glUniform3f(faceColorLoc, 0.5f, 0.0f, 0.0f);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3f(faceColorLoc, 0.0f, 0.0f, 0.5f);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3f(faceColorLoc, 0.0f, 0.5f, 0.0f);
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

void Keyboard(unsigned char key, int x, int y) {
	const float rotationStep = 10.0f;
	const float moveStep = 0.2f;
	const float scaleStep = 0.1f;

	switch (key) {
	case 'q':
		exit(0);
		break;

		// 객체 선택
	case '1':
		selectedObject = 1; // 큐브만
		std::cout << "큐브만 선택됨" << std::endl;
		break;
	case '2':
		selectedObject = 2; // 구만
		std::cout << "구만 선택됨" << std::endl;
		break;
	case '3':
		selectedObject = 3; // 둘 다
		std::cout << "두 객체 모두 선택됨" << std::endl;
		break;

		// 애니메이션 명령들 (항상 두 모델에 적용)
	case 't':
		StartSwapThroughOrigin();
		std::cout << "원점을 통과하며 자리 바꾸기 애니메이션 시작" << std::endl;
		break;
	case 'u':
		StartSwapVertical();
		std::cout << "수직 이동하며 자리 바꾸기 애니메이션 시작" << std::endl;
		break;
	case 'v':
		StartScaleAndRotate();
		std::cout << "확대/축소 + 회전 애니메이션 시작" << std::endl;
		break;
	case 's':
		ResetToInitialState();
		std::cout << "초기 상태로 리셋" << std::endl;
		break;

		// X축 자전 (애니메이션 중이 아닐 때만)
	case 'x':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubeXRotation += rotationStep;
			}
			if (selectedObject == 2 || selectedObject == 3) {
				sphereXRotation += rotationStep;
			}
			std::cout << "X축 양방향 회전" << std::endl;
		}
		break;
	case 'X':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubeXRotation -= rotationStep;
			}
			if (selectedObject == 2 || selectedObject == 3) {
				sphereXRotation -= rotationStep;
			}
			std::cout << "X축 음방향 회전" << std::endl;
		}
		break;

		// Y축 자전 (애니메이션 중이 아닐 때만)
	case 'y':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubeYRotation += rotationStep;
			}
			if (selectedObject == 2 || selectedObject == 3) {
				sphereYRotation += rotationStep;
			}
			std::cout << "Y축 양방향 회전" << std::endl;
		}
		break;
	case 'Y':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubeYRotation -= rotationStep;
			}
			if (selectedObject == 2 || selectedObject == 3) {
				sphereYRotation -= rotationStep;
			}
			std::cout << "Y축 음방향 회전" << std::endl;
		}
		break;

		// Y축 공전 (애니메이션 중이 아닐 때만)
	case 'r':
		if (currentAnimation == ANIM_NONE) {
			globalYRotation += rotationStep;
			std::cout << "중앙 Y축 양방향 공전" << std::endl;
		}
		break;
	case 'R':
		if (currentAnimation == ANIM_NONE) {
			globalYRotation -= rotationStep;
			std::cout << "중앙 Y축 음방향 공전" << std::endl;
		}
		break;

		// 제자리 확대/축소 (애니메이션 중이 아닐 때만)
	case 'a':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubeScale += glm::vec3(scaleStep);
			}
			if (selectedObject == 2 || selectedObject == 3) {
				sphereScale += glm::vec3(scaleStep);
			}
			std::cout << "제자리 확대" << std::endl;
		}
		break;
	case 'A':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubeScale -= glm::vec3(scaleStep);
				if (cubeScale.x < 0.1f) cubeScale = glm::vec3(0.1f);
			}
			if (selectedObject == 2 || selectedObject == 3) {
				sphereScale -= glm::vec3(scaleStep);
				if (sphereScale.x < 0.1f) sphereScale = glm::vec3(0.1f);
			}
			std::cout << "제자리 축소" << std::endl;
		}
		break;

		// 원점 기준 확대/축소 (애니메이션 중이 아닐 때만)
	case 'b':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubePosition *= 1.1f;
				cubeScale += glm::vec3(scaleStep);
			}
			if (selectedObject == 2 || selectedObject == 3) {
				spherePosition *= 1.1f;
				sphereScale += glm::vec3(scaleStep);
			}
			std::cout << "원점 기준 확대" << std::endl;
		}
		break;
	case 'B':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubePosition *= 0.9f;
				cubeScale -= glm::vec3(scaleStep);
				if (cubeScale.x < 0.1f) cubeScale = glm::vec3(0.1f);
			}
			if (selectedObject == 2 || selectedObject == 3) {
				spherePosition *= 0.9f;
				sphereScale -= glm::vec3(scaleStep);
				if (sphereScale.x < 0.1f) sphereScale = glm::vec3(0.1f);
			}
			std::cout << "원점 기준 축소" << std::endl;
		}
		break;

		// X축 이동 (애니메이션 중이 아닐 때만)
	case 'd':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubePosition.x += moveStep;
			}
			if (selectedObject == 2 || selectedObject == 3) {
				spherePosition.x += moveStep;
			}
			std::cout << "X축 우측 이동" << std::endl;
		}
		break;
	case 'D':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubePosition.x -= moveStep;
			}
			if (selectedObject == 2 || selectedObject == 3) {
				spherePosition.x -= moveStep;
			}
			std::cout << "X축 좌측 이동" << std::endl;
		}
		break;

		// Y축 이동 (애니메이션 중이 아닐 때만)
	case 'e':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubePosition.y += moveStep;
			}
			if (selectedObject == 2 || selectedObject == 3) {
				spherePosition.y += moveStep;
			}
			std::cout << "Y축 위로 이동" << std::endl;
		}
		break;
	case 'E':
		if (currentAnimation == ANIM_NONE) {
			if (selectedObject == 1 || selectedObject == 3) {
				cubePosition.y -= moveStep;
			}
			if (selectedObject == 2 || selectedObject == 3) {
				spherePosition.y -= moveStep;
			}
			std::cout << "Y축 아래로 이동" << std::endl;
		}
		break;

	default:
		break;
	}
	glutPostRedisplay();
}

void Timer(int value) {
	// 애니메이션 업데이트 (60FPS 기준 약 16ms)
	if (currentAnimation != ANIM_NONE) {
		UpdateAnimation(0.016f);
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}

// 정규 직교 기저 관련 함수 구현
void InitializeOrthonormalBasis(ExtendedModel* extModel) {
	// 모델의 중심점을 원점으로 설정 (모델 로컬 좌표계의 중심)
	extModel->basis.origin = glm::vec3(0.0f, 0.0f, 0.0f); // 모델 중심이 원점

	// 기본 정규 직교 기저 설정 (월드 좌표계와 동일한 방향)
	extModel->basis.right = glm::vec3(1.0f, 0.0f, 0.0f);// X축
	extModel->basis.up = glm::vec3(0.0f, 1.0f, 0.0f);    // Y축  
	extModel->basis.forward = glm::vec3(0.0f, 0.0f, 1.0f); // Z축

	std::cout << "정규 직교 기저 초기화 완료 - 모델 중심이 원점" << std::endl;
}

void DrawBasis(const OrthonormalBasis& basis, float length) {
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	GLint faceColorLoc = glGetUniformLocation(shaderProgramID, "faceColor");

	// 항등 변환 매트릭스 설정
	glm::mat4 modelMat = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	// 기저 벡터들을 선분으로 그리기
	std::vector<GLfloat> basisData;

	// X축 (right vector) - 밝은 빨간색
	glm::vec3 xEnd = basis.origin + basis.right * length;
	basisData.insert(basisData.end(), { basis.origin.x, basis.origin.y, basis.origin.z });
	basisData.insert(basisData.end(), { xEnd.x, xEnd.y, xEnd.z });

	// Y축 (up vector) - 밝은 초록색
	glm::vec3 yEnd = basis.origin + basis.up * length;
	basisData.insert(basisData.end(), { basis.origin.x, basis.origin.y, basis.origin.z });
	basisData.insert(basisData.end(), { yEnd.x, yEnd.y, yEnd.z });

	// Z축 (forward vector) - 밝은 파란색
	glm::vec3 zEnd = basis.origin + basis.forward * length;
	basisData.insert(basisData.end(), { basis.origin.x, basis.origin.y, basis.origin.z });
	basisData.insert(basisData.end(), { zEnd.x, zEnd.y, zEnd.z });

	glBindVertexArray(axisVAO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, basisData.size() * sizeof(GLfloat), basisData.data(), GL_STATIC_DRAW);

	// X축 그리기 (밝은 빨간색)
	glUniform3f(faceColorLoc, 1.0f, 0.0f, 0.0f);
	glDrawArrays(GL_LINES, 0, 2);

	// Y축 그리기 (밝은 초록색)
	glUniform3f(faceColorLoc, 0.0f, 1.0f, 0.0f);
	glDrawArrays(GL_LINES, 2, 2);

	// Z축 그리기 (밝은 파란색)
	glUniform3f(faceColorLoc, 0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_LINES, 4, 2);
}

// 애니메이션 관련 함수들 구현
float EaseInOutQuad(float t) {
	return t < 0.5f ? 2.0f * t * t : 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
}

void SaveCurrentState() {
	cubeStartPos = cubePosition;
	sphereStartPos = spherePosition;
	cubeStartScale = cubeScale;
	sphereStartScale = sphereScale;
	startCubeXRotation = cubeXRotation;
	startCubeYRotation = cubeYRotation;
	startSphereXRotation = sphereXRotation;
	startSphereYRotation = sphereYRotation;
	startGlobalYRotation = globalYRotation;
}

void UpdateAnimation(float deltaTime) {
	animationTime += deltaTime;
	float progress = animationTime / ANIMATION_DURATION;

	if (progress >= 1.0f) {
		// 애니메이션 완료
		progress = 1.0f;
		currentAnimation = ANIM_NONE;
		animationTime = 0.0f;
	}

	float easedProgress = EaseInOutQuad(progress);

	switch (currentAnimation) {
	case ANIM_SWAP_THROUGH_ORIGIN:
	{
		// t: 원점을 통과하며 자리 바꾸기
		glm::vec3 cubeTarget = sphereStartPos;
		glm::vec3 sphereTarget = cubeStartPos;

		cubePosition = cubeStartPos + (cubeTarget - cubeStartPos) * easedProgress;
		spherePosition = sphereStartPos + (sphereTarget - sphereStartPos) * easedProgress;
	}
	break;

	case ANIM_SWAP_VERTICAL:
	{
		// u: 수직으로 이동하며 자리 바꾸기
		glm::vec3 cubeTarget = sphereStartPos;
		glm::vec3 sphereTarget = cubeStartPos;

		float height = 3.0f;
		float cubeHeight = sin(easedProgress * M_PI) * height;
		float sphereHeight = -sin(easedProgress * M_PI) * height;

		cubePosition.x = cubeStartPos.x + (cubeTarget.x - cubeStartPos.x) * easedProgress;
		cubePosition.y = cubeHeight;
		cubePosition.z = cubeStartPos.z + (cubeTarget.z - cubeStartPos.z) * easedProgress;

		spherePosition.x = sphereStartPos.x + (sphereTarget.x - sphereStartPos.x) * easedProgress;
		spherePosition.y = sphereHeight;
		spherePosition.z = sphereStartPos.z + (sphereTarget.z - sphereStartPos.z) * easedProgress;
	}
	break;

	case ANIM_SCALE_AND_ROTATE:
	{
		// v: 확대/축소 + 자전/공전
		float cubeScaleFactor = 1.0f + sin(progress * M_PI * 4.0f) * 0.5f;
		float sphereScaleFactor = 1.0f - sin(progress * M_PI * 4.0f) * 0.3f;

		cubeScale = cubeStartScale * cubeScaleFactor;
		sphereScale = sphereStartScale * sphereScaleFactor;

		// 자전 (기존 회전값에서 증분)
		cubeXRotation = startCubeXRotation + progress * 720.0f; // 2바퀴
		cubeYRotation = startCubeYRotation + progress * 360.0f; // 1바퀴
		sphereXRotation = startSphereXRotation + progress * 480.0f; // 1.33바퀴
		sphereYRotation = startSphereYRotation + progress * 240.0f; // 0.67바퀴

		// 공전 (기존 공전값에서 증분)
		globalYRotation = startGlobalYRotation + progress * 360.0f; // 1바퀴
	}
	break;

	default:
		break;
	}
}

void StartSwapThroughOrigin() {
	if (currentAnimation != ANIM_NONE) return;

	currentAnimation = ANIM_SWAP_THROUGH_ORIGIN;
	animationTime = 0.0f;
	SaveCurrentState();
}

void StartSwapVertical() {
	if (currentAnimation != ANIM_NONE) return;

	currentAnimation = ANIM_SWAP_VERTICAL;
	animationTime = 0.0f;
	SaveCurrentState();
}

void StartScaleAndRotate() {
	if (currentAnimation != ANIM_NONE) return;

	currentAnimation = ANIM_SCALE_AND_ROTATE;
	animationTime = 0.0f;
	SaveCurrentState();
}

void ResetToInitialState() {
	// 애니메이션 중단
	currentAnimation = ANIM_NONE;
	animationTime = 0.0f;

	// 모든 변환 변수들을 초기값으로 리셋
	cubePosition = glm::vec3(2.0f, 0.0f, 0.0f);
	spherePosition = glm::vec3(-2.0f, 0.0f, 0.0f);
	cubeScale = glm::vec3(0.75f, 0.75f, 0.75f);
	sphereScale = glm::vec3(3.0f, 3.0f, 3.0f);

	cubeXRotation = 0.0f;
	cubeYRotation = 0.0f;
	sphereXRotation = 0.0f;
	sphereYRotation = 0.0f;
	globalYRotation = 0.0f;

	// 객체 선택도 초기값으로 리셋
	selectedObject = 3; // 둘 다 선택

	// 정규 직교 기저도 초기화
	InitializeOrthonormalBasis(&cubeModel);
	InitializeOrthonormalBasis(&sphereModel);

	std::cout << "모든 변환이 초기 상태로 리셋되었습니다." << std::endl;
}