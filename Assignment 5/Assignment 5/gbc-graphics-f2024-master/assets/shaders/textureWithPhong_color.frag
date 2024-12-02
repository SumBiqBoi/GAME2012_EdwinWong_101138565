#version 460 core

in vec3 position;
in vec3 normal;
in vec2 tcoord;

uniform sampler2D u_tex;

out vec4 FragColor;

uniform vec3 u_cameraPosition;
uniform vec3 u_lightPosition;
uniform vec3 u_lightColor;
uniform float u_lightRadius;

uniform vec3 u_lightPositionDir;
uniform float u_lightRadiusDir;

void main()
{
    // Orbit Light
    vec3 N = normalize(normal);
    vec3 L = normalize(u_lightPosition - position);
    vec3 V = normalize(u_cameraPosition - position);
    vec3 R = normalize(reflect(L, N));
    float dotNL = max(dot(N, L), 0.0);
    float dotVR = max(dot(V, R), 0.0);

    float dist = length(u_lightPosition - position);
    float attenuation = clamp(u_lightRadius / dist, 0.0, 1.0);

    vec3 lighting = vec3(0.0);
    vec3 ambient = u_lightColor * 0.1;
    vec3 diffuse = u_lightColor * dotNL;
    vec3 specular = u_lightColor * pow(dotVR, 64);

    lighting += ambient;
    lighting += diffuse;
    lighting += specular;
    lighting *= attenuation;

    // Direction Light
    vec3 NDir = normalize(normal);
    vec3 LDir = normalize(u_lightPositionDir - position);
    float dotNLDir = max(dot(NDir, LDir), 0.0);

    float distDir = length(u_lightPositionDir - position);
    float attenuationDir = clamp(u_lightRadiusDir / distDir, 0.0, 1.0);

    vec3 lightingDir = vec3(0.0);
    vec3 ambientDir = u_lightColor * 0.1;
    vec3 diffuseDir = u_lightColor * dotNLDir;

    lightingDir += ambientDir;
    lightingDir += diffuseDir;
    lightingDir *= attenuationDir;

    vec3 result = (lighting + lightingDir) * texture(u_tex, tcoord).rgb;
    FragColor = vec4(result, 1.0);
}
