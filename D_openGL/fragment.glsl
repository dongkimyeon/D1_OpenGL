#version 330 core
in vec3 ourColor;
in vec2 UV;
out vec4 FragColor;
uniform sampler2D ourTexture;
uniform bool useVertexColor; // <-- 이 변수 추가
void main() {

    if (useVertexColor)
    {
        FragColor = vec4(ourColor, 1.0); // 정점 색상 사용
    }
    else
    {
        FragColor = texture(ourTexture, UV); // 텍스처 사용
    }

}