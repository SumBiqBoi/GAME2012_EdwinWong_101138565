#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTcoord;

uniform mat4 u_mvp;
uniform mat4 u_world;
uniform mat3 u_normal;

out vec3 position;
out vec3 normal;
out vec2 tcoord;

void main()
{
   position = (u_world * vec4(aPosition, 1.0)).xyz;
   gl_Position = u_mvp * vec4(aPosition, 1.0);
   normal = u_normal * aNormal;
   tcoord = aTcoord;
}
