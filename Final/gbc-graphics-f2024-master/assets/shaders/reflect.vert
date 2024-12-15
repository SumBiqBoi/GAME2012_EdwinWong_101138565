#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_mvp;
uniform mat4 u_world;
uniform mat3 u_normal;

out vec3 position;
out vec3 normal;

void main()
{
   normal = mat3(transpose(inverse(u_world))) * aNormal;
   position = vec3(u_world * vec4(aPosition, 1.0));
   gl_Position = u_mvp * vec4(aPosition, 1.0);
}
