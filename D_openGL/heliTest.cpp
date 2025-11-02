#include <GL/glut.h>
#include <cmath>
#include <iostream>

#define M_PI 3.14159265358979323846
float heliX = 0.0f, heliY = 2.0f, heliZ = 0.0f;  // 헬리콥터 초기 위치 (바닥 위 시작)
float yaw = 0.0f;    // yaw 회전 (방향)
float pitch = 0.0f;  // pitch 회전 (기수 상하)
float roll = 0.0f;   // roll 회전 (기울기)
float power = 0.5f;  // 엔진 출력 (리프트, 초기값 0.5로 안정적 시작)
float rotorAngle = 0.0f;  // 로터 회전 각도 (애니메이션)

float verticalVelocity = 0.0f;  // 수직 속도
float forwardVelocity = 0.0f;  // 전진 속도
float sideVelocity = 0.0f;     // 측면 속도
const float gravity = -0.002f;  // 중력 가속도
const float liftFactor = 0.006f;  // 리프트 계수 (더 부드럽게)
const float drag = 0.98f;  // 공기 저항 (더 강하게, 빠른 감쇠)

int lastMouseX = 0, lastMouseY = 0;  // 마우스 이전 위치
bool mouseCaptured = false;  // 마우스 캡처 상태

void drawHelicopter() {
    glPushMatrix();
    glTranslatef(heliX, heliY, heliZ);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);    // yaw 회전
    glRotatef(pitch, 1.0f, 0.0f, 0.0f);  // pitch 회전
    glRotatef(roll, 0.0f, 0.0f, 1.0f);   // roll 회전

    // 몸체 (큐브, 약간 길게)
    glPushMatrix();
    glScalef(1.5f, 0.8f, 0.8f);  // 길쭉한 몸체
    glColor3f(0.0f, 0.5f, 0.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 메인 로터 (더 사실적으로)
    glPushMatrix();
    glTranslatef(0.0f, 0.6f, 0.0f);
    glRotatef(rotorAngle, 0.0f, 1.0f, 0.0f);  // 로터 회전
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_QUADS);
    glVertex3f(-2.5f, 0.0f, 0.1f);
    glVertex3f(2.5f, 0.0f, 0.1f);
    glVertex3f(2.5f, 0.0f, -0.1f);
    glVertex3f(-2.5f, 0.0f, -0.1f);
    glEnd();
    glPopMatrix();

    // 테일 로터
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.2f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(rotorAngle * 2.0f, 0.0f, 0.0f, 1.0f);  // 더 빠르게
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_QUADS);
    glVertex3f(-0.4f, 0.0f, 0.05f);
    glVertex3f(0.4f, 0.0f, 0.05f);
    glVertex3f(0.4f, 0.0f, -0.05f);
    glVertex3f(-0.4f, 0.0f, -0.05f);
    glEnd();
    glPopMatrix();

    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 카메라: 헬리콥터 뒤에서 부드럽게 따라감
    float camX = heliX - 20.0f * sin(yaw * M_PI / 180.0f);
    float camY = heliY + 7.0f;
    float camZ = heliZ - 20.0f * cos(yaw * M_PI / 180.0f);
    gluLookAt(camX, camY, camZ, heliX, heliY, heliZ, 0.0f, 1.0f, 0.0f);

    // 바닥 (더 넓게, 색상 추가)
    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.3f, 0.1f);  // 잔디 같은 색상
    glVertex3f(-100.0f, 0.0f, -100.0f);
    glVertex3f(100.0f, 0.0f, -100.0f);
    glVertex3f(100.0f, 0.0f, 100.0f);
    glVertex3f(-100.0f, 0.0f, 100.0f);
    glEnd();

    drawHelicopter();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a': roll += 3.0f; break;  // roll 왼쪽 (빠르게)
    case 'd': roll -= 3.0f; break;  // roll 오른쪽
    case 'w': power += 0.05f; break;  // 리프트 증가
    case 's': power -= 0.05f; break;  // 리프트 감소
    case 27: exit(0); break;  // ESC로 종료
    }
    if (power > 1.0f) power = 1.0f;  // 최대 출력
    if (power < 0.0f) power = 0.0f;  // 최소 출력
    if (roll > 60.0f) roll = 60.0f;  // roll 제한 확대
    if (roll < -60.0f) roll = -60.0f;
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouseCaptured = true;
        lastMouseX = x;
        lastMouseY = y;
        glutSetCursor(GLUT_CURSOR_NONE);  // 커서 숨김 (게임 스타일)
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        mouseCaptured = false;
        glutSetCursor(GLUT_CURSOR_LEFT_ARROW);  // 커서 복구
    }
}

void motion(int x, int y) {
    if (mouseCaptured) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;
        yaw -= dx * 0.3f;  // 마우스 좌우: yaw (더 민감하게)
        pitch += dy * 0.3f;  // 마우스 상하: pitch (위로 올리면 nose up)
        lastMouseX = x;
        lastMouseY = y;
        if (yaw > 360) yaw -= 360;
        if (yaw < -360) yaw += 360;
        if (pitch > 60.0f) pitch = 60.0f;
        if (pitch < -60.0f) pitch = -60.0f;

        // 마우스 중앙으로 리셋 (게임 스타일)
        int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
        int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
        if (x != centerX || y != centerY) {
            glutWarpPointer(centerX, centerY);
            lastMouseX = centerX;
            lastMouseY = centerY;
        }
        glutPostRedisplay();
    }
}

void idle() {
    // 로터 애니메이션
    rotorAngle += 10.0f;  // 더 빠른 로터
    if (rotorAngle > 360) rotorAngle -= 360;

    // 물리 업데이트
    // 수직 운동
    float lift = power * liftFactor;
    verticalVelocity += lift + gravity;
    verticalVelocity *= drag;
    heliY += verticalVelocity;

    // 바닥 충돌
    if (heliY <= 0.5f) {  // 헬리콥터 중심이 바닥보다 약간 위
        heliY = 0.5f;
        verticalVelocity = 0.0f;
    }

    // 전진/측면 운동
    float pitchAccel = sin(pitch * M_PI / 180.0f) * 0.008f;  // pitch up (positive) = backward
    float rollAccel = -sin(roll * M_PI / 180.0f) * 0.008f;  // roll left (positive) = left
    forwardVelocity += pitchAccel;
    sideVelocity += rollAccel;
    forwardVelocity *= drag;
    sideVelocity *= drag;

    // 위치 업데이트 (yaw 방향)
    heliX += forwardVelocity * sin(yaw * M_PI / 180.0f) + sideVelocity * cos(yaw * M_PI / 180.0f);
    heliZ += forwardVelocity * cos(yaw * M_PI / 180.0f) - sideVelocity * sin(yaw * M_PI / 180.0f);

    glutPostRedisplay();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)w / h, 0.1f, 300.0f);  // 더 넓은 시야
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Delta Force Style Helicopter");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // 하늘색 배경

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}