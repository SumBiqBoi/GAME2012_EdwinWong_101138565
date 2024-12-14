#version 460 core

in vec3 position;
in vec3 normal;
in vec2 tcoord;

uniform sampler2D u_tex;

out vec4 FragColor;

uniform vec3 u_cameraPositionPoint;
uniform vec3 u_lightPositionPoint;
uniform vec3 u_lightColorPoint;
uniform float u_lightRadiusPoint;

void main()
{
    // Point Light
    vec3 N = normalize(normal);
    vec3 L = normalize(u_lightPositionPoint - position);
    vec3 V = normalize(u_cameraPositionPoint - position);
    vec3 R = normalize(reflect(L, N));
    float dotNL = max(dot(N, L), 0.0);
    float dotVR = max(dot(V, R), 0.0);

    float dist = length(u_lightPositionPoint - position);
    float attenuation = clamp(u_lightRadiusPoint / dist, 0.0, 1.0);

    vec3 lighting = vec3(0.0);
    vec3 ambient = u_lightColorPoint * 0.3;
    vec3 diffuse = u_lightColorPoint * dotNL;
    vec3 specular = u_lightColorPoint * pow(dotVR, 4);

    lighting += ambient;
    lighting += diffuse;
    lighting += specular;
    lighting *= attenuation;

    FragColor = vec4(lighting * texture(u_tex, tcoord).rgb, 1.0);
}