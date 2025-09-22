// 필요한 헤더 파일 포함
#include <GL/glew.h> // GLEW: OpenGL 확장 기능을 관리하는 라이브러리
#include <GL/freeglut.h> // GLUT: 창 생성, 입력 처리 등을 위한 라이브러리
#include <iostream> // 콘솔 출력을 위한 C++ 표준 라이브러리
#include "CommonInclude.h" // filetobuf 함수가 포함된 사용자 정의 헤더 파일

// 함수 선언: OpenGL 셰이더와 렌더링 관련 함수들을 미리 정의
void MakeVertexShaders(); // Vertex Shader 생성 함수
void MakeFragmentShaders(); // Fragment Shader 생성 함수
GLuint MakeShaderProgram(); // 셰이더 프로그램을 생성하고 연결하는 함수
GLvoid DrawScene(); // 화면에 도형을 그리는 함수
GLvoid Reshape(int w, int h); // 창 크기 변경 시 호출되는 함수

// 전역 변수 선언
GLint width, height; // 창의 너비와 높이를 저장하는 변수
GLuint shaderProgramID; // 셰이더 프로그램의 ID (OpenGL 객체를 식별)
GLuint vertexShader; // Vertex Shader의 ID
GLuint fragmentShader; // Fragment Shader의 ID

// 프로그램 진입점: OpenGL 초기화 및 창 생성
void main(int argc, char** argv) {
    // GLUT 초기화: OpenGL 창과 이벤트 처리를 위한 환경 설정
    glutInit(&argc, argv);

    // 디스플레이 모드 설정
    // GLUT_DOUBLE: 더블 버퍼링 사용 (깜빡임 방지)
    // GLUT_RGBA: 색상 모델로 Red, Green, Blue, Alpha 사용
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    // 창의 크기와 위치 초기화
    width = 800; // 창 너비 (800px)
    height = 600; // 창 높이 (600px)
    glutInitWindowPosition(100, 100); // 창의 화면 위치 (x=100, y=100)
    glutInitWindowSize(width, height); // 창 크기 설정
    glutCreateWindow("S8"); // 창 제목을 "S8"로 설정하여 창 생성

    // GLEW 초기화: OpenGL 확장 기능 사용 가능하도록 설정
    glewExperimental = GL_TRUE; // 최신 OpenGL 기능을 사용하기 위해 설정
    glewInit(); // GLEW 초기화, 실패 시 에러 처리 필요

    // 셰이더 생성 및 프로그램 연결
    MakeVertexShaders(); // Vertex Shader 파일을 읽고 컴파일
    MakeFragmentShaders(); // Fragment Shader 파일을 읽고 컴파일
    shaderProgramID = MakeShaderProgram(); // 셰이더들을 링크하여 프로그램 생성

    // 콜백 함수 등록
    glutDisplayFunc(DrawScene); // 화면을 그릴 때 호출될 함수 지정
    glutReshapeFunc(Reshape); // 창 크기 변경 시 호출될 함수 지정

    // GLUT 메인 루프 시작: 사용자 입력 및 화면 갱신을 계속 처리
    glutMainLoop();
}

// Vertex Shader를 생성하고 컴파일하는 함수
void MakeVertexShaders()
{
    // vertex.glsl 파일에서 소스 코드를 읽어옴
    GLchar* vertexSource;
    // filetobuf: 텍스트 파일을 읽어 문자열로 반환하는 사용자 정의 함수
    vertexSource = filetobuf("vertex.glsl");

    // 파일 읽기 실패 시 에러 출력
    if (vertexSource == NULL) {
        std::cerr << "ERROR: vertex.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }

    // Vertex Shader 객체 생성
    vertexShader = glCreateShader(GL_VERTEX_SHADER); // GL_VERTEX_SHADER: 정점 처리용 셰이더
    glShaderSource(vertexShader, 1, &vertexSource, NULL); // 셰이더 소스 코드 설정
    glCompileShader(vertexShader); // 셰이더 컴파일

    // 컴파일 결과 확인
    GLint result; // 컴파일 성공 여부를 저장
    GLchar errorLog[512]; // 에러 메시지를 저장할 버퍼
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result); // 컴파일 상태 확인
    if (!result) // 컴파일 실패 시
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog); // 에러 로그 가져오기
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

// Fragment Shader를 생성하고 컴파일하는 함수
void MakeFragmentShaders()
{
    // fragment.glsl 파일에서 소스 코드를 읽어옴
    GLchar* fragmentSource;
    fragmentSource = filetobuf("fragment.glsl"); // 프래그먼트 셰이더 파일 읽기

    // 파일 읽기 실패 시 에러 출력
    if (fragmentSource == NULL) {
        std::cerr << "ERROR: fragment.glsl 파일을 읽지 못했습니다." << std::endl;
        return;
    }

    // Fragment Shader 객체 생성
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // GL_FRAGMENT_SHADER: 픽셀 색상 처리용 셰이더
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL); // 셰이더 소스 코드 설정
    glCompileShader(fragmentShader); // 셰이더 컴파일

    // 컴파일 결과 확인
    GLint result; // 컴파일 성공 여부를 저장
    GLchar errorLog[512]; // 에러 메시지를 저장할 버퍼
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result); // 컴파일 상태 확인
    if (!result) // 컴파일 실패 시
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog); // 에러 로그 가져오기
        std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

// 셰이더 프로그램을 생성하고 Vertex/Fragment Shader를 링크하는 함수
GLuint MakeShaderProgram()
{
    GLuint shaderID; // 셰이더 프로그램 ID를 저장할 변수

    GLint result; // 링크 성공 여부를 저장
    GLchar errorLog[512]; // 에러 메시지를 저장할 버퍼

    // 셰이더 프로그램 객체 생성
    shaderID = glCreateProgram(); // 새로운 셰이더 프로그램 생성

    // Vertex Shader와 Fragment Shader를 프로그램에 연결
    glAttachShader(shaderID, vertexShader); // Vertex Shader 추가
    glAttachShader(shaderID, fragmentShader); // Fragment Shader 추가

    // 셰이더 프로그램 링크: 두 셰이더를 결합하여 실행 가능한 프로그램 생성
    glLinkProgram(shaderID);

    // 개별 셰이더 객체 삭제: 프로그램에 링크했으므로 더 이상 필요 없음
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 링크 결과 확인
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // 링크 상태 확인
    if (!result) // 링크 실패 시
    {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog); // 에러 로그 가져오기
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        return false; // 실패 시 0 반환
    }

    // 생성된 셰이더 프로그램을 OpenGL에 사용하도록 설정
    glUseProgram(shaderID);

    // 성공적으로 생성된 셰이더 프로그램 ID 반환
    return shaderID;
}

// 화면에 그래픽을 그리는 함수
void DrawScene() {
    // 배경색 설정: 파란색 (R=0, G=0, B=1, Alpha=1)
    GLfloat rColor, gColor, bColor;
    rColor = gColor = 0.0; // 빨강, 초록 색상 값 0
    bColor = 1.0; // 파랑 색상 값 1
    glClearColor(rColor, gColor, bColor, 1.0f); // 배경색을 파란색으로 설정

    // 색상 버퍼를 지움: 화면을 설정된 배경색으로 초기화
    glClear(GL_COLOR_BUFFER_BIT);

    // 사용할 셰이더 프로그램 활성화
    glUseProgram(shaderProgramID);

    // 점의 크기 설정: 10픽셀 크기의 점으로 그림
    glPointSize(10.0);

    // 정점을 그리는 명령: 3개의 점을 그림
    // GL_POINTS: 점으로 렌더링
    // 0: 시작 정점 인덱스
    // 3: 그릴 정점 수
    glDrawArrays(GL_POINTS, 0, 3);

    // 더블 버퍼링을 사용하여 화면에 출력
    // GLUT_DOUBLE 모드에서 화면 깜빡임을 방지하기 위해 버퍼 교체
    glutSwapBuffers();
}

// 창 크기가 변경될 때 호출되는 함수
GLvoid Reshape(int w, int h) {
    // 뷰포트 설정: 렌더링 영역을 창 크기에 맞춤
    // (0, 0): 뷰포트의 왼쪽 아래 좌표
    // w, h: 창의 너비와 높이
    glViewport(0, 0, w, h);
}