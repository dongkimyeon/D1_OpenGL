#version 330 core
layout(location = 0) in vec2 aPos; //0 ~ 8바이트까지 들어가고
layout(location = 1) in vec3 aColor; //8바이트부터 16 바이트까지 

out vec3 ourColor;




void main() {

   
    gl_Position = vec4(aPos, 0.0, 1.0);
    ourColor = aColor;

}