#include "CommonInclude.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500




struct Rect {
    float x1, y1, x2, y2;
    float r, g, b;
	bool ismatched = false;
};

std::vector<Rect> boardRects;
std::vector<Rect> UserRects;

int selectedRectIndex = -1;
bool isDragging = false;
bool autoMatch = false;

void InitBoard();
void InitUserRects();
void DrawScene();
void Reshape(int w, int h);
void DrawBoard();
void Mouse(int button, int state, int x, int y);
void Keyboard(unsigned char key, int x, int y);
void Motion(int x, int y);
bool isOverlapping(const Rect& newRect, const std::vector<Rect>& existingRects) {
    for (const auto& rect : existingRects) {
        if (!(newRect.x2 <= rect.x1 || newRect.x1 >= rect.x2 ||
            newRect.y1 <= rect.y2 || newRect.y2 >= rect.y1)) {
            return true; 
        }
    }
    return false; 
}
void AutoMatching(int value);


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("S7");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "GLEW 초기화 성공\n";
    }

    InitBoard();
    InitUserRects();
    glutDisplayFunc(DrawScene);
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
	glutKeyboardFunc(Keyboard);
	
    glutMainLoop();
    return 0;
}

void InitBoard() {
    boardRects.clear();

    std::vector<Rect> predefinedRects = {
        // 첫 번째 줄
        {-0.9f, 0.9f, -0.75f, 0.4f, 0.6f, 0.4f, 0.5f}, // 사각형 1
        {-0.75f, 0.9f, -0.55f, 0.5f, 0.5f, 0.6f, 0.4f}, // 사각형 2

        // 두 번째 줄
        {-0.9f, 0.4f, -0.7f, 0.0f, 0.7f, 0.5f, 0.3f},  // 사각형 3
        {-0.7f, 0.4f, -0.4f, 0.15f, 0.3f, 0.7f, 0.5f},  // 사각형 4

        // 세 번째 줄
        {-0.9f, 0.0f, -0.75f, -0.4f, 0.5f, 0.3f, 0.7f}, // 사각형 5
        {-0.75f, 0.0f, -0.45f, -0.25f, 0.6f, 0.7f, 0.4f}, // 사각형 6

        // 네 번째 줄
        {-0.9f, -0.4f, -0.7f, -0.8f, 0.7f, 0.4f, 0.6f},  // 사각형 7
        {-0.7f, -0.25f, -0.5f, -0.65f, 0.4f, 0.6f, 0.5f}, // 사각형 8

        // 다섯 번째 줄
        {-0.9f, -0.8f, -0.65f, -1.0f, 0.5f, 0.6f, 0.7f}, // 사각형 9
        {-0.65f, -0.8f, -0.45f, -1.0f, 0.6f, 0.4f, 0.5f} // 사각형 10
    };

    for(auto& rect : predefinedRects) {
        rect.ismatched = false; // 매칭 상태 초기화
    }
    boardRects = predefinedRects;

    
}
void InitUserRects() {
    UserRects.clear();

    // 난수 생성기 초기화
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist_x(0.1f, 0.9f); // x 좌표 범위 (오른쪽 절반)
    std::uniform_real_distribution<float> dist_y(-0.9f, 0.9f); // y 좌표 범위 (전체 높이)

    for (const auto& r : boardRects) {
        Rect temp = r;
		
        float width = r.x2 - r.x1;
        float height = r.y1 - r.y2;
        bool placed = false;
        int attempts = 0;
        const int maxAttempts = 100; // 최대 시도 횟수

        while (!placed && attempts < maxAttempts) {
            // 랜덤한 중심점 생성
            float center_x = dist_x(gen);
            float center_y = dist_y(gen);

            // 새로운 좌표 계산
            temp.x1 = center_x - width / 2.0f;
            temp.x2 = center_x + width / 2.0f;
            temp.y1 = center_y + height / 2.0f;
            temp.y2 = center_y - height / 2.0f;

            // 화면 경계를 벗어나지 않도록 조정
            if (temp.x1 < 0.0f) {
                temp.x1 = 0.0f;
                temp.x2 = temp.x1 + width;
            }
            else if (temp.x2 > 1.0f) {
                temp.x2 = 1.0f;
                temp.x1 = temp.x2 - width;
            }
            if (temp.y1 > 1.0f) {
                temp.y1 = 1.0f;
                temp.y2 = temp.y1 - height;
            }
            else if (temp.y2 < -1.0f) {
                temp.y2 = -1.0f;
                temp.y1 = temp.y2 + height;
            }

            // 기존 사각형과 겹치는지 확인
            if (!isOverlapping(temp, UserRects)) {
                UserRects.push_back(temp);
                placed = true;
            }
            attempts++;
        }

        if (!placed) {
            // 최대 시도 횟수를 초과하면 기본 위치로 배치
            temp.x1 = 0.1f + (UserRects.size() % 4) * 0.2f;
            temp.x2 = temp.x1 + width;
            temp.y1 = 0.9f - (UserRects.size() / 4) * 0.4f;
            temp.y2 = temp.y1 - height;
            UserRects.push_back(temp);
        }
    }

    for(auto& r : boardRects) {
        r.ismatched = false; // 매칭 상태 초기화
	}
}
void DrawScene() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 어두운 배경
    glClear(GL_COLOR_BUFFER_BIT);
    DrawBoard();
    glutSwapBuffers();
}
void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}
void DrawBoard() {
    glLineWidth(2.0f); // 테두리 선 두께 설정
    for (const auto& rect : boardRects) {
        glColor3f(rect.r, rect.g, rect.b);
        glBegin(GL_LINE_LOOP);
        glVertex2f(rect.x1, rect.y1); // 좌상단
        glVertex2f(rect.x2, rect.y1); // 우상단
        glVertex2f(rect.x2, rect.y2); // 우하단
        glVertex2f(rect.x1, rect.y2); // 좌하단
        glEnd();
    }
    for (const auto& rect : UserRects) {
        glColor3f(rect.r, rect.g, rect.b);
        glRectf(rect.x1, rect.y1, rect.x2, rect.y2); // 채워진 사각형
    }
}
void Mouse(int button, int state, int x, int y) {
    float ndc_x = (2.0f * x / glutGet(GLUT_WINDOW_WIDTH)) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / glutGet(GLUT_WINDOW_HEIGHT));

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            selectedRectIndex = -1;
            for (int i = UserRects.size() - 1; i >= 0; --i) {
                if (ndc_x >= UserRects[i].x1 && ndc_x <= UserRects[i].x2 &&
                    ndc_y <= UserRects[i].y1 && ndc_y >= UserRects[i].y2 && !UserRects[i].ismatched) {
                    selectedRectIndex = i;
                    isDragging = true;
                    //std::cout << "Selected Rect: " << i << " at (" << ndc_x << ", " << ndc_y << ")\n";
                    break;
                }
            }
        }
        else if (state == GLUT_UP) {
            if (selectedRectIndex >= 0) {
                // 보드 사각형과 유저 사각형 비교
                const float EPSILON = 0.05f; // 오차 허용 범위 
                bool matched = false;
                int matchedBoardIndex = -1;

                for (int i = 0; i < boardRects.size(); ++i) {
                    const auto& board = boardRects[i];
                    const auto& user = UserRects[selectedRectIndex];

                    // 중심점 기반으로 근접 여부 확인
                    float userCenterX = (user.x1 + user.x2) / 2.0f;
                    float userCenterY = (user.y1 + user.y2) / 2.0f;
                    float boardCenterX = (board.x1 + board.x2) / 2.0f;
                    float boardCenterY = (board.y1 + board.y2) / 2.0f;

                    if (std::abs(userCenterX - boardCenterX) < EPSILON &&
                        std::abs(userCenterY - boardCenterY) < EPSILON &&
                        std::abs(user.x2 - user.x1 - (board.x2 - board.x1)) < EPSILON &&
                        std::abs(user.y1 - user.y2 - (board.y1 - board.y2)) < EPSILON
                       ) {
                        matched = true;
                        matchedBoardIndex = i;
                        break;
                    }
                }

                if (matched && UserRects[selectedRectIndex].r == boardRects[matchedBoardIndex].r
                    && UserRects[selectedRectIndex].g == boardRects[matchedBoardIndex].g
                    && UserRects[selectedRectIndex].b == boardRects[matchedBoardIndex].b) {
                    // 스냅: 유저 사각형을 보드 사각형의 정확한 위치로 이동
                    UserRects[selectedRectIndex].x1 = boardRects[matchedBoardIndex].x1;
                    UserRects[selectedRectIndex].y1 = boardRects[matchedBoardIndex].y1;
                    UserRects[selectedRectIndex].x2 = boardRects[matchedBoardIndex].x2;
                    UserRects[selectedRectIndex].y2 = boardRects[matchedBoardIndex].y2;

					UserRects[selectedRectIndex].ismatched = true; // 매칭 완료 표시
                    UserRects[selectedRectIndex].r = 1.0f;
                    UserRects[selectedRectIndex].g = 1.0f;
                    UserRects[selectedRectIndex].b = 1.0f;

                    std::cout << "UserRect " << selectedRectIndex << " matched to BoardRect " << matchedBoardIndex << "\n";
                }
            }
            isDragging = false;
            selectedRectIndex = -1;
            glutPostRedisplay();
        }
    }
}
void Motion(int x, int y) {
    if (isDragging && selectedRectIndex >= 0 && selectedRectIndex < static_cast<int>(UserRects.size())) {
        float ndc_x = (2.0f * x / glutGet(GLUT_WINDOW_WIDTH)) - 1.0f;
        float ndc_y = 1.0f - (2.0f * y / glutGet(GLUT_WINDOW_HEIGHT));
        // 사각형 이동
        float width = UserRects[selectedRectIndex].x2 - UserRects[selectedRectIndex].x1;
        float height = UserRects[selectedRectIndex].y1 - UserRects[selectedRectIndex].y2;
        UserRects[selectedRectIndex].x1 = ndc_x - width / 2.0f;
        UserRects[selectedRectIndex].x2 = ndc_x + width / 2.0f;
        UserRects[selectedRectIndex].y1 = ndc_y + height / 2.0f;
        UserRects[selectedRectIndex].y2 = ndc_y - height / 2.0f;
        glutPostRedisplay();
    }
}
void Keyboard(unsigned char key, int x, int y)
{
    if(key == 'q') // ESC 키
    {
        exit(0);
    }
    else if (key == 'r' || key == 'R') // 'r' 키를 눌러 초기화
    {
       
        InitUserRects();
        isDragging = false;
        selectedRectIndex = -1;
        for (auto& rect : UserRects) {
            rect.ismatched = false; // 매칭 상태 초기화
        }

        glutPostRedisplay();
	}
    else if(key == 'a' || key == 'A') // 'a' 키를 눌러 자동 매칭
    {
		autoMatch = !autoMatch;
		//std::cout << "자동매칭을 실시합니다." << std::endl;
		glutTimerFunc(100, AutoMatching, 1);
	}
}
void AutoMatching(int value) {
    if (!autoMatch) return;

    bool allMatched = true; // 모든 사각형이 매칭되었는지 확인
	
    for (int i = 0; i < UserRects.size(); ++i) {
        if (UserRects[i].ismatched) continue; // 이미 매칭된 사각형은 건너뜀

        allMatched = false; // 매칭되지 않은 사각형이 있으므로 false

        // 가장 가까운 boardRect 찾기
        int bestMatchIndex = -1;
        float minDistance = std::numeric_limits<float>::max();
        const float EPSILON = 0.05f; // 매칭 허용 오차

        for (int j = 0; j < boardRects.size(); ++j) {
            if (boardRects[j].ismatched) continue; // 이미 매칭된 boardRect는 건너뜀

            // 중심점 계산
            float userCenterX = (UserRects[i].x1 + UserRects[i].x2) / 2.0f;
            float userCenterY = (UserRects[i].y1 + UserRects[i].y2) / 2.0f;
            float boardCenterX = (boardRects[j].x1 + boardRects[j].x2) / 2.0f;
            float boardCenterY = (boardRects[j].y1 + boardRects[j].y2) / 2.0f;

            // 크기 비교
            float userWidth = UserRects[i].x2 - UserRects[i].x1;
            float userHeight = UserRects[i].y1 - UserRects[i].y2;
            float boardWidth = boardRects[j].x2 - boardRects[j].x1;
            float boardHeight = boardRects[j].y1 - boardRects[j].y2;

            // 색상과 크기가 일치하는지 확인
            if (std::abs(userWidth - boardWidth) < EPSILON &&
                std::abs(userHeight - boardHeight) < EPSILON &&
                std::abs(UserRects[i].r - boardRects[j].r) < EPSILON &&
                std::abs(UserRects[i].g - boardRects[j].g) < EPSILON &&
                std::abs(UserRects[i].b - boardRects[j].b) < EPSILON) {
                float distance = std::sqrt(std::pow(userCenterX - boardCenterX, 2) +
                    std::pow(userCenterY - boardCenterY, 2));
                if (distance < minDistance) {
                    minDistance = distance;
                    bestMatchIndex = j;
                }
            }
        }

        if (bestMatchIndex != -1) {
            // 부드러운 이동을 위해 보간(lerp) 사용
            const float SPEED = 0.05f; // 이동 속도 조절 (0.0f ~ 1.0f)
            float targetX1 = boardRects[bestMatchIndex].x1;
            float targetY1 = boardRects[bestMatchIndex].y1;
            float targetX2 = boardRects[bestMatchIndex].x2;
            float targetY2 = boardRects[bestMatchIndex].y2;

            // 현재 위치에서 목표 위치로 부드럽게 이동
            UserRects[i].x1 += (targetX1 - UserRects[i].x1) * SPEED;
            UserRects[i].y1 += (targetY1 - UserRects[i].y1) * SPEED;
            UserRects[i].x2 += (targetX2 - UserRects[i].x2) * SPEED;
            UserRects[i].y2 += (targetY2 - UserRects[i].y2) * SPEED;

            // 목표 위치에 거의 도달했는지 확인
            if (std::abs(UserRects[i].x1 - targetX1) < EPSILON &&
                std::abs(UserRects[i].y1 - targetY1) < EPSILON &&
                std::abs(UserRects[i].x2 - targetX2) < EPSILON &&
                std::abs(UserRects[i].y2 - targetY2) < EPSILON) {
                // 정확한 위치로 스냅
                UserRects[i].x1 = targetX1;
                UserRects[i].y1 = targetY1;
                UserRects[i].x2 = targetX2;
                UserRects[i].y2 = targetY2;
                UserRects[i].ismatched = true;
               
                // 매칭된 사각형 색상 변경
                UserRects[i].r = 1.0f;
                UserRects[i].g = 1.0f;
                UserRects[i].b = 1.0f;

                std::cout << "UserRect " << i << " matched to BoardRect " << bestMatchIndex << "\n";
            }
        }
    }

    
    if (allMatched) {
        autoMatch = false;
        std::cout << "모든 사각형이 매칭되었습니다!" << std::endl;
    }
    else {
        // 계속해서 업데이트
        glutPostRedisplay();
        glutTimerFunc(16, AutoMatching, 1); // 약 60fps로 업데이트 (1000ms / 60 ≈ 16ms)
    }
}
