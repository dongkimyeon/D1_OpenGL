#define _CRT_SECURE_NO_WARNINGS
#include "CommonInclude.h" // filetobuf 함수가 포함된 헤더 파일
#include <algorithm> // std::clamp
#include <vector>
#include <iostream>
#include <random> // 랜덤 색상 생성을 위해 추가
#include <array>
#define M_PI 3.14159265358979323846

// 랜덤 색상 생성 함수 (밝은 색상으로 제한)
std::vector<GLfloat> randomColor() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<GLfloat> dis(0.5f, 1.0f);
    return { dis(gen), dis(gen), dis(gen) };
}

struct Point
{
    std::vector<GLfloat> vertices; // x, y, r, g, b 
};

// 함수 선언
void MakeVertexShaders();
void MakeFragmentShaders();
GLuint MakeShaderProgram();
void InitBuffers();
void InitQuadrants();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void UpdateShapes();
void InitLine(std::vector<Point>& verts);
void Timer(int value);

GLint width = 800, height = 800;
GLuint shaderProgramID, vertexShader, fragmentShader;
GLuint VAO, VBO, EBO;

enum ShapeType { mLine, mTriangle, mRectangle };
ShapeType currentShape = mLine;

struct QuadrantState {
    std::vector<Point> tempVertices;
    std::vector<Point> originalPositions;
    int animationPhase;
    int currentFrame;
    int pauseRemaining;
};

std::array<QuadrantState, 4> quadrants;
const float baseX = 0.3f; // 초기 base x
const float expandX = 0.3f; // 확장 x
const float topY = 0.4f; // 상단 y
const float pentTopY = 0.6f; // 오각형 최상단 y
const float pentMidX = 0.5f; // 오각형 중단 x (대칭 위해)
const int totalFrames = 60; // 각 phase 애니메이션 프레임 수 (약 1초)
const int pauseFrames = 60; // 각 phase 간 딜레이 프레임 수 (약 1초)

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("S12");

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
    InitQuadrants(); // 각 사분면 초기화

    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, Timer, 0); // 16ms마다 Timer 호출 (약 60fps)

    glutMainLoop();
    return 0;
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
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // 위치 속성 (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    // 색상 속성 (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void InitLine(std::vector<Point>& verts) {
    verts.clear();
    auto c0 = randomColor();
    verts.push_back(Point{ {-baseX, 0.0f, c0[0], c0[1], c0[2]} });
    auto c1 = randomColor();
    verts.push_back(Point{ {baseX, 0.0f, c1[0], c1[1], c1[2]} });
}

void InitQuadrants() {
    for (size_t i = 0; i < 4; ++i) {
        auto& q = quadrants[i];
        InitLine(q.tempVertices);
        q.animationPhase = 1;
        q.currentFrame = 0;
        q.pauseRemaining = 0;
        q.originalPositions.clear();
    }

    // Quadrant 2 (index 1): start at phase 2 (4 points, P2 and P3 at end of phase 1)
    {
        auto& q = quadrants[1];
        q.animationPhase = 2;
        auto c2 = randomColor();
        q.tempVertices.push_back(Point{ {0.0f, topY, c2[0], c2[1], c2[2]} });
        auto c3 = randomColor();
        q.tempVertices.push_back(Point{ {0.0f, topY, c3[0], c3[1], c3[2]} });
    }

    // Quadrant 3 (index 2): start at phase 3 (5 points, at start of phase 3)
    {
        auto& q = quadrants[2];
        q.animationPhase = 3;
        auto c2 = randomColor();
        q.tempVertices.push_back(Point{ {expandX, topY, c2[0], c2[1], c2[2]} });
        auto c3 = randomColor();
        q.tempVertices.push_back(Point{ {-expandX, topY, c3[0], c3[1], c3[2]} });
        auto c4 = randomColor();
        q.tempVertices.push_back(Point{ {0.0f, topY, c4[0], c4[1], c4[2]} });
    }

    // Quadrant 4 (index 3): start at phase 4 (5 points, at end of phase 3)
    {
        auto& q = quadrants[3];
        q.animationPhase = 4;
        float p2x = expandX + (pentMidX - expandX) / 2.0f;
        float p3x = -expandX + (-pentMidX + expandX) / 2.0f;
        auto c2 = randomColor();
        q.tempVertices.push_back(Point{ {p2x, topY, c2[0], c2[1], c2[2]} });
        auto c3 = randomColor();
        q.tempVertices.push_back(Point{ {p3x, topY, c3[0], c3[1], c3[2]} });
        auto c4 = randomColor();
        q.tempVertices.push_back(Point{ {0.0f, pentTopY, c4[0], c4[1], c4[2]} });
        q.originalPositions = q.tempVertices;
    }
}

// 사분면에 따라 좌표 변환 + 중앙 오프셋 함수
std::vector<GLfloat> TransformVerticesForQuadrant(const std::vector<Point>& points, int quadrant) {
    std::vector<GLfloat> data;
    for (const auto& point : points) {
        GLfloat x = point.vertices[0];
        GLfloat y = point.vertices[1];
        switch (quadrant) {
        case 1: // 오른쪽 위
            x += 0.5f;
            y += 0.3f;
            break;
        case 2: // 왼쪽 위
            x = -x - 0.5f;
            y += 0.3f;
            break;
        case 3: // 왼쪽 아래
            x = x - 0.5f;
            y = y - 0.7f;
            break;
        case 4: // 오른쪽 아래
            x += 0.5f;
            y = y - 0.7f;
            break;
        }
        data.push_back(x);
        data.push_back(y);
        data.insert(data.end(), point.vertices.begin() + 2, point.vertices.end()); // r,g,b 추가
    }
    return data;
}

void DrawScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO);

    // 화면을 4등분하는 선 (기존 코드 유지)
    std::vector<GLfloat> lines = {
        // 수직선
        0.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        // 수평선
        -1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 0.0f, 1.0f, 1.0f, 1.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(GLfloat), lines.data(), GL_DYNAMIC_DRAW);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 4);

    // 각 사분면에 독립적인 도형 그리기
    for (int quad = 1; quad <= 4; ++quad) {
        int qidx = quad - 1;
        const auto& state = quadrants[qidx];
        size_t numPoints = state.tempVertices.size();
        if (numPoints >= 2) {
            std::vector<GLfloat> quadData;
            const std::vector<Point>& points = state.tempVertices;

            if (numPoints == 2) {
                // 선: 2개 정점 변환
                std::vector<GLfloat> transformed = TransformVerticesForQuadrant(points, quad);
                quadData = transformed;
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, quadData.size() * sizeof(GLfloat), quadData.data(), GL_DYNAMIC_DRAW);
                glLineWidth(4.0f);
                glDrawArrays(GL_LINES, 0, 2);
            }
            else if (numPoints == 3) {
                // 삼각형: 3개 정점 변환
                std::vector<GLfloat> transformed = TransformVerticesForQuadrant(points, quad);
                quadData = transformed;
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, quadData.size() * sizeof(GLfloat), quadData.data(), GL_DYNAMIC_DRAW);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            else if (numPoints == 4) {
                // 사각형: 두 개의 삼각형 변환
                // tri1: P0, P1, P2
                std::vector<Point> tri1 = { points[0], points[1], points[2] };
                std::vector<GLfloat> transformed1 = TransformVerticesForQuadrant(tri1, quad);
                quadData.insert(quadData.end(), transformed1.begin(), transformed1.end());
                // tri2: P0, P2, P3
                std::vector<Point> tri2 = { points[0], points[2], points[3] };
                std::vector<GLfloat> transformed2 = TransformVerticesForQuadrant(tri2, quad);
                quadData.insert(quadData.end(), transformed2.begin(), transformed2.end());
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, quadData.size() * sizeof(GLfloat), quadData.data(), GL_DYNAMIC_DRAW);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            else if (numPoints == 5) {
                // 오각형: GL_TRIANGLE_FAN 변환 (순서 유지)
                // 첫 점 (P0) 반복 추가
                std::vector<Point> fanPoints = { points[0] };
                // 순서: P1, P2, P4, P3, P0
                int order[5] = { 1, 2, 4, 3, 0 };
                for (int i = 0; i < 5; ++i) {
                    fanPoints.push_back(points[order[i]]);
                }
                std::vector<GLfloat> transformed = TransformVerticesForQuadrant(fanPoints, quad);
                quadData = transformed;
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, quadData.size() * sizeof(GLfloat), quadData.data(), GL_DYNAMIC_DRAW);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 6); // 6 버텍스
            }
        }
    }

    glBindVertexArray(0);
    glutSwapBuffers();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

void Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // 마우스 이벤트는 기존과 동일 (미구현)
    }
}

void Keyboard(unsigned char key, int x, int y) {
    // 키보드 이벤트는 기존과 동일 (미구현)
    glutPostRedisplay();
}

void Timer(int value)
{
    UpdateShapes();
    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0); // 다음 16ms 후에 다시 호출
}

void UpdateShapes()
{
    for (auto& q : quadrants) {
        if (q.pauseRemaining > 0) {
            q.pauseRemaining--;
            continue;
        }

        float progress = static_cast<float>(q.currentFrame) / totalFrames;
        auto& verts = q.tempVertices;

        if (q.animationPhase == 1) {
            // Phase 1: 3점으로 성장 (P2 y 이동)
            if (verts.size() == 2 && q.currentFrame == 0) {
                auto c2 = randomColor();
                Point newP2{ {0.0f, 0.0f, c2[0], c2[1], c2[2]} };
                verts.push_back(newP2);
            }
            if (verts.size() == 3 && q.currentFrame <= totalFrames) {
                verts[2].vertices[1] = 0.0f + topY * progress;
                q.currentFrame++;
                if (q.currentFrame > totalFrames) {
                    verts[2].vertices[1] = topY;
                    q.animationPhase = 2;
                    q.currentFrame = 0;
                    // P3 추가 (초기: P2와 같은 위치)
                    auto c3 = randomColor();
                    Point newP3{ {0.0f, topY, c3[0], c3[1], c3[2]} };
                    verts.push_back(newP3);
                    q.pauseRemaining = pauseFrames;
                }
                continue;
            }
        }
        else if (q.animationPhase == 2) {
            // Phase 2: 4점으로 확장 (x 이동)
            if (q.currentFrame <= totalFrames) {
                // P2: 0 -> 0.5
                verts[2].vertices[0] = 0.0f + expandX * progress;
                // P3: 0 -> -0.5
                verts[3].vertices[0] = 0.0f + (-expandX) * progress;
                q.currentFrame++;
                if (q.currentFrame > totalFrames) {
                    q.animationPhase = 3;
                    q.currentFrame = 0;
                    // P4 추가 (초기: 중앙 상단, topY)
                    auto c4 = randomColor();
                    Point newP4{ {0.0f, topY, c4[0], c4[1], c4[2]} };
                    verts.push_back(newP4);
                    q.pauseRemaining = pauseFrames;
                }
            }
        }
        else if (q.animationPhase == 3) {
            // Phase 3: 5점으로 확장 (P4 y 증가, P0/P1/P2/P3 x 안쪽으로 조정 for 대칭 오각형)
            if (q.currentFrame <= totalFrames) {
                // P0: x -expandX -> -baseX (아래 좌 안쪽)
                verts[0].vertices[0] = -expandX + (-baseX + expandX) * progress;
                // P1: x expandX -> baseX (아래 우 안쪽)
                verts[1].vertices[0] = expandX + (baseX - expandX) * progress;
                // P4: y topY -> pentTopY
                verts[4].vertices[1] = topY + (pentTopY - topY) * progress;
                // P2: x expandX -> pentMidX (중 우 안쪽)
                verts[2].vertices[0] = expandX + (pentMidX - expandX) * progress / 2;
                // P3: x -expandX -> -pentMidX (중 좌 안쪽)
                verts[3].vertices[0] = -expandX + (-pentMidX + expandX) * progress / 2;
                q.currentFrame++;
                if (q.currentFrame > totalFrames) {
                    q.animationPhase = 4;
                    q.currentFrame = 0;
                    q.pauseRemaining = pauseFrames;
                }
            }
        }
        else if (q.animationPhase == 4) {
            // Phase 4: 오각형을 선으로 모핑
            if (q.currentFrame == 0) {
                q.originalPositions = verts; // 축소 시작 시 원본 위치 저장
            }
            if (q.currentFrame <= totalFrames) {
                float morphProgress = static_cast<float>(q.currentFrame) / totalFrames; // 0 -> 1 (모핑 진행률)
                // 타겟 위치: P0(-0.3,0), P1(0.3,0), P2(0.3,0), P3(-0.3,0), P4(0,0) - base 선으로 collapse
                std::vector<GLfloat> targetX = { -baseX, baseX, baseX, -baseX, 0.0f };
                std::vector<GLfloat> targetY = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
                for (size_t i = 0; i < 5; ++i) {
                    verts[i].vertices[0] = q.originalPositions[i].vertices[0] * (1.0f - morphProgress) + targetX[i] * morphProgress;
                    verts[i].vertices[1] = q.originalPositions[i].vertices[1] * (1.0f - morphProgress) + targetY[i] * morphProgress;
                }
                q.currentFrame++;
                if (q.currentFrame > totalFrames) {
                    // 리셋: 2점 선으로 새로 시작 (새 랜덤 색상)
                    InitLine(verts);
                    q.animationPhase = 1;
                    q.currentFrame = 0;
                    q.pauseRemaining = pauseFrames;
                }
            }
        }
    }
}