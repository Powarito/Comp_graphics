#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 projection;
uniform mat4 moveOrigin;
uniform mat4 scaleToPixels;
uniform mat4 affineBasis;
uniform mat4 rawModel;
out vec3     ourColor;

void main() {
    gl_Position = projection * moveOrigin * scaleToPixels * affineBasis * rawModel * vec4(aPos.xy, 0.0, 1.0);
    ourColor = aColor;
}
