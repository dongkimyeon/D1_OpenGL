#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    // view 행렬에서 이동(translation) 부분을 제거하여 스카이박스가 카메라를 따라다니도록 함
    mat4 view_no_translation = mat4(mat3(view)); 
    vec4 pos = projection * view_no_translation * vec4(aPos, 1.0);
    
    // 이 트릭은 스카이박스가 항상 다른 모든 객체 뒤에 그려지도록 보장합니다.
    // z 값을 w와 같게 설정하여, perspective divide 이후 깊이(z/w)가 1.0이 되게 합니다.
    gl_Position = pos.xyww; 
}