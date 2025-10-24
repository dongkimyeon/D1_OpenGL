#define _CRT_SECURE_NO_WARNINGS
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
void LoadOBJ(const char* filename);
void Timer(int value);
glm::vec3 CalculateModelCenter(); // 모델 중심 계산 함수 선언
void InitSpline(); // 스플라인 초기화 함수 선언
glm::vec3 CatmullRomSpline(float t, const std::vector<glm::vec3>& points); // Catmull-Rom 스플라인 함수 선언


bool reverseFlag = false; //도형 회전 방향
bool xRotateFlag = false; //X축 회전 여부
bool yRotateFlag = false; //Y축 회전 여부
bool stopFlag = false; //회전 정지 여부
// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;
GLuint axisVAO, axisVBO;  // 축 그리기용 별도 VAO/VBO 추가

Model model;
glm::vec3 modelCenter = glm::vec3(0.0f); // 모델 중심 추가
int facenum = 0;  // 초기값 0으로 변경 (-1에서 0으로)
int endfacenum = 0; // endfacenum 변수 추가
int modelType = 1; // 0: Cube, 1: Cone
bool allFaceDraw = true; // true: 전체 면 그리기, false: 한 면씩 그리기
bool randomFace = false; // true: 랜덤 면 그리기, false: 순차 면 그리기
bool wireMode = false; // 와이어프레임 모드 여부
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

// Catmull-Rom 스플라인 관련 전역 변수
bool cameraWalk = false; // 카메라 워크 토글
float splineTime = 0.0f; // 스플라인 시간
float splineSpeed = 1.0f; // 스플라인 속도
std::vector<glm::vec3> controlPoints; // 제어점

glm::vec4 lAmb = glm::vec4(0.1, 0.3, 0.2, 1);	//빛의 ambient 값
glm::vec4 lDif = glm::vec4(0.3, 0.5, 0.3, 1);	//빛의 diffuse 값
glm::vec4 lSpc = glm::vec4(1, 1, 1, 1);		//빛의 specular 값

//float spec = ?f; // 재질의 specular 강도 (0~1)
glm::vec4 mAmb = glm::vec4(0.1, 0.4, 0.1, 1);	//재질의 ambient 값
glm::vec4 mDif = glm::vec4(0.8, 0.8, 0.8, 1);	//재질의 diffuse 값
//gllm::vec4 mSpc = glm::vec4(spec, spec, spec, 1);	//재질의 specular 값
float mShininess = 50.0f; // 재질의 sh값(게임수학)

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
	InitSpline(); // 스플라인 초기화

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard); // 특수 키(화살표) 함수 등록
	glutTimerFunc(16, Timer, 0); // 타이머 등록 (~60 FPS)

	glutMainLoop();
	return 0;
}

void InitSpline() {
	controlPoints = {
		glm::vec3(5.0f, 2.0f, 5.0f),
		glm::vec3(5.0f, 2.0f, -5.0f),
		glm::vec3(-5.0f, 2.0f, -5.0f),
		glm::vec3(-5.0f, 2.0f, 5.0f),
		glm::vec3(0.0f, 6.0f, 0.0f)
	};
	std::cout << "Catmull-Rom 스플라인 제어점 초기화 완료" << std::endl;
}

glm::vec3 CatmullRomSpline(float t, const std::vector<glm::vec3>& points) {
	size_t n = points.size();
	if (n < 2) return points[0];

	// Closed 스플라인을 위한 t 정규화 (0 ~ n)
	t = std::fmod(t, static_cast<float>(n));
	if (t < 0.0f) t += static_cast<float>(n);

	int i = static_cast<int>(std::floor(t));
	float u = t - static_cast<float>(i);

	// 인덱스 모듈로 처리 (closed)
	int idx0 = (i + n - 1) % n;
	int idx1 = i % n;
	int idx2 = (i + 1) % n;
	int idx3 = (i + 2) % n;

	glm::vec3 p0 = points[idx0];
	glm::vec3 p1 = points[idx1];
	glm::vec3 p2 = points[idx2];
	glm::vec3 p3 = points[idx3];

	// Catmull-Rom 베이시스 함수
	float u2 = u * u;
	float u3 = u2 * u;

	glm::vec3 a = 0.5f * (2.0f * p1);
	glm::vec3 b = 0.5f * (-p0 + p2);
	glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
	glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

	return a + b * u + c * u2 + d * u3;
}

void LoadOBJ(const char* filename) {
	read_obj_file(filename, &model);
	if (model.vertex_count == 0 || model.face_count == 0) {
		std::cerr << "OBJ 로드 실패: " << filename << std::endl;
	}
	else {
		std::cout << "OBJ 로드 완료: " << model.vertex_count << " 버텍스, " << model.face_count << " 페이스" << std::endl;
		modelCenter = CalculateModelCenter(); // 모델 중심 계산
		std::cout << "모델 중심: " << modelCenter.x << ", " << modelCenter.y << ", " << modelCenter.z << std::endl;
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

	// 모델 매트릭스:
	glm::mat4 modelMat = glm::mat4(1.0f);

	// 4. 전체 모델 위치 이동 적용 (modelPos)
	modelMat = glm::translate(modelMat, modelPos);

	// 1. 모델 중심으로 이동 (회전 기준점 설정)
	modelMat = glm::translate(modelMat, modelCenter);

	// 2. 회전 적용 (이제 중심을 기준으로 회전)
	modelMat = glm::rotate(modelMat, modelRotY, glm::vec3(0.0f, 1.0f, 0.0f));  // Y축 회전
	modelMat = glm::rotate(modelMat, modelRotX, glm::vec3(1.0f, 0.0f, 0.0f));  // X축 회전

	// 3. 중심에서 복귀 (회전 후 원래 중심 위치 복원)
	modelMat = glm::translate(modelMat, -modelCenter);



	//--------------------------------------------------------------------------
	// Camera (View) 및 Projection 매트릭스 설정
	glm::mat4 view = glm::lookAt(cameraPos, modelCenter, glm::vec3(0.0f, 1.0f, 0.0f)); // 뷰 매트릭스 (타겟을 modelCenter로 변경)
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
				if (wireMode) {
					glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
				}
				else {
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
				}
			}
		}

	}
	else if (modelType == 1) {  // 콘: face_count faces, 각 3 indices
		if (allFaceDraw) {
			for (size_t i = 0; i < model.face_count; ++i) {  // 9 faces
				int startIdx = i * 3;
				glm::vec3 color = colors[i % 6];  // 색상 6개 재사용
				glUniform3f(faceColorLoc, color.r, color.g, color.b);
				if (wireMode) {
					glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
				}
				else {
					glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(startIdx * sizeof(GLuint)));
				}

			}
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
	case 'd':
		if (!cameraWalk) {
			targetCameraAngle -= glm::radians(5.0f); // 카메라 각도 5도 감소
		}
		break;
	case 'a':
		if (!cameraWalk) {
			targetCameraAngle += glm::radians(5.0f); // 카메라 각도 5도 증가
		}
		break;
	case 'w':
		wireMode = !wireMode;
		return;
	case '=':
		cameraPos.x -= 1.0f;
		cameraPos.y -= 1.0f;
		cameraPos.z -= 1.0f;
		break;
	case '-':
		cameraPos.x += 1.0f;
		cameraPos.y += 1.0f;
		cameraPos.z += 1.0f;
		break;
	case 'u':
		modelPos.z -= 0.1f; // 모델을 카메라 쪽으로 이동
		break;
	case 'i':
		modelPos.z += 0.1f; // 모델을 카메라 반대쪽으로 이동
		break;
	case 'h':
		backfaceCulling = !backfaceCulling;
		break;
	case 'p':
		modelType = (modelType + 1) % 2; // 0과 1 사이 토글
		LoadOBJ(modelType == 0 ? "cube.obj" : "cone.obj"); // 모델 재로드
		// 이전 버퍼 삭제 및 재초기화
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		InitBuffers();  // 모델 버퍼 재초기화 (축은 유지)
		break;
	case 'c': // 'c' 키로 카메라 워크 토글
		cameraWalk = !cameraWalk;
		if (cameraWalk) {
			splineTime = 0.0f; // 재시작
			std::cout << "카메라 워크 시작 (Catmull-Rom 스플라인)" << std::endl;
		}
		else {
			std::cout << "카메라 워크 중지" << std::endl;
		}
		break;
	case 'x':
		xRotateFlag = !xRotateFlag;
		reverseFlag = true;
		break;
	case 'X':
		xRotateFlag = !xRotateFlag;
		reverseFlag = false;

		break;
	case 'y':
		yRotateFlag = !yRotateFlag;
		reverseFlag = true;

		break;
	case 'Y':
		yRotateFlag = !yRotateFlag;
		reverseFlag = false;
		break;
	case 's':
		targetModelPos = glm::vec3(0.0f, 0.0f, 0.0f); // 원점으로 이동
		stopFlag = !stopFlag;
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

	glutPostRedisplay();  // 재렌더링 요청
}

void Timer(int value) {
	float delta = 1.0f / 60.0f; // 고정 타임스텝 (~60 FPS)

	// 카메라 업데이트
	if (cameraWalk) {
		splineTime += delta * splineSpeed;
		
		cameraPos = CatmullRomSpline(splineTime, controlPoints);
	}
	else {
		// 기존 각도 기반 카메라 업데이트
		cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);
		float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z);
		cameraPos.x = r * glm::cos(cameraAngle);
		cameraPos.z = r * glm::sin(cameraAngle);
	}

	if (stopFlag)
	{
		xRotateFlag = false;
		yRotateFlag = false;
	}
	if (xRotateFlag) {
		if (reverseFlag)
			targetModelRotX += 0.05f;
		else
			targetModelRotX -= 0.05f;
	}
	if (yRotateFlag) {
		if (reverseFlag)
			targetModelRotY += 0.05f;
		else
			targetModelRotY -= 0.05f;
	}
	modelPos = glm::mix(modelPos, targetModelPos, interpSpeed * delta);
	modelRotX = glm::mix(modelRotX, targetModelRotX, interpSpeed * delta);
	modelRotY = glm::mix(modelRotY, targetModelRotY, interpSpeed * delta);
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0); // 다음 타이머 호출
}