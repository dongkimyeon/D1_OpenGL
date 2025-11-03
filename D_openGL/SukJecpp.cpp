    #include "CommonInclude.h"

    std::uniform_real_distribution <float> randomSpeed(0.2f, 1.5f);
    std::random_device rd;

    //구조체
    struct ExtendedModel {
        Model baseModel;
        glm::vec3 center;
        GLuint indexCount = 0;
        std::vector<GLuint> indices;
        float height = 1.0f;
        int speed = randomSpeed(rd);  // 초기화 시 랜덤 (int로 캐스팅 필요: static_cast<int>(randomSpeed(rd)))
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
    int n = 0; // 큐브 개수 
    int m = 0;
    GLuint MazeVAO, MazeVBO, MazeEBO;
    float maxHeight = 4.0f;

    ExtendedModel** CubeModels;

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
                // 수정: 초기 speed를 int로 캐스팅 (float 분포이므로)
                CubeModels[i][j].speed = static_cast<int>(randomSpeed(rd) * 100);  // 예: 20~150 범위로 스케일링 (조정 가능)
            }
        }
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

    // 수정된 DrawScene 함수
    void DrawScene() {
        if (n <= 0 || m <= 0 || CubeModels == nullptr) {
            // nothing to draw
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glutSwapBuffers();
            return;
        }

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgramID);

        std::vector<GLfloat> vertexData; // x,y,z,r,g,b
        std::vector<GLuint> indices;

        GLuint vertexBase = 0;
        const float spacing = 1.0f;

        // 수정: 각 큐브의 index count를 미리 저장 (나중에 draw 시 사용)
        std::vector<int> cubeIndexCounts;
        GLuint currentIndexOffset = 0;  // EBO offset 추적 (빌드 중 누적)

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                Model& mdl = CubeModels[i][j].baseModel;
                glm::vec3 center = CubeModels[i][j].center;
                // 수정: speed 재설정 제거 (Timer에서만)

                float r = (float)i / std::max(1, n - 1);
                float g = (float)j / std::max(1, m - 1);
                float b = 0.5f;

                // compute translation so grid is centered around origin
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

                // add indices (adjust by current base)
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



        float camY = std::max(5.0f, std::max(n, m) * 1.5f);
        glm::vec3 cameraPos = glm::vec3(0.0f, camY, camY);
        glm::vec3 centerPoint = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, centerPoint, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);


        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        // 수정: 각 큐브마다 model matrix 설정 후 draw
        int cubeIdx = 0;
        currentIndexOffset = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                // 각 큐브의 model matrix
                glm::mat4 modelMat = glm::mat4(1.0f);
                modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.5f, 0.0f));

                // 2. S: Y축 스케일 적용 (원래 높이 1.0 기준)
                modelMat = glm::scale(modelMat, glm::vec3(1.0f, CubeModels[i][j].height, 1.0f));

                // 3. T_to_origin: 피벗(y=-0.5)을 원점(y=0)으로 이동
                modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.5f, 0.0f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

                // 이 큐브만 draw
                int thisCubeIndexCount = cubeIndexCounts[cubeIdx];
                glDrawElements(GL_TRIANGLES, thisCubeIndexCount, GL_UNSIGNED_INT,
                    (void*)(currentIndexOffset * sizeof(GLuint)));

                currentIndexOffset += thisCubeIndexCount;
                cubeIdx++;
            }
        }

        glBindVertexArray(0);

        glutSwapBuffers(); // 화면에 출력하기
    }

    GLvoid Reshape(int w, int h) {
        glViewport(0, 0, w, h);
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
        switch (key) {
	    case 'q':
		    exit(0);
            break;
        }

    }
    void Timer(int value)
    {
        float delta = 1.0f / 60.0f;


        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                CubeModels[i][j].height += CubeModels[i][j].speed * delta / 100.0f;  
                if (CubeModels[i][j].height >= maxHeight) {
                    CubeModels[i][j].height = maxHeight;
                    CubeModels[i][j].speed = -static_cast<int>(randomSpeed(rd) * 100);
                }
                else if (CubeModels[i][j].height <= 1.0f) {
                    CubeModels[i][j].height = 1.0f;
                    CubeModels[i][j].speed = static_cast<int>(randomSpeed(rd) * 100);
                }
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