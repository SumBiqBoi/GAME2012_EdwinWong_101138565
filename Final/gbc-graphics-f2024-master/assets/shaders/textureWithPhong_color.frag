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

uniform vec3 u_cameraPositionSpot;
uniform vec3 u_lightPositionSpot;
uniform vec3 u_lightColorSpot;
uniform vec3 u_lightDirSpot;
uniform float u_lightRadiusSpot;

void main()
{
    // Spot Light
    vec3 LSpot = normalize(position - u_lightPositionSpot);
    vec3 spotDir = normalize(-u_lightDirSpot);
    float theta = dot(LSpot, spotDir);

    float inCutoff = cos(radians(u_lightRadiusSpot / 2.0));
    float outCutoff = cos(radians((u_lightRadiusSpot / 2.0) + 0.5));
    float epsilon = inCutoff - outCutoff;

    float intensity = clamp((theta - outCutoff) / epsilon, 0.0, 1.0);

    vec3 lightingSpot = vec3(0.0);
    vec3 ambientSpot = u_lightColorSpot * 0.3;

    lightingSpot += ambientSpot;
    lightingSpot *= intensity;

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
    vec3 ambient = u_lightColor * 0.3;
    vec3 diffuse = u_lightColor * dotNL;
    vec3 specular = u_lightColor * pow(dotVR, 4);

    lighting += ambient;
    lighting += diffuse;
    lighting += specular;
    lighting *= attenuation;
    lighting = clamp(lighting, 0.0, 1.0);

    // Direction Light
    vec3 NDir = normalize(normal);
    vec3 LDir = normalize(u_lightPositionDir - position);
    float dotNLDir = max(dot(NDir, LDir), 0.0);

    vec3 lightingDir = vec3(0.0);
    vec3 ambientDir = u_lightColor * 0.3;
    vec3 diffuseDir = u_lightColor * dotNLDir;

    lightingDir += ambientDir;
    lightingDir += diffuseDir;

    vec3 result = lighting + lightingDir + lightingSpot;
    FragColor = vec4(result * texture(u_tex, tcoord).rgb, 1.0);
}
