// GLSL 버전을 지정: 3.3 core 프로파일 사용
// #version 330: GLSL 3.3 버전, core는 현대 OpenGL 기능만 사용
#version 330 core

// Vertex Shader의 메인 함수: 각 정점마다 호출됨
void main()
{
    // 3개의 정점을 정의하는 상수 배열
    // vec4: 4차원 벡터(x, y, z, w)로, 위치를 나타냄
    // const: 배열은 수정 불가능
    // 배열 요소는 삼각형의 3개 정점 좌표
    const vec4 vertex[3] = vec4[3] (
        vec4(-0.25, -0.25, 0.5, 1.0), // 첫 번째 정점: 화면의 왼쪽 아래
        vec4(0.25, -0.25, 0.5, 1.0),  // 두 번째 정점: 화면의 오른쪽 아래
        vec4(0.0, 0.25, 0.5, 1.0)     // 세 번째 정점: 화면의 중앙 위쪽
    );

    // gl_Position: 정점의 최종 위치를 OpenGL에 전달
    // gl_VertexID: 현재 처리 중인 정점의 인덱스(0, 1, 2)
    // vertex[gl_VertexID]: 해당 인덱스의 정점을 선택하여 위치 설정
    gl_Position = vertex[gl_VertexID];
}