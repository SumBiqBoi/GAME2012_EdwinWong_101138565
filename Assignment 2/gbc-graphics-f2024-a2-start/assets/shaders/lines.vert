#version 460 core

// Position attribute is now a vec2 (see glVertexAttribPointer parameters)
layout (location = 0) in vec2 aPosition;
//layout (location = 0) in vec3 aColor;

out vec3 color;
uniform float u_a;

// No need to transform our lines, although it might make your assignment easier if you render the same square with different transformations!
//uniform mat4 u_mvp;

void main()
{
   //vec4 position = u_mvp * vec4(aPosition, 1.0);
   color = vec3(aPosition * 0.5 + 0.5, u_a);
   //color = aColor;

   gl_Position = vec4(aPosition, 0.0, 1.0);
   //gl_Position = vec4(aColor, 1.0);
}