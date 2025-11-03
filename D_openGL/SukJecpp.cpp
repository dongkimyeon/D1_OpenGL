#include "CommonInclude.h"

std::uniform_real_distribution <float> randomSpeed(0.2f, 1.5f);
std::uniform_real_distribution <float> randomMaxHeight(4.0f, 6.0f);
std::uniform_real_distribution <float> randomMinHeight(1.0f, 2.0f);
std::uniform_int_distribution <int> randomInt(0, 1);
std::random_device rd;

//구조체
struct ExtendedModel {
    Model baseModel;
    glm::vec3 center;
    GLuint indexCount = 0;
    std::vector<GLuint> indices;
    float minHeight = randomMinHeight(rd);
    float MaxHeight = randomMaxHeight(rd);
    float height = minHeight;
    float speed = randomSpeed(rd);

};

void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
glm::vec3 CalculateModelCenter(Model* model);
void InitBuffers();
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);
void Mouse(int button, int state, int x, int y);
void SpecialKeyboard(int key, int x, int y);
void Motion(int x, int y);



GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
//선형보간 속도
float interpSpeed = 10.0f;

// 미로 VAO, VBO, EBO
GLuint MazeVAO, MazeVBO, MazeEBO;
// 플레이어 VAO, VBO, EBO
GLuint PlayerVAO, PlayerVBO, PlayerEBO;
int n = 0; // 큐브 개수 
int m = 0;

// n x m 큐브 모델 배열
ExtendedModel** CubeModels;
ExtendedModel Player;

//초기 시작 애니메이션 관련 변수 
bool enterAnimation = true;
float yMoveValue = -5.0f;

//카메라 관련 변수 
float cameraAngle = glm::atan(1.0f);
float targetCameraAngle = glm::atan(1.0f);

glm::vec3 cameraPos = glm::vec3(0.0f, 23.5f, 23.5f);
glm::vec3 centerPoint = glm::vec3(0.0f, 0.0f, 0.0f);

//명령어 변수
bool mFlag = false;
bool oFlag = true;

void CubeSpeedUp() {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            CubeModels[i][j].speed += 0.1f;
        }
    }
}
void CubeSpeedDown() {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (CubeModels[i][j].speed > 0.1f) {
                CubeModels[i][j].speed -= 0.1f;
            }

        }
    }
}
void InitBuffers() {
    const GLsizei stride = 6 * sizeof(GLfloat);  // 위치(3) + 색상(3)

    glGenVertexArrays(1, &MazeVAO);
    glBindVertexArray(MazeVAO);
    glGenBuffers(1, &MazeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, MazeVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &MazeEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MazeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenVertexArrays(1, &PlayerVAO);
    glBindVertexArray(PlayerVAO);
    glGenBuffers(1, &PlayerVBO);
    glBindBuffer(GL_ARRAY_BUFFER, PlayerVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &PlayerEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PlayerEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

}

void main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    width = 800; // 적절한 값으로 초기화     
    height = 600;
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("3D Maze");

    glewExperimental = GL_TRUE;
    glewInit();

    MakeVertexShaders(); // 버텍스 셰이더 생성
    MakeFragmentShaders(); // 프래그먼트 셰이더 생성
    shaderProgramID = MakeShaderProgram(); // 셰이더 프로그램 생성

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard);
    glutTimerFunc(16, Timer, 0);
    glutMouseFunc(Mouse);

    std::cin >> n >> m;
    //n x m 큐브 모델 동적 할당
    CubeModels = new ExtendedModel * [n];
    for (int i = 0; i < n; ++i) {
        CubeModels[i] = new ExtendedModel[m];
        for (int j = 0; j < m; ++j) {
            read_obj_file("cube.obj", &CubeModels[i][j].baseModel);
            CubeModels[i][j].center = CalculateModelCenter(&CubeModels[i][j].baseModel);
            // 수정: speed를 float으로 랜덤 설정 (int 캐스팅 및 스케일링 제거)
            CubeModels[i][j].speed = randomSpeed(rd);
            // minHeight와 MaxHeight는 구조체 초기화에서 이미 랜덤 설정됨
        }
    }

	read_obj_file("sphere.obj", &Player.baseModel);
	Player.center = CalculateModelCenter(&Player.baseModel);
	Player.speed = 1.0f;
    InitBuffers(); // 버퍼 초기화
    glutMainLoop();
}

void MakeVertexShaders()
{
    GLchar* vertexSource;
    //--- 버텍스 세이더 읽어 저장하고 컴파일 하기
    //--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
    vertexSource = filetobuf("vertex3D.glsl");
    if (vertexSource == NULL) {
        std::cerr << "ERROR: vertex.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

void MakeFragmentShaders()
{
    GLchar* fragmentSource;
    //--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
    fragmentSource = filetobuf("fragment.glsl"); // 프래그세이더 읽어오기
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

GLuint MakeShaderProgram()
{
    GLuint shaderID;

    GLint result;
    GLchar errorLog[512];

    shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
    glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
    glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
    glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기
    glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
    glDeleteShader(fragmentShader);
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기

    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        return false;  // 수정: GLuint 반환 타입이지만 false 반환 시 0으로 처리됨
    }

    glUseProgram(shaderID);
    return shaderID;
}

// 수정된 DrawScene: scissor test로 미니맵 클리어 제한
void DrawScene() {
    float r = glm::sqrt(cameraPos.x * cameraPos.x + cameraPos.z * cameraPos.z); // XZ plane distance
    cameraPos.x = r * glm::cos(cameraAngle);
    cameraPos.z = r * glm::sin(cameraAngle);


    glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
  
 
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    std::vector<GLfloat> vertexData; // x,y,z,r,g,b
    std::vector<GLuint> indices;

    GLuint vertexBase = 0;
    const float spacing = 1.0f;


    std::vector<int> cubeIndexCounts;
    GLuint currentIndexOffset = 0;  // EBO offset 추적 

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            Model& mdl = CubeModels[i][j].baseModel;
            glm::vec3 center = CubeModels[i][j].center;

            float r = (float)i / std::max(1, n - 1);
            float g = (float)j / std::max(1, m - 1);
            float b = 0.5f;

            //초기 위치 설정 y값 
            glm::vec3 translation = glm::vec3((j - (m - 1) / 2.0f) * spacing, 0.0f, (i - (n - 1) / 2.0f) * spacing);

            // add vertices
            for (size_t v = 0; v < mdl.vertex_count; ++v) {
                Vertex vert = mdl.vertices[v];
                float x = vert.x - center.x + translation.x;
                float y = vert.y - center.y + translation.y;
                float z = vert.z - center.z + translation.z;
                vertexData.push_back(x);
                vertexData.push_back(y);
                vertexData.push_back(z);
                vertexData.push_back(r);
                vertexData.push_back(g);
                vertexData.push_back(b);
            }

            int thisCubeIndexCount = 0;  // 이 큐브의 index 수
            for (size_t f = 0; f < mdl.face_count; ++f) {
                // assume faces are zero-based indices
                indices.push_back(static_cast<GLuint>(mdl.faces[f].v1) + vertexBase);
                indices.push_back(static_cast<GLuint>(mdl.faces[f].v2) + vertexBase);
                indices.push_back(static_cast<GLuint>(mdl.faces[f].v3) + vertexBase);
                thisCubeIndexCount += 3;
            }
            cubeIndexCounts.push_back(thisCubeIndexCount);  // 저장

            vertexBase += static_cast<GLuint>(mdl.vertex_count);
        }
    }

    glBindVertexArray(MazeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, MazeVBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MazeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);
   


    GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
    GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

    // 1. 메인 뷰: 전체 화면 렌더링 (기존 카메라)
    glViewport(0, 0, width, height);  // 전체 뷰포트
    glm::mat4 mainView = glm::lookAt(cameraPos, centerPoint, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 mainProj;
    if (oFlag) {
        //원근
        mainProj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

    }
    else {
        //직교
        mainProj = glm::ortho(-10.0f, 10.0f, -10.0f * (float)height / (float)width, 10.0f * (float)height / (float)width, 0.1f, 100.0f);

    }
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mainView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(mainProj));

    int cubeIdx = 0;
    currentIndexOffset = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            glm::mat4 modelMat = glm::mat4(1.0f);
            if (enterAnimation) {
                modelMat = glm::translate(modelMat, glm::vec3(0.0f, yMoveValue, 0.0f));
            }
            modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.5f, 0.0f));
            modelMat = glm::scale(modelMat, glm::vec3(1.0f, CubeModels[i][j].height, 1.0f));
            modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.5f, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
            int thisCubeIndexCount = cubeIndexCounts[cubeIdx];
            glDrawElements(GL_TRIANGLES, thisCubeIndexCount, GL_UNSIGNED_INT,
                (void*)(currentIndexOffset * sizeof(GLuint)));

            currentIndexOffset += thisCubeIndexCount;
            cubeIdx++;
        }
    }

    // 미니맵
    int miniWidth = 200;   
    int miniHeight = 200; 
    int miniX = width - miniWidth;         // 오른쪽 시작
    int miniY = height - miniHeight;       // 위쪽 시작

    // 미니맵 배경 클리어: scissor test로 영역 제한 (메인 뷰 보호)
    glDisable(GL_DEPTH_TEST);  // 오버레이를 위해 깊이 테스트 끄기
    glEnable(GL_SCISSOR_TEST); // scissor 활성화
    glScissor(miniX, miniY, miniWidth, miniHeight);  // 클리어 영역 제한
    glViewport(miniX, miniY, miniWidth, miniHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
    glDisable(GL_SCISSOR_TEST);  // scissor 복원

    // 탑뷰 카메라/프로젝션
	
    glm::mat4 topView = glm::lookAt(glm::vec3(0.0f, (float)std::max(n, m) + 10.0f, 0.0f), centerPoint, glm::vec3(1.0f, 0.0f, 0.0f));
	topView = glm::rotate(topView, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 topProj = glm::perspective(glm::radians(45.0f), (float)miniWidth / (float)miniHeight, 0.1f, 1000.0f);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(topView));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(topProj));

    // 탑뷰 재렌더링 
    cubeIdx = 0;
    currentIndexOffset = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            glm::mat4 modelMat = glm::mat4(1.0f);
            if (enterAnimation) {
                modelMat = glm::translate(modelMat, glm::vec3(0.0f, yMoveValue, 0.0f));
            }
            modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.5f, 0.0f));
            modelMat = glm::scale(modelMat, glm::vec3(1.0f, CubeModels[i][j].height, 1.0f));
            modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.5f, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
            int thisCubeIndexCount = cubeIndexCounts[cubeIdx];
            glDrawElements(GL_TRIANGLES, thisCubeIndexCount, GL_UNSIGNED_INT,
                (void*)(currentIndexOffset * sizeof(GLuint)));

            currentIndexOffset += thisCubeIndexCount;
            cubeIdx++;
        }
    }

    glEnable(GL_DEPTH_TEST);  
  
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  

    glutSwapBuffers(); 
    glBindVertexArray(0);
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);  // 전체 뷰포트 유지
    width = w;
    height = h;
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

void Keyboard(unsigned char key, int x, int y)
{
    switch (key) 
    {
	case 'o': // 직각 투영 / 원근 투영 토글
		oFlag = !oFlag;
        break;
	case 'm': // 애니메이션 정지/재개 토글
		mFlag = !mFlag;
        break;
    case 'y':
		targetCameraAngle += glm::radians(5.0f);
		break;
    case 'Y':
        targetCameraAngle -= glm::radians(5.0f);
		break;
    case 'r': // 미로를 제작
        // 기본 뼈대 
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
              

                if (i % 2 == 0 && j % 2 == 0)
                {
                    CubeModels[i][j].height = 4.0f;
           
                }
                else
                {
					CubeModels[i][j].height = -0.1f;
                }

                if (j == 0 || j == m - 1 || i == 0 || i == n - 1)
                {
                    CubeModels[i][j].height = 4.0f;
                }
            }
        }

        for (int i = 0; i < n; i += 2) {  // 짝수 i (행)
            for (int j = 0; j < m; j += 2) {  // 짝수 j (열)
                // 오른쪽 벽
                if (j + 1 < m && randomInt(rd) == 1) {
                    CubeModels[i][j + 1].height = 4.0f;
                }
                // 아래쪽 벽
                if (i + 1 < n && randomInt(rd) == 0) {
                    CubeModels[i + 1][j].height = 4.0f;
                }
            }
        }
		break;

    case 'v': // 육면체들이 움직임이 멈추고 낮은 높이로 변함
		mFlag = !mFlag;
        for(int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                CubeModels[i][j].height = 2.0f;
            }
		}
        break;
    case 's': // 미로에 객체 생성
		break;
	case '-': // 육면체 스피드 감소
        CubeSpeedDown();
        break;
	case '=': // 육면체 스피드 증가
        CubeSpeedUp();
        break;
    case '1': // 객체 시점 (1인칭)
		break;
	case '3': // 객체 시점 (3인칭)
        break;
	case 'c': // 모든 값 초기화
        break;
    case 'q':
        exit(0);
        break;
    }
}

void Timer(int value)
{
    float delta = 1.0f / 60.0f;
    cameraAngle = glm::mix(cameraAngle, targetCameraAngle, interpSpeed * delta);

    if (!enterAnimation)
    {
        if (!mFlag)
        {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j) {
                    CubeModels[i][j].height += CubeModels[i][j].speed * delta;
                    if (CubeModels[i][j].height >= CubeModels[i][j].MaxHeight) {
                        CubeModels[i][j].height = CubeModels[i][j].MaxHeight;
                        CubeModels[i][j].speed = -randomSpeed(rd);
                    }
                    else if (CubeModels[i][j].height <= CubeModels[i][j].minHeight) {
                        CubeModels[i][j].height = CubeModels[i][j].minHeight;
                        CubeModels[i][j].speed = randomSpeed(rd);
                    }
                }
            }
        }
       
    }
    else
    {
        if (yMoveValue <= 0.0f)
        {
            yMoveValue += 2.0f * delta;
        }
        else
        {
            yMoveValue = -5.0f;
            enterAnimation = false;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}

void Mouse(int button, int state, int x, int y)
{

}

void SpecialKeyboard(int key, int x, int y)
{

}

void Motion(int x, int y)
{

}