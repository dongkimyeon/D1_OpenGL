#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include "CommonInclude.h"
#include "ObjRead.h"

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);
void DrawWalls();
void DrawCubes();
void DrawGrid();
void LoadCubeModel();
void UpdatePhysics(float deltaTime);
void DrawGravityVector(); // 중력 벡터 그리기 함수 추가
void LoadSphereModel(); // 구 모델 로드 함수 추가
void DrawSpheres(); // 구 그리기 함수 추가
void UpdateSpherePhysics(float deltaTime); // 구 물리 업데이트 함수 추가
void CreateSphere(); // 구 생성 함수 추가

// 전역 변수
GLint width = 800, height = 600;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint wallVAO, wallVBO;
GLuint cubeVAO, cubeVBO;
GLuint gridVAO, gridVBO;
GLuint gravityVAO, gravityVBO; // 중력 벡터용 VAO, VBO 추가
GLuint sphereVAO, sphereVBO; // 구용 VAO, VBO 추가

// OBJ 모델 데이터
Model cubeModel;
Model sphereModel; // 구 모델 추가

// 마우스 회전 변수
bool isMousePressed = false;
int mouseX, mouseY;
float rotateX = 0.0f, rotateY = 0.0f;

// 카메라 설정
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
float cameraOffsetZ = 0.0f;
float cameraRotationY = 0.0f;

// 벽 회전 변수 추가
float wallRotationX = 0.0f;
float wallRotationY = 0.0f;
float wallRotationZ = 0.0f;

// 공간 경계
const float ROOM_SIZE = 4.0f;
const float FLOOR_Y = -4.0f;  // 바닥을 더 아래로 이동

// 바닥 열림 상태
bool floorOpen = false;

// 중력 벡터 표시 설정 추가
bool showGravityVector = true;  // 중력 벡터 표시 여부

// 구 정보 추가
struct Sphere {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 color;
	float radius;
	bool active;
};

std::vector<Sphere> spheres;
const int MAX_SPHERES = 5;

// 육면체 정보
struct Cube {
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 color;
	glm::vec3 velocity;  // 속도 추가
	bool onGround;    // 바닥에 있는지 확인
};

std::vector<Cube> cubes = {
	{ glm::vec3(0.0f, FLOOR_Y + 0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.5f, 1.0f), glm::vec3(0.0f), false },  // 마젠타 큐브 (앞으로 이동)
	{ glm::vec3(0.0f, FLOOR_Y + 0.75f, -2.0f), glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(1.0f, 0.5f, 0.5f), glm::vec3(0.0f), false }, // 시안 큐브 (뒤로 이동)
	{ glm::vec3(0.0f, FLOOR_Y + 0.25f, 2.0f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 1.0f), glm::vec3(0.0f), false }  // 오렌지 큐브 (그대로)
};

// 큐브 초기 위치 저장
std::vector<glm::vec3> initialCubePositions = {
	glm::vec3(0.0f, FLOOR_Y + 0.5f, 0.0f),   // 마젠타 큐브 초기 위치
	glm::vec3(0.0f, FLOOR_Y + 0.75f, -2.0f), // 시안 큐브 초기 위치
	glm::vec3(0.0f, FLOOR_Y + 0.25f, 2.0f)   // 오렌지 큐브 초기 위치
};

// 물리 상수
const float GRAVITY = 9.8f;  // 양수로 변경 (중력 크기)
const float BOUNCE_DAMPING = 0.7f;  // 바운스 감쇠 (0.0 ~ 1.0)

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("실습 21");

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

	InitBuffers();
	LoadCubeModel();
	LoadSphereModel(); // 구 모델 로드 추가

	glutDisplayFunc(DrawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, Timer, 0);

	glutMainLoop();
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
	// 벽면 버퍼 초기화
	glGenVertexArrays(1, &wallVAO);
	glBindVertexArray(wallVAO);

	glGenBuffers(1, &wallVBO);
	glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// 육면체 버퍼 초기화
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// 그리드 버퍼 초기화
	glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);

	glGenBuffers(1, &gridVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// 중력 벡터 버퍼 초기화 (추가)
	glGenVertexArrays(1, &gravityVAO);
	glBindVertexArray(gravityVAO);

	glGenBuffers(1, &gravityVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gravityVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// 구 버퍼 초기화 (추가)
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);

	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void DrawWalls() {
	// 카메라 위치 계산 (회전된 좌표계에서)
	glm::vec3 adjustedCameraPos = cameraPos + glm::vec3(0.0f, 0.0f, cameraOffsetZ);

	// 벽 회전 행렬 계산
	glm::mat4 rotationMatrix = glm::mat4(1.0f);
	rotationMatrix = glm::rotate(rotationMatrix, glm::radians(wallRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationMatrix = glm::rotate(rotationMatrix, glm::radians(wallRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 invRotationMatrix = glm::inverse(rotationMatrix);

	// 카메라 위치를 벽의 좌표계로 변환
	glm::vec4 localCameraPos = invRotationMatrix * glm::vec4(adjustedCameraPos, 1.0f);
	glm::vec3 cameraLocalPos = glm::vec3(localCameraPos);

	// 각 면의 중심점까지의 거리 계산
	struct WallInfo {
		std::string name;
		glm::vec3 center;
		float distance;
		std::vector<GLfloat> data;
	};

	std::vector<WallInfo> walls;

	// 왼쪽 벽 (빨간색) - X = -ROOM_SIZE
	{
		WallInfo wall;
		wall.name = "left";
		wall.center = glm::vec3(-ROOM_SIZE, (FLOOR_Y + ROOM_SIZE) * 0.5f, 0.0f);
		wall.distance = glm::distance(cameraLocalPos, wall.center);
		wall.data = {
			-ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 1.0f, 0.0f, 0.0f,
			-ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 1.0f, 0.0f, 0.0f,
			-ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 0.0f, 0.0f,
			-ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 0.0f, 0.0f,
			-ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 1.0f, 0.0f, 0.0f,
			-ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 1.0f, 0.0f, 0.0f
		};
		walls.push_back(wall);
	}

	// 오른쪽 벽 (초록색) - X = ROOM_SIZE
	{
		WallInfo wall;
		wall.name = "right";
		wall.center = glm::vec3(ROOM_SIZE, (FLOOR_Y + ROOM_SIZE) * 0.5f, 0.0f);
		wall.distance = glm::distance(cameraLocalPos, wall.center);
		wall.data = {
			ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 1.0f, 0.0f,
			ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 1.0f, 0.0f,
			ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 0.0f, 1.0f, 0.0f,
			ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 0.0f, 1.0f, 0.0f,
			ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.0f, 1.0f, 0.0f,
			ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 1.0f, 0.0f
		};
		walls.push_back(wall);
	}

	// 뒷벽 (파란색) - Z = -ROOM_SIZE
	{
		WallInfo wall;
		wall.name = "back";
		wall.center = glm::vec3(0.0f, (FLOOR_Y + ROOM_SIZE) * 0.5f, -ROOM_SIZE);
		wall.distance = glm::distance(cameraLocalPos, wall.center);
		wall.data = {
			-ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,
			ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,
			ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,
			ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,
			-ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 0.0f, 0.0f, 1.0f,
			-ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.0f, 0.0f, 1.0f
		};
		walls.push_back(wall);
	}

	// 위쪽 벽 (노란색) - Y = ROOM_SIZE
	{
		WallInfo wall;
		wall.name = "top";
		wall.center = glm::vec3(0.0f, ROOM_SIZE, 0.0f);
		wall.distance = glm::distance(cameraLocalPos, wall.center);
		wall.data = {
			-ROOM_SIZE, ROOM_SIZE, -ROOM_SIZE, 1.0f, 1.0f, 0.0f,
			-ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 1.0f, 0.0f,
			ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 1.0f, 0.0f,
			ROOM_SIZE,  ROOM_SIZE,  ROOM_SIZE, 1.0f, 1.0f, 0.0f,
			ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 1.0f, 1.0f, 0.0f,
			-ROOM_SIZE,  ROOM_SIZE, -ROOM_SIZE, 1.0f, 1.0f, 0.0f
		};
		walls.push_back(wall);
	}

	// 앞면 (보라색) - Z = ROOM_SIZE
	{
		WallInfo wall;
		wall.name = "front";
		wall.center = glm::vec3(0.0f, (FLOOR_Y + ROOM_SIZE) * 0.5f, ROOM_SIZE);
		wall.distance = glm::distance(cameraLocalPos, wall.center);
		wall.data = {
			-ROOM_SIZE, FLOOR_Y, ROOM_SIZE, 1.0f, 0.0f, 1.0f,
			-ROOM_SIZE,  ROOM_SIZE, ROOM_SIZE, 1.0f, 0.0f, 1.0f,
			ROOM_SIZE,  ROOM_SIZE, ROOM_SIZE, 1.0f, 0.0f, 1.0f,
			ROOM_SIZE,  ROOM_SIZE, ROOM_SIZE, 1.0f, 0.0f, 1.0f,
			ROOM_SIZE, FLOOR_Y, ROOM_SIZE, 1.0f, 0.0f, 1.0f,
			-ROOM_SIZE, FLOOR_Y, ROOM_SIZE, 1.0f, 0.0f, 1.0f
		};
		walls.push_back(wall);
	}

	// 바닥 (회색) - Y = FLOOR_Y
	if (!floorOpen) {
		WallInfo wall;
		wall.name = "bottom";
		wall.center = glm::vec3(0.0f, FLOOR_Y, 0.0f);
		wall.distance = glm::distance(cameraLocalPos, wall.center);
		wall.data = {
			-ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.5f, 0.5f, 0.5f,
			-ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.5f, 0.5f, 0.5f,
			ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.5f, 0.5f, 0.5f,
			ROOM_SIZE, FLOOR_Y,  ROOM_SIZE, 0.5f, 0.5f, 0.5f,
			ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.5f, 0.5f, 0.5f,
			-ROOM_SIZE, FLOOR_Y, -ROOM_SIZE, 0.5f, 0.5f, 0.5f
		};
		walls.push_back(wall);
	}

	// 가장 가까운 면 찾기
	float minDistance = std::numeric_limits<float>::max();
	int closestWallIndex = -1;
	for (size_t i = 0; i < walls.size(); i++) {
		if (walls[i].distance < minDistance) {
			minDistance = walls[i].distance;
			closestWallIndex = static_cast<int>(i);
		}
	}

	// 가장 가까운 면을 제외한 나머지 면들만 그리기
	std::vector<GLfloat> wallData;
	for (size_t i = 0; i < walls.size(); i++) {
		if (static_cast<int>(i) != closestWallIndex) {  // 가장 가까운 면은 제외
			for (const auto& vertex : walls[i].data) {
				wallData.push_back(vertex);
			}
		}
	}

	// 디버그 출력 (옵션)
	if (closestWallIndex >= 0) {
		static std::string lastClosestWall = "";
		if (lastClosestWall != walls[closestWallIndex].name) {
			lastClosestWall = walls[closestWallIndex].name;
			std::cout << "가장 가까운 면 (컬링됨): " << lastClosestWall << " (거리: " << minDistance << ")" << std::endl;
		}
	}

	// 버퍼에 데이터 업로드
	glBindVertexArray(wallVAO);
	glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
	glBufferData(GL_ARRAY_BUFFER, wallData.size() * sizeof(GLfloat), wallData.data(), GL_DYNAMIC_DRAW);

	// 벽에 회전 변환 적용
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(rotationMatrix));

	// Face culling 비활성화 (수동으로 컬링했으므로)
	glDisable(GL_CULL_FACE);

	// 그리기
	glDrawArrays(GL_TRIANGLES, 0, wallData.size() / 6);

	glBindVertexArray(0);
}

void LoadCubeModel() {
	read_obj_file("cube.obj", &cubeModel);
	if (cubeModel.vertex_count == 0 || cubeModel.face_count == 0) {
		std::cerr << "ERROR: cube.obj 파일을 읽지 못했습니다." << std::endl;
	}
	else {
		std::cout << "cube.obj 로드 완료: " << cubeModel.vertex_count << " vertices, "
			<< cubeModel.face_count << " faces" << std::endl;
	}
}

void DrawCubes() {
	if (cubeModel.vertex_count == 0 || cubeModel.face_count == 0) {
		return; // 모델이 로드되지 않은 경우
	}

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

	for (const auto& cube : cubes) {
		// OBJ 모델 데이터를 기반으로 버텍스 데이터 생성
		std::vector<GLfloat> cubeData;

		for (size_t i = 0; i < cubeModel.face_count; i++) {
			// 각 면의 3개 정점 추가
			Face face = cubeModel.faces[i];

			// 첫 번째 정점
			Vertex v1 = cubeModel.vertices[face.v1];
			cubeData.push_back(v1.x - 0.5f); // 중심을 원점으로 이동
			cubeData.push_back(v1.y - 0.5f);
			cubeData.push_back(v1.z - 0.5f);
			cubeData.push_back(cube.color.r);
			cubeData.push_back(cube.color.g);
			cubeData.push_back(cube.color.b);

			// 두 번째 정점
			Vertex v2 = cubeModel.vertices[face.v2];
			cubeData.push_back(v2.x - 0.5f);
			cubeData.push_back(v2.y - 0.5f);
			cubeData.push_back(v2.z - 0.5f);
			cubeData.push_back(cube.color.r);
			cubeData.push_back(cube.color.g);
			cubeData.push_back(cube.color.b);

			// 세 번째 정점
			Vertex v3 = cubeModel.vertices[face.v3];
			cubeData.push_back(v3.x - 0.5f);
			cubeData.push_back(v3.y - 0.5f);
			cubeData.push_back(v3.z - 0.5f);
			cubeData.push_back(cube.color.r);
			cubeData.push_back(cube.color.g);
			cubeData.push_back(cube.color.b);
		}

		glBufferData(GL_ARRAY_BUFFER, cubeData.size() * sizeof(GLfloat), cubeData.data(), GL_DYNAMIC_DRAW);

		// 모델 변환 행렬 설정 - 벽의 회전과 함께 회전
		glm::mat4 modelMat = glm::mat4(1.0f);

		// 위치 이동을 먼저 적용
		modelMat = glm::translate(modelMat, cube.position);

		// 모든 큐브가 벽의 회전과 함께 회전하도록 변경
		// 벽의 회전과 동일하게 X축과 Z축 기준으로 회전
		modelMat = glm::rotate(modelMat, glm::radians(wallRotationX), glm::vec3(1.0f, 0.0f, 0.0f));  // X축 기준 회전 추가
		modelMat = glm::rotate(modelMat, glm::radians(wallRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));  // Z축 기준 회전

		// 크기 조절을 마지막에 적용
		modelMat = glm::scale(modelMat, cube.scale);

		GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

		glDrawArrays(GL_TRIANGLES, 0, cubeModel.face_count * 3);
	}

	glBindVertexArray(0);
}

void DrawScene() {
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	// 카메라 설정
	glm::vec3 adjustedCameraPos = cameraPos + glm::vec3(0.0f, 0.0f, cameraOffsetZ);

	glm::mat4 view = glm::mat4(1.0f);
	view = glm::rotate(view, glm::radians(cameraRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::rotate(view, glm::radians(rotateY), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::lookAt(adjustedCameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// 그리드 그리기 (가장 먼저)
	DrawGrid();

	// 중력 벡터 그리기 (추가)
	DrawGravityVector();

	// 벽 그리기
	DrawWalls();

	// 육면체 그리기
	DrawCubes();

	// 구 그리기 (추가)
	DrawSpheres();

	glutSwapBuffers();
}

void Reshape(int w, int h) {
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			isMousePressed = true;
			mouseX = x;
			mouseY = y;
		}
		else if (state == GLUT_UP) {
			isMousePressed = false;
		}
	}
}

void Motion(int x, int y) {
	if (isMousePressed) {
		int deltaX = x - mouseX;
		int deltaY = y - mouseY;

		// 전체 박스 회전 (벽면 회전)
		wallRotationZ += deltaX * 0.5f;  // 마우스 X 이동으로 Z축 회전
		wallRotationX += deltaY * 0.5f;  // 마우스 Y 이동으로 X축 회전 (추가)

		mouseX = x;
		mouseY = y;

		glutPostRedisplay();
	}
}

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'z':
		cameraOffsetZ += 0.5f;
		break;
	case 'Z':
		cameraOffsetZ -= 0.5f;
		break;
	case 'y':
		wallRotationZ += 5.0f;
		break;
	case 'Y':
		wallRotationZ -= 5.0f;
		break;
	case 'x':  // 추가: X축 회전 제어
		wallRotationX += 5.0f;
		break;
	case 'X':  // 추가: X축 회전 제어
		wallRotationX -= 5.0f;
		break;
	case 'g':  // 추가: 중력 벡터 표시 토글
	case 'G':
		showGravityVector = !showGravityVector;
		break;
	case 'b':  // 추가: 구 생성
	case 'B':
		CreateSphere();
		break;
	case ' ':
		floorOpen = !floorOpen;
		// 바닥이 다시 생겼을 때 큐브들을 초기 위치로 복원
		if (!floorOpen) {
			for (size_t i = 0; i < cubes.size() && i < initialCubePositions.size(); ++i) {
				cubes[i].position = initialCubePositions[i];
				cubes[i].velocity = glm::vec3(0.0f);  // 속도도 초기화
				cubes[i].onGround = false;
			}
		}
		break;
	case 'q':
		exit(0);
		break;
	}

	glutPostRedisplay();
}

void Timer(int value) {
	float deltaTime = 1.0f / 60.0f;

	// 물리 업데이트
	UpdatePhysics(deltaTime);
	UpdateSpherePhysics(deltaTime); // 구 물리 업데이트 추가

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}


void DrawGrid() {
	std::vector<GLfloat> gridData;
	const float axisLength = 10.0f;    // 축의 길이

	// X축 (빨간색) - 원점에서 x 방향으로
	gridData.insert(gridData.end(), {
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // 원점
	 axisLength, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f   // x 방향 끝점
		});

	// Y축 (초록색) - 원점에서 y 방향으로
	gridData.insert(gridData.end(), {
	 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 원점
		0.0f, axisLength, 0.0f, 0.0f, 1.0f, 0.0f   // y 방향 끝점
		});

	// Z축 (파란색) - 원점에서 z 방향으로
	gridData.insert(gridData.end(), {
	 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 원점
		  0.0f, 0.0f, axisLength, 0.0f, 0.0f, 1.0f   // z 방향 끝점
		});

	glBindVertexArray(gridVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, gridData.size() * sizeof(GLfloat), gridData.data(), GL_DYNAMIC_DRAW);

	// 그리드는 회전하지 않음 (항상 고정)
	glm::mat4 modelMat = glm::mat4(1.0f);
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	// 선으로 그리기
	//glDrawArrays(GL_LINES, 0, gridData.size() / 6);

	glBindVertexArray(0);
}

void DrawGravityVector() {
	if (!showGravityVector) return;

	// 중력은 항상 월드 좌표계의 -Y 방향 (벽면과 수직, xz평면과 평행)
	glm::vec3 gravityDirection = glm::vec3(0.0f, -1.0f, 0.0f);
	float arrowLength = 3.0f;  // 화살표 길이
	glm::vec3 arrowEnd = gravityDirection * arrowLength;

	// 화살표 시작점을 원점으로 설정
	glm::vec3 arrowStart = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 arrowEndPos = arrowStart + arrowEnd;

	// 화살촉을 위한 보조 벡터 계산
	glm::vec3 perpendicular1 = glm::vec3(0.3f, 0.0f, 0.0f);  // X축 방향
	glm::vec3 perpendicular2 = glm::vec3(0.0f, 0.0f, 0.3f);  // Z축 방향

	// 화살촉 시작점 (화살표 끝에서 조금 뒤)
	glm::vec3 arrowHeadBase = arrowEndPos - gravityDirection * 0.5f;

	std::vector<GLfloat> gravityData;

	// 메인 화살표 선 (빨간색)
	gravityData.insert(gravityData.end(), {
		arrowStart.x, arrowStart.y, arrowStart.z, 1.0f, 0.0f, 0.0f,  // 시작점 (원점)
		arrowEndPos.x, arrowEndPos.y, arrowEndPos.z, 1.0f, 0.0f, 0.0f   // 끝점
		});

	// 화살촉 라인 1
	gravityData.insert(gravityData.end(), {
		arrowEndPos.x, arrowEndPos.y, arrowEndPos.z, 1.0f, 0.0f, 0.0f,  // 화살표 끝
		arrowHeadBase.x + perpendicular1.x, arrowHeadBase.y + perpendicular1.y, arrowHeadBase.z + perpendicular1.z, 1.0f, 0.0f, 0.0f
		});

	// 화살촉 라인 2
	gravityData.insert(gravityData.end(), {
		arrowEndPos.x, arrowEndPos.y, arrowEndPos.z, 1.0f, 0.0f, 0.0f,  // 화살표 끝
		arrowHeadBase.x - perpendicular1.x, arrowHeadBase.y - perpendicular1.y, arrowHeadBase.z - perpendicular1.z, 1.0f, 0.0f, 0.0f
		});

	// 화살촉 라인 3
	gravityData.insert(gravityData.end(), {
		arrowEndPos.x, arrowEndPos.y, arrowEndPos.z, 1.0f, 0.0f, 0.0f,  // 화살표 끝
		arrowHeadBase.x + perpendicular2.x, arrowHeadBase.y + perpendicular2.y, arrowHeadBase.z + perpendicular2.z, 1.0f, 0.0f, 0.0f
		});

	// 화살촉 라인 4
	gravityData.insert(gravityData.end(), {
		arrowEndPos.x, arrowEndPos.y, arrowEndPos.z, 1.0f, 0.0f, 0.0f,  // 화살표 끝
		arrowHeadBase.x - perpendicular2.x, arrowHeadBase.y - perpendicular2.y, arrowHeadBase.z - perpendicular2.z, 1.0f, 0.0f, 0.0f
		});

	glBindVertexArray(gravityVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gravityVBO);
	glBufferData(GL_ARRAY_BUFFER, gravityData.size() * sizeof(GLfloat), gravityData.data(), GL_DYNAMIC_DRAW);

	// 중력 벡터는 회전하지 않음 (항상 고정)
	glm::mat4 modelMat = glm::mat4(1.0f);
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

	// 선 두께 설정
	glLineWidth(3.0f);

	// 선으로 그리기
	//glDrawArrays(GL_LINES, 0, gravityData.size() / 6);

	// 선 두께 복원
	glLineWidth(1.0f);

	glBindVertexArray(0);
}

void LoadSphereModel() {
	read_obj_file("sphere.obj", &sphereModel);
	if (sphereModel.vertex_count == 0 || sphereModel.face_count == 0) {
		std::cerr << "ERROR: sphere.obj 파일을 읽지 못했습니다." << std::endl;
	}
	else {
		std::cout << "sphere.obj 로드 완료: " << sphereModel.vertex_count << " vertices, "
			<< sphereModel.face_count << " faces" << std::endl;
	}
}

void DrawSpheres() {
	if (sphereModel.vertex_count == 0 || sphereModel.face_count == 0) {
		return; // 모델이 로드되지 않은 경우
	}

	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);

	for (const auto& sphere : spheres) {
		if (!sphere.active) continue; // 비활성 구는 그리지 않음

		// OBJ 모델 데이터를 기반으로 버텍스 데이터 생성
		std::vector<GLfloat> sphereData;

		for (size_t i = 0; i < sphereModel.face_count; i++) {
			// 각 면의 3개 정점 추가
			Face face = sphereModel.faces[i];

			// 첫 번째 정점
			Vertex v1 = sphereModel.vertices[face.v1];
			sphereData.push_back(v1.x * sphere.radius); // 반지름 적용
			sphereData.push_back(v1.y * sphere.radius);
			sphereData.push_back(v1.z * sphere.radius);
			sphereData.push_back(sphere.color.r);
			sphereData.push_back(sphere.color.g);
			sphereData.push_back(sphere.color.b);

			// 두 번째 정점
			Vertex v2 = sphereModel.vertices[face.v2];
			sphereData.push_back(v2.x * sphere.radius);
			sphereData.push_back(v2.y * sphere.radius);
			sphereData.push_back(v2.z * sphere.radius);
			sphereData.push_back(sphere.color.r);
			sphereData.push_back(sphere.color.g);
			sphereData.push_back(sphere.color.b);

			// 세 번째 정점
			Vertex v3 = sphereModel.vertices[face.v3];
			sphereData.push_back(v3.x * sphere.radius);
			sphereData.push_back(v3.y * sphere.radius);
			sphereData.push_back(v3.z * sphere.radius);
			sphereData.push_back(sphere.color.r);
			sphereData.push_back(sphere.color.g);
			sphereData.push_back(sphere.color.b);
		}

		glBufferData(GL_ARRAY_BUFFER, sphereData.size() * sizeof(GLfloat), sphereData.data(), GL_DYNAMIC_DRAW);

		// 모델 변환 행렬 설정 - 구는 벽의 회전에 영향받지 않음
		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat = glm::translate(modelMat, sphere.position);
		modelMat = glm::scale(modelMat, glm::vec3(3.0f)); // 구는 크기 조절 없음

		GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

		glDrawArrays(GL_TRIANGLES, 0, sphereModel.face_count * 3);
	}

	glBindVertexArray(0);
}

void CreateSphere() {
	// 최대 개수 체크
	if (spheres.size() >= MAX_SPHERES) {
		return; // 최대 5개까지만 생성
	}

	// 새로운 구 생성
	Sphere newSphere;
	newSphere.position = glm::vec3(0.0f, 0.0f, 0.0f); // 원점에서 시작
	newSphere.radius = 0.9f; // 반지름을 300% 확대 (0.3f * 3 = 0.9f)

	// 랜덤한 색상 생성
	newSphere.color = glm::vec3(
		static_cast<float>(rand()) / RAND_MAX,
		static_cast<float>(rand()) / RAND_MAX,
		static_cast<float>(rand()) / RAND_MAX
	);

	// 랜덤한 방향으로 속도 설정 (중력 영향 없음)
	float speed = 3.0f; // 고정 속도
	float angleXY = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI; // XY 평면 각도
	float angleZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * M_PI; // Z 방향 각도

	// 정규화된 방향 벡터 생성
	glm::vec3 direction = glm::vec3(
		cos(angleXY) * cos(angleZ),
		sin(angleXY) * cos(angleZ),
		sin(angleZ)
	);

	// 방향 벡터를 정규화하여 일정한 속도 보장
	direction = glm::normalize(direction);
	newSphere.velocity = direction * speed;

	newSphere.active = true;

	spheres.push_back(newSphere);
}

void UpdatePhysics(float deltaTime) {
	for (auto& cube : cubes) {
		// 중력은 항상 월드 좌표계의 -Y 방향으로만 작용 (벽면과 수직, xz평면과 평행)
		glm::vec3 gravity = glm::vec3(0.0f, -GRAVITY, 0.0f);

		// 중력 적용
		cube.velocity += gravity * deltaTime;

		// 위치 업데이트
		cube.position.x += cube.velocity.x * deltaTime;
		cube.position.y += cube.velocity.y * deltaTime;
		cube.position.z += cube.velocity.z * deltaTime; // Z축 이동도 허용

		// 벽 회전에 따른 변환 행렬 계산 (벽은 전체 회전 적용)
		glm::mat4 rotationMatrix = glm::mat4(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(wallRotationX), glm::vec3(1.0f, 0.0f, 0.0f));  // X축 기준 회전 추가
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(wallRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));  // Z축 기준 회전
		glm::mat4 invRotationMatrix = glm::inverse(rotationMatrix);

		// 큐브 위치를 벽의 회전된 좌표계로 변환
		glm::vec4 localPos = invRotationMatrix * glm::vec4(cube.position, 1.0f);
		glm::vec3 cubeLocalPos = glm::vec3(localPos);

		// 큐브의 확장된 경계 계산
		glm::vec3 cubeExtents = cube.scale * 0.5f;

		// 회전된 좌표계에서의 충돌 검사
		bool collisionOccurred = false;
		glm::vec3 correctedLocalPos = cubeLocalPos;

		// 바닥 충돌 검사 (회전된 좌표계에서)
		float cubeBottomY = correctedLocalPos.y - cubeExtents.y;
		if (cubeBottomY <= FLOOR_Y && !floorOpen) {
			correctedLocalPos.y = FLOOR_Y + cubeExtents.y;

			// 바닥에 닿으면 회전된 좌표계에서의 y축 속도를 0으로 설정
			glm::vec4 localVel = invRotationMatrix * glm::vec4(cube.velocity, 0.0f);
			if (localVel.y < 0) {
				localVel.y = 0.0f;
				// 다시 월드 좌표계로 변환
				glm::vec4 worldVel = rotationMatrix * localVel;
				cube.velocity = glm::vec3(worldVel);
			}
			cube.onGround = true;
			collisionOccurred = true;
		}
		else if (floorOpen && cube.position.y < FLOOR_Y - 10.0f) {
			// 바닥이 열려있고 너무 아래로 떨어지면 위에서 다시 시작
			cube.position.y = ROOM_SIZE + 2.0f;
			cube.velocity = glm::vec3(0.0f);
			cube.onGround = false;
		}
		else {
			cube.onGround = false;
		}

		// 왼쪽 벽 충돌 (회전된 좌표계에서) - X = -ROOM_SIZE
		if (correctedLocalPos.x - cubeExtents.x <= -ROOM_SIZE) {
			correctedLocalPos.x = -ROOM_SIZE + cubeExtents.x;
			// 회전된 좌표계에서 x축 속도 반사
			glm::vec4 localVel = invRotationMatrix * glm::vec4(cube.velocity, 0.0f);
			localVel.x = -localVel.x * BOUNCE_DAMPING; // 반사와 감쇠 적용
			glm::vec4 worldVel = rotationMatrix * localVel;
			cube.velocity = glm::vec3(worldVel);
			collisionOccurred = true;
		}

		// 오른쪽 벽 충돌 (회전된 좌표계에서) - X = ROOM_SIZE
		if (correctedLocalPos.x + cubeExtents.x >= ROOM_SIZE) {
			correctedLocalPos.x = ROOM_SIZE - cubeExtents.x;
			// 회전된 좌표계에서 x축 속도 반사
			glm::vec4 localVel = invRotationMatrix * glm::vec4(cube.velocity, 0.0f);
			localVel.x = -localVel.x * BOUNCE_DAMPING; // 반사와 감쇠 적용
			glm::vec4 worldVel = rotationMatrix * localVel;
			cube.velocity = glm::vec3(worldVel);
			collisionOccurred = true;
		}

		// 뒷벽 충돌 (회전된 좌표계에서) - Z = -ROOM_SIZE
		if (correctedLocalPos.z - cubeExtents.z <= -ROOM_SIZE) {
			correctedLocalPos.z = -ROOM_SIZE + cubeExtents.z;
			// 회전된 좌표계에서 z축 속도 반사
			glm::vec4 localVel = invRotationMatrix * glm::vec4(cube.velocity, 0.0f);
			localVel.z = -localVel.z * BOUNCE_DAMPING; // 반사와 감쇠 적용
			glm::vec4 worldVel = rotationMatrix * localVel;
			cube.velocity = glm::vec3(worldVel);
			collisionOccurred = true;
		}

		// 앞벽 충돌 (회전된 좌표계에서) - Z = ROOM_SIZE (컬링된 벽도 충돌 처리)
		if (correctedLocalPos.z + cubeExtents.z >= ROOM_SIZE) {
			correctedLocalPos.z = ROOM_SIZE - cubeExtents.z;
			// 회전된 좌표계에서 z축 속도 반사
			glm::vec4 localVel = invRotationMatrix * glm::vec4(cube.velocity, 0.0f);
			localVel.z = -localVel.z * BOUNCE_DAMPING; // 반사와 감쇠 적용
			glm::vec4 worldVel = rotationMatrix * localVel;
			cube.velocity = glm::vec3(worldVel);
			collisionOccurred = true;
		}

		// 천장 충돌 검사 (회전된 좌표계에서) - Y = ROOM_SIZE
		float cubeTopY = correctedLocalPos.y + cubeExtents.y;
		if (cubeTopY >= ROOM_SIZE) {
			correctedLocalPos.y = ROOM_SIZE - cubeExtents.y;
			// 회전된 좌표계에서 y축 속도 반사
			glm::vec4 localVel = invRotationMatrix * glm::vec4(cube.velocity, 0.0f);
			if (localVel.y > 0) {
				localVel.y = -localVel.y * BOUNCE_DAMPING; // 반사와 감쇠 적용
				glm::vec4 worldVel = rotationMatrix * localVel;
				cube.velocity = glm::vec3(worldVel);
			}
			collisionOccurred = true;
		}

		// 수정된 위치를 월드 좌표계로 다시 변환
		if (collisionOccurred) {
			glm::vec4 worldPos = rotationMatrix * glm::vec4(correctedLocalPos, 1.0f);
			cube.position = glm::vec3(worldPos);
		}
	}
}

void UpdateSpherePhysics(float deltaTime) {
	for (auto& sphere : spheres) {
		if (!sphere.active) continue;

		// 속도 크기를 일정하게 유지
		float originalSpeed = glm::length(sphere.velocity);
		if (originalSpeed == 0.0f) originalSpeed = 3.0f; // 기본 속도 설정

		// 위치 업데이트 (중력 영향 없음)
		sphere.position += sphere.velocity * deltaTime;

		// 벽과의 충돌 검사 및 반사
		bool collisionOccurred = false;

		// 벽 회전에 따른 변환 행렬 계산
		glm::mat4 rotationMatrix = glm::mat4(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(wallRotationX), glm::vec3(1.0f, 0.0f, 0.0f));  // X축 기준 회전 추가
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(wallRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));  // Z축 기준 회전
		glm::mat4 invRotationMatrix = glm::inverse(rotationMatrix);

		// 구 위치를 벽의 회전된 좌표계로 변환
		glm::vec4 localPos = invRotationMatrix * glm::vec4(sphere.position, 1.0f);
		glm::vec3 sphereLocalPos = glm::vec3(localPos);

		// 구 속도를 회전된 좌표계로 변환
		glm::vec4 localVel = invRotationMatrix * glm::vec4(sphere.velocity, 0.0f);
		glm::vec3 sphereLocalVel = glm::vec3(localVel);

		// 왼쪽 벽 충돌 (회전된 좌표계에서) - X = -ROOM_SIZE
		if (sphereLocalPos.x - sphere.radius <= -ROOM_SIZE) {
			sphereLocalPos.x = -ROOM_SIZE + sphere.radius;
			sphereLocalVel.x = -sphereLocalVel.x; // X축 속도 반사
			collisionOccurred = true;
		}

		// 오른쪽 벽 충돌 (회전된 좌표계에서) - X = ROOM_SIZE
		if (sphereLocalPos.x + sphere.radius >= ROOM_SIZE) {
			sphereLocalPos.x = ROOM_SIZE - sphere.radius;
			sphereLocalVel.x = -sphereLocalVel.x; // X축 속도 반사
			collisionOccurred = true;
		}

		// 바닥 충돌 (회전된 좌표계에서) - Y = FLOOR_Y
		if (sphereLocalPos.y - sphere.radius <= FLOOR_Y && !floorOpen) {
			sphereLocalPos.y = FLOOR_Y + sphere.radius;
			sphereLocalVel.y = -sphereLocalVel.y; // Y축 속도 반사
			collisionOccurred = true;
		}

		// 천장 충돌 (회전된 좌표계에서) - Y = ROOM_SIZE
		if (sphereLocalPos.y + sphere.radius >= ROOM_SIZE) {
			sphereLocalPos.y = ROOM_SIZE - sphere.radius;
			sphereLocalVel.y = -sphereLocalVel.y; // Y축 속도 반사
			collisionOccurred = true;
		}

		// 뒷벽 충돌 (회전된 좌표계에서) - Z = -ROOM_SIZE
		if (sphereLocalPos.z - sphere.radius <= -ROOM_SIZE) {
			sphereLocalPos.z = -ROOM_SIZE + sphere.radius;
			sphereLocalVel.z = -sphereLocalVel.z; // Z축 속도 반사
			collisionOccurred = true;
		}

		// 앞벽 충돌 (회전된 좌표계에서) - Z = ROOM_SIZE (컬링된 벽도 충돌 처리)
		if (sphereLocalPos.z + sphere.radius >= ROOM_SIZE) {
			sphereLocalPos.z = ROOM_SIZE - sphere.radius;
			sphereLocalVel.z = -sphereLocalVel.z; // Z축 속도 반사
			collisionOccurred = true;
		}

		// 바닥이 열려있고 구가 떨어진 경우
		if (floorOpen && sphereLocalPos.y < FLOOR_Y - 10.0f) {
			// 구를 위에서 다시 시작
			sphereLocalPos.y = ROOM_SIZE + 1.0f;
			sphereLocalPos.x = 0.0f;
			sphereLocalPos.z = 0.0f;

			// 새로운 랜덤 방향으로 속도 재설정 (속도 크기는 유지)
			float angleXY = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
			float angleZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * M_PI;

			glm::vec3 direction = glm::vec3(
				cos(angleXY) * cos(angleZ),
				sin(angleXY) * cos(angleZ),
				sin(angleZ)
			);
			direction = glm::normalize(direction);
			sphereLocalVel = direction * originalSpeed;
			collisionOccurred = true;
		}

		// 수정된 위치와 속도를 월드 좌표계로 다시 변환
		if (collisionOccurred || (floorOpen && sphere.position.y < FLOOR_Y - 10.0f)) {
			glm::vec4 worldPos = rotationMatrix * glm::vec4(sphereLocalPos, 1.0f);
			glm::vec4 worldVel = rotationMatrix * glm::vec4(sphereLocalVel, 0.0f);

			sphere.position = glm::vec3(worldPos);
			sphere.velocity = glm::vec3(worldVel);
		}

		// 속도 크기를 일정하게 유지 (충돌 후에도)
		if (collisionOccurred) {
			float currentSpeed = glm::length(sphere.velocity);
			if (currentSpeed > 0.0f) {
				sphere.velocity = glm::normalize(sphere.velocity) * originalSpeed;
			}
		}
	}
}