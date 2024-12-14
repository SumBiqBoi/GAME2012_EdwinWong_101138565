#version 460 core

layout (location = 0) in vec3 aPosition;

uniform mat4 u_mvp;

out vec3 position;

void main()
{
	position = aPosition;

	gl_Position = u_mvp * vec4(aPosition, 1.0);
}
