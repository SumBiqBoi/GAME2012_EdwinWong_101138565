#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr float SCREEN_ASPECT = SCREEN_WIDTH / (float)SCREEN_HEIGHT;

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void error_callback(int error, const char* description);

GLuint CreateShader(GLint type, const char* path);
GLuint CreateProgram(GLuint vs, GLuint fs);

std::array<int, GLFW_KEY_LAST> gKeysCurr{}, gKeysPrev{};
bool IsKeyDown(int key);
bool IsKeyUp(int key);
bool IsKeyPressed(int key);

void Print(Matrix m);

enum Projection : int
{
    ORTHO,  // Orthographic, 2D
    PERSP   // Perspective,  3D
};

struct Line
{
    Vector2 start;
    Vector2 end;
};

int main(void)
{
    glfwSetErrorCallback(error_callback);
    assert(glfwInit() == GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#ifdef NDEBUG
#else
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Graphics 1", NULL, NULL);
    glfwMakeContextCurrent(window);
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
    glfwSetKeyCallback(window, key_callback);

#ifdef NDEBUG
#else
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glDebugOutput, nullptr);
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // Vertex shaders:
    GLuint vs = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/default.vert");
    GLuint vsLines = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/lines.vert");
    GLuint vsVertexPositionColor = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/vertex_color.vert");
    GLuint vsColorBufferColor = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/buffer_color.vert");
    
    // Fragment shaders:
    GLuint fsLines = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/lines.frag");
    GLuint fsUniformColor = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/uniform_color.frag");
    GLuint fsVertexColor = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/vertex_color.frag");
    GLuint fsTcoords = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/tcoord_color.frag");
    GLuint fsNormals = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/normal_color.frag");
    GLuint fsTexture = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/texture_color.frag");
    GLuint fsTextureWithPhong = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/textureWithPhong_color.frag");
    GLuint fsTextureWithPoint = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/textureWithPointLight.frag");
    GLuint fsColor = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/grey_color.frag");
    GLuint fsPhong = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/phong.frag");
    
    // Shader programs:
    GLuint shaderUniformColor = CreateProgram(vs, fsUniformColor);
    GLuint shaderVertexPositionColor = CreateProgram(vsVertexPositionColor, fsVertexColor);
    GLuint shaderVertexBufferColor = CreateProgram(vsColorBufferColor, fsVertexColor);
    GLuint shaderLines = CreateProgram(vsLines, fsLines);
    GLuint shaderTcoords = CreateProgram(vs, fsTcoords);
    GLuint shaderNormals = CreateProgram(vs, fsNormals);
    GLuint shaderTexture = CreateProgram(vs, fsTexture);
    GLuint shaderTextureWithPhong = CreateProgram(vs, fsTextureWithPhong);
    GLuint shaderTextureWithPoint = CreateProgram(vs, fsTextureWithPoint);
    GLuint shaderColor = CreateProgram(vs, fsColor);
    GLuint shaderPhong = CreateProgram(vs, fsPhong);

    // Step 1: Load image from disk to CPU
    stbi_set_flip_vertically_on_load(true);
    int tKnifeWidth = 0;
    int tKnifeHeight = 0;
    int tKnifeChannels = 0;
    stbi_uc* knifePixels = stbi_load("./assets/textures/knife_colour.jpg", &tKnifeWidth, &tKnifeHeight, &tKnifeChannels, 0);

    int tBackgroundWidth = 0;
    int tBackgroundHeight = 0;
    int tBackgroundChannels = 0;
    stbi_uc* backgroundPixels = stbi_load("./assets/textures/water_Color.jpg", &tBackgroundWidth, &tBackgroundHeight, &tBackgroundChannels, 0);

    // Step 2: Upload image from CPU to GPU
    GLuint knifeTexture = GL_NONE;
    glGenTextures(1, &knifeTexture);
    glBindTexture(GL_TEXTURE_2D, knifeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tKnifeWidth, tKnifeHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, knifePixels);
    stbi_image_free(knifePixels);
    knifePixels = nullptr;

    GLuint backgroundTexture = GL_NONE;
    glGenTextures(1, &backgroundTexture);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tBackgroundWidth, tBackgroundHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, backgroundPixels);
    stbi_image_free(backgroundPixels);
    backgroundPixels = nullptr;

    // Positions of our triangle's vertices (CCW winding-order)
    Vector3 positions[] =
    {
        0.5f, -0.5f, 0.0f,  // vertex 1 (bottom-right)
        0.0f, 0.5f, 0.0f,   // vertex 2 (top-middle)
        -0.5f, -0.5f, 0.0f  // vertex 3 (bottom-left)
    };

    // Colours of our triangle's vertices (xyz = rgb)
    Vector3 colours[] =
    {
        1.0f, 0.0f, 0.0f,   // vertex 1
        0.0f, 1.0f, 0.0f,   // vertex 2
        0.0f, 0.0f, 1.0f    // vertex 3
    };

    Vector2 curr[4]
    {
        { -0.5f,  0.5f },   // top-left
        {  0.5f,  0.5f },   // top-right
        {  0.5f, -0.5f },   // bot-right
        { -0.5f, -0.5f }    // bot-left
    };

    Vector2 next[4]
    {
        (curr[0] + curr[1]) * 0.5f,
        (curr[1] + curr[2]) * 0.5f,
        (curr[2] + curr[3]) * 0.5f,
        (curr[3] + curr[0]) * 0.5f
    };

    // vao = "Vertex Array Object". A vao is a collection of vbos.
    // vbo = "Vertex Buffer Object". "Buffer" generally means "group of memory".
    // A vbo is a piece of graphics memory VRAM.
    GLuint vao, pbo, cbo;       // pbo = "position buffer object", "cbo = color buffer object"
    glGenVertexArrays(1, &vao); // Allocate a vao handle
    glBindVertexArray(vao);     // Bind = "associate all bound buffer object with the current array object"
    
    // Create position buffer:
    glGenBuffers(1, &pbo);              // Allocate a vbo handle
    glBindBuffer(GL_ARRAY_BUFFER, pbo); // Associate this buffer with the bound vertex array
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(Vector3), positions, GL_STATIC_DRAW);  // Upload the buffer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), 0);          // Describe the buffer
    glEnableVertexAttribArray(0);

    // Create color buffer:
    glGenBuffers(1, &cbo);              // Allocate a vbo handle
    glBindBuffer(GL_ARRAY_BUFFER, cbo); // Associate this buffer with the bound vertex array
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(Vector3), colours, GL_STATIC_DRAW);    // Upload the buffer
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), nullptr);      // Describe the buffer
    glEnableVertexAttribArray(1);

    GLuint vaoLines, pboLines, cboLines; // Note that cboLines is currently unused.
    glGenVertexArrays(1, &vaoLines);
    glBindVertexArray(vaoLines);

    glGenBuffers(1, &pboLines);
    glBindBuffer(GL_ARRAY_BUFFER, pboLines);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector2), curr, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(GL_NONE);

    GLint u_color = glGetUniformLocation(shaderUniformColor, "u_color");
    GLint u_intensity = glGetUniformLocation(shaderUniformColor, "u_intensity");

    int object = 0;
    printf("Object %i\n", object + 1);

    Projection projection = PERSP;
    Vector3 cameraPos{ 0.0f, 0.0f, 3.0f }; // Camera position
    Vector3 cameraTarget = { 0.0f, 0.0f,0.0f };
    Vector3 cameraDir = Normalize(cameraPos - cameraTarget);
    Quaternion camRot = QuaternionIdentity();
    float fov = 75.0f * DEG2RAD;
    float left = -1.0f;
    float right = 1.0f;
    float top = 1.0f;
    float bottom = -1.0f;
    float near = 0.01f; // 1.0 for testing purposes. Usually 0.1f or 0.01f
    float far = 10.0f;

    // Whether we render the imgui demo widgets
    bool imguiDemo = false;
    bool camToggle = false;

    Mesh planeMesh, sphereMesh, knifeMesh;
    CreateMesh(&planeMesh, PLANE);
    CreateMesh(&sphereMesh, "assets/meshes/uvsphere.obj");
    CreateMesh(&knifeMesh, "assets/meshes/knife.obj");

    float camPitch = 0;
    float camYaw = 0;
    float camSpeed = 10.0f;

    Vector3 lightPositionDir = { 10.0f,10.0f,10.0f };
    Vector3 lightColor = { 1.0f, 1.0f, 1.0f };

    Vector3 lightPositionOrbit = { 0.0, 0.0, 0.0 };
    float lightRadius = 10.0f;

    Vector3 lightPositionSpot = { 0.0f, 3.0f, 0.0f };
    Vector3 lightColorSpot = { 1.0f, 1.0f, 1.0f };
    Vector3 lightDirSpot = { 0.0f, 1.0f, 0.0f };
    float lightRadiusSpot = 2.0;

    // Render looks weird cause this isn't enabled, but its causing unexpected problems which I'll fix soon!
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    float timePrev = glfwGetTime();
    float timeCurr = glfwGetTime();
    float dt = 0.0f;

    double pmx = 0.0, pmy = 0.0, mx = 0.0, my = 0.0;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        float time = glfwGetTime();
        timePrev = time;
        camRot = FromEuler(-camPitch * DEG2RAD, -camYaw * DEG2RAD, 0.0f);
        Matrix camRotation = ToMatrix(FromEuler(camPitch * DEG2RAD, camYaw * DEG2RAD, 0.0f));
        Matrix camTranslation = Translate(cameraPos);
        //printf("Cam Rotation: \n", camRotation);
        Vector3 camRight = { camRotation.m0, camRotation.m1, camRotation.m2 };
        Vector3 camUp = { camRotation.m4, camRotation.m5, camRotation.m6 };
        Vector3 camForward = { camRotation.m8, camRotation.m9,camRotation.m10 };
        float camDelta = camSpeed * dt;
        Matrix camMatrix = camRotation * camTranslation;

        pmx = mx; pmy = my;
        glfwGetCursorPos(window, &mx, &my);
        Vector2 mouseDelta = { mx - pmx, my - pmy };
        float mouseScale = 1.0f;

        if (!camToggle)
        {
            camDelta = 0.0f;
            mouseScale = 0.0f;
        }
        camPitch += mouseDelta.y * mouseScale;
        camYaw += mouseDelta.x * mouseScale;


        //printf("x: %f, y: %f, cam Pitch: %f\n", mouseDelta.x, mouseDelta.y, camPitch);

        // Change object when space is pressed
        if (IsKeyPressed(GLFW_KEY_F))
        {
            ++object %= 5;
            printf("Object %i\n", object + 1);
        }

        if (IsKeyDown(GLFW_KEY_W))
        {
            cameraPos -= camForward * camDelta;
        }
        if (IsKeyDown(GLFW_KEY_S))
        {
            cameraPos += camForward * camDelta;
        }
        if (IsKeyDown(GLFW_KEY_A))
        {
            cameraPos -= camRight * camDelta;
        }
        if (IsKeyDown(GLFW_KEY_D))
        {
            cameraPos += camRight * camDelta;
        }
        if (IsKeyDown(GLFW_KEY_SPACE))
        {
            cameraPos += camUp * camDelta;
        }
        if (IsKeyDown(GLFW_KEY_LEFT_SHIFT))
        {
            cameraPos -= camUp * camDelta;
        }
        if (IsKeyPressed(GLFW_KEY_I))
            imguiDemo = !imguiDemo;

        if (IsKeyPressed(GLFW_KEY_C))
        {
            camToggle = !camToggle;
            if (camToggle)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Interpolation parameter (0 means fully A, 1 means fully B)
        float a = cosf(time) * 0.5f + 0.5f;

        // Interpolate scale
        Vector3 sA = V3_ONE;
        Vector3 sB = V3_ONE * 10.0f;
        Vector3 sC = Lerp(sA, sB, a);

        // Interpolate rotation (slerp = "spherical lerp" because we rotate in a circle) 
        Quaternion rA = QuaternionIdentity();
        Quaternion rB = FromEuler(0.0f, 0.0f, 90.0f * DEG2RAD);
        Quaternion rC = Slerp(rA, rB, a);

        // Interpolate translation
        Vector3 tA = { -10.0f, 0.0f, 0.0f };
        Vector3 tB = {  10.0f, 0.0f, 0.0f };
        Vector3 tC = Lerp(tA, tB, a);

        // Interpolate color
        Vector3 cA = V3_UP;
        Vector3 cB = V3_FORWARD;
        Vector3 cC = Lerp(cA, cB, a);

        Matrix s = Scale(sC);
        Matrix r = ToMatrix(rC);
        Matrix t = Translate(tC);

        Matrix world = MatrixIdentity();
        Matrix normal = MatrixIdentity();
        Matrix view = LookAt(cameraPos, cameraPos - Rotate(cameraDir, camRot), V3_UP);
        Matrix proj = projection == ORTHO ? Ortho(left, right, bottom, top, near, far) : Perspective(fov, SCREEN_ASPECT, near, far);
        Matrix mvp = MatrixIdentity();
        GLint u_world = -2;
        GLint u_normal = -2;
        GLint u_mvp = -2;
        GLint u_tex = -2;
        GLuint shaderProgram = GL_NONE;

        // Light handle
        GLint u_cameraPositionPoint = -2;
        GLint u_lightPositionPoint = -2;
        GLint u_lightColorPoint = -2;
        GLint u_lightRadiusPoint = -2;
        GLint u_lightPositionDir = -2;
        GLint u_cameraPositionSpot = -2;
        GLint u_lightPositionSpot = -2;
        GLint u_lightColorSpot = -2;
        GLint u_lightDirSpot = -2;
        GLint u_lightRadiusSpot = -2;
        
        switch (object + 1)
        {
        case 1:
        {
            Vector3 tcoordsSpherePosition = { 1.5 * sin(time), 0.0, 1.5 * cos(time) };
            tcoordsSpherePosition += lightPositionOrbit;

            Vector3 normalSpherePosition = { 1.5 * sin(time + -7.33), 0.0, 1.5 * cos(time + -7.33) };
            normalSpherePosition += lightPositionOrbit;

            Vector3 pointLightSpherePosition = { 1.5 * sin(time + -14.66), 0.0, 1.5 * cos(time + -14.66) };
            pointLightSpherePosition += lightPositionOrbit;

            Vector3 spotLightSpherePosition = { 1.5 * sin(time + -36.65), 0.0, 1.5 * cos(time + -36.65) };
            spotLightSpherePosition += lightPositionOrbit;

            Vector3 refractionSpherePosition = { 1.5 * sin(time + -29.32), 0.0, 1.5 * cos(time + -29.32) };
            refractionSpherePosition += lightPositionOrbit;

            Vector3 reflectionSpherePosition = { 1.5 * sin(time + -21.99), 0.0, 1.5 * cos(time + -21.99) };
            reflectionSpherePosition += lightPositionOrbit;

            shaderProgram = shaderTexture;
            glUseProgram(shaderProgram);
            world = Translate(0.0f, 0.0f, 0.0f);
            mvp = world * view * proj;
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_tex = glGetUniformLocation(shaderProgram, "u_tex");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform1i(u_tex, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, backgroundTexture);
            DrawMesh(sphereMesh);

            shaderProgram = shaderTcoords;
            glUseProgram(shaderProgram);
            world = Scale(V3_ONE) * Translate(tcoordsSpherePosition);
            mvp = world * view * proj;
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            DrawMesh(sphereMesh);
            
            shaderProgram = shaderNormals;
            glUseProgram(shaderProgram);
            world = Scale(V3_ONE) * Translate(normalSpherePosition) * RotateZ(30 * DEG2RAD);
            normal = NormalMatrix(world);
            mvp = world * view * proj;
            u_normal = glGetUniformLocation(shaderProgram, "u_normal");
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            glUniformMatrix3fv(u_normal, 1, GL_FALSE, ToFloat9(normal).v);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            DrawMesh(sphereMesh);
            
            Vector3 lightPosition = pointLightSpherePosition;

            shaderProgram = shaderTextureWithPoint;
            glUseProgram(shaderProgram);
            world = Scale(V3_ONE) * Translate(pointLightSpherePosition) * RotateZ(60 * DEG2RAD);
            normal = NormalMatrix(world);
            mvp = world * view * proj;
            u_world = glGetUniformLocation(shaderProgram, "u_world");
            u_normal = glGetUniformLocation(shaderProgram, "u_normal");
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_tex = glGetUniformLocation(shaderProgram, "u_tex");
            u_cameraPositionPoint = glGetUniformLocation(shaderProgram, "u_cameraPositionPoint");
            u_lightPositionPoint = glGetUniformLocation(shaderProgram, "u_lightPositionPoint");
            u_lightColorPoint = glGetUniformLocation(shaderProgram, "u_lightColorPoint");
            u_lightRadiusPoint = glGetUniformLocation(shaderProgram, "u_lightRadiusPoint");
            glUniformMatrix4fv(u_world, 1, GL_FALSE, ToFloat16(world).v);
            glUniformMatrix3fv(u_normal, 1, GL_FALSE, ToFloat9(normal).v);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform3fv(u_lightPositionPoint, 1, &cameraPos.x);
            glUniform3fv(u_lightPositionPoint, 1, &lightPosition.x);
            glUniform3fv(u_lightColorPoint, 1, &lightColor.x);
            glUniform1f(u_lightRadiusPoint, lightRadius);
            glUniform1i(u_tex, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, backgroundTexture);
            DrawMesh(sphereMesh);
            
            shaderProgram = shaderTexture;
            glUseProgram(shaderProgram);
            world = Scale(V3_ONE) * Translate(spotLightSpherePosition) * RotateZ(90 * DEG2RAD);
            mvp = world * view * proj;
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_tex = glGetUniformLocation(shaderProgram, "u_tex");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform1i(u_tex, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, backgroundTexture);
            DrawMesh(sphereMesh);
            
            shaderProgram = shaderTexture;
            glUseProgram(shaderProgram);
            world = Scale(V3_ONE) * Translate(refractionSpherePosition) * RotateZ(120 * DEG2RAD);
            mvp = world * view * proj;
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_tex = glGetUniformLocation(shaderProgram, "u_tex");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform1i(u_tex, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, backgroundTexture);
            DrawMesh(sphereMesh);
            
            shaderProgram = shaderTexture;
            glUseProgram(shaderProgram);
            world = Scale(V3_ONE) * Translate(reflectionSpherePosition) * RotateZ(150 * DEG2RAD);
            mvp = world * view * proj;
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_tex = glGetUniformLocation(shaderProgram, "u_tex");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform1i(u_tex, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, backgroundTexture);
            DrawMesh(sphereMesh);

            break;
        }
        case 2:
            break;

        case 3:
            break;

        case 4:
            break;

        case 5:
            float rotationAmount = 90.0f * DEG2RAD;
            float planeValues = 20.0f;
            Vector3 lightPosition = { 2.0 * sin(time), 0.0, 2.0 * cos(time) };
            lightPosition += lightPositionOrbit;

            shaderProgram = shaderUniformColor;
            glUseProgram(shaderProgram);
            world = Scale(V3_ONE) * Translate(lightPositionDir);
            mvp = world * view * proj;
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_color = glGetUniformLocation(shaderProgram, "u_color");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform3fv(u_color, 1, &lightColor.x);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            DrawMesh(sphereMesh); // Draw directional light source
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            shaderProgram = shaderUniformColor;
            glUseProgram(shaderProgram);
            world = Scale(V3_ONE * lightRadius) * Translate(lightPosition);
            mvp = world * view * proj;
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_color = glGetUniformLocation(shaderProgram, "u_color");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform3fv(u_color, 1, &lightColor.x);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            DrawMesh(sphereMesh); // Draw orbit light source
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            shaderProgram = shaderUniformColor;
            glUseProgram(shaderProgram);
            world = Scale(V3_ONE) * Translate(lightPositionSpot);
            mvp = world * view * proj;
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_color = glGetUniformLocation(shaderProgram, "u_color");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform3fv(u_color, 1, &lightColor.x);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            DrawMesh(sphereMesh); // Draw directional light source
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            shaderProgram = shaderTextureWithPhong;
            glUseProgram(shaderProgram);
            world = Translate(0.0f, 0.0f, 0.0f);
            mvp = world * view * proj;
            normal = Transpose(Invert(world));
            u_world = glGetUniformLocation(shaderProgram, "u_world");
            u_normal = glGetUniformLocation(shaderProgram, "u_normal");
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_tex = glGetUniformLocation(shaderProgram, "u_tex");
            u_cameraPositionPoint = glGetUniformLocation(shaderProgram, "u_cameraPosition");
            u_lightPositionPoint = glGetUniformLocation(shaderProgram, "u_lightPosition");
            u_lightColorPoint = glGetUniformLocation(shaderProgram, "u_lightColor");
            u_lightRadiusPoint = glGetUniformLocation(shaderProgram, "u_lightRadius");
            u_lightPositionDir = glGetUniformLocation(shaderProgram, "u_lightPositionDir");
            u_cameraPositionSpot = glGetUniformLocation(shaderProgram, "u_cameraPositionSpot");
            u_lightPositionSpot = glGetUniformLocation(shaderProgram, "u_lightPositionSpot");
            u_lightColorSpot = glGetUniformLocation(shaderProgram, "u_lightColorSpot");
            u_lightRadiusSpot = glGetUniformLocation(shaderProgram, "u_lightRadiusSpot");
            u_lightDirSpot = glGetUniformLocation(shaderProgram, "u_lightDirSpot");
            glUniformMatrix4fv(u_world, 1, GL_FALSE, ToFloat16(world).v);
            glUniformMatrix3fv(u_normal, 1, GL_FALSE, ToFloat9(normal).v);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform3fv(u_lightPositionPoint, 1, &cameraPos.x);
            glUniform3fv(u_lightPositionPoint, 1, &lightPosition.x);
            glUniform3fv(u_lightColorPoint, 1, &lightColor.x);
            glUniform1f(u_lightRadiusPoint, lightRadius);
            glUniform3fv(u_lightPositionDir, 1, &lightPositionDir.x);
            glUniform3fv(u_lightPositionSpot, 1, &cameraPos.x);
            glUniform3fv(u_lightPositionSpot, 1, &lightPositionSpot.x);
            glUniform3fv(u_lightColorSpot, 1, &lightColorSpot.x);
            glUniform3fv(u_lightDirSpot, 1, &lightDirSpot.x);
            glUniform1f(u_lightRadiusSpot, u_lightRadiusSpot);
            glUniform1i(u_tex, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, knifeTexture);
            DrawMesh(knifeMesh); // Draw knife mesh

            shaderProgram = shaderColor;
            glUseProgram(shaderProgram);
            world = Scale(planeValues, planeValues, planeValues) * RotateX(rotationAmount) * Translate(-planeValues / 2, -1.5f, -planeValues / 2);
            normal = NormalMatrix(world);
            mvp = world * view * proj;
            u_world = glGetUniformLocation(shaderProgram, "u_world");
            u_normal = glGetUniformLocation(shaderProgram, "u_normal");
            u_mvp = glGetUniformLocation(shaderProgram, "u_mvp");
            u_cameraPositionPoint = glGetUniformLocation(shaderProgram, "u_cameraPosition");
            u_lightPositionPoint = glGetUniformLocation(shaderProgram, "u_lightPosition");
            u_lightColorPoint = glGetUniformLocation(shaderProgram, "u_lightColor");
            u_lightRadiusPoint = glGetUniformLocation(shaderProgram, "u_lightRadius");
            //u_lightPositionDir = glGetUniformLocation(shaderProgram, "u_lightPositionDir");
            //u_cameraPositionSpot = glGetUniformLocation(shaderProgram, "u_cameraPositionSpot");
            //u_lightPositionSpot = glGetUniformLocation(shaderProgram, "u_lightPositionSpot");
            //u_lightColorSpot = glGetUniformLocation(shaderProgram, "u_lightColorSpot");
            //u_lightRadiusSpot = glGetUniformLocation(shaderProgram, "u_lightRadiusSpot");
            //u_lightDirSpot = glGetUniformLocation(shaderProgram, "u_lightDirSpot");
            glUniformMatrix4fv(u_world, 1, GL_FALSE, ToFloat16(world).v);
            glUniformMatrix3fv(u_normal, 1, GL_FALSE, ToFloat9(normal).v);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            glUniform3fv(u_lightPositionPoint, 1, &cameraPos.x);
            glUniform3fv(u_lightPositionPoint, 1, &lightPosition.x);
            glUniform3fv(u_lightColorPoint, 1, &lightColor.x);
            glUniform1f(u_lightRadiusPoint, lightRadius);
            //glUniform3fv(u_lightPositionDir, 1, &lightPositionDir.x);
            //glUniform3fv(u_lightPositionSpot, 1, &cameraPos.x);
            //glUniform3fv(u_lightPositionSpot, 1, &lightPositionSpot.x);
            //glUniform3fv(u_lightColorSpot, 1, &lightColorSpot.x);
            //glUniform3fv(u_lightDirSpot, 1, &lightDirSpot.x);
            //glUniform1f(u_lightRadiusSpot, lightRadiusSpot);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, ToFloat16(mvp).v);
            DrawMesh(planeMesh); // Draw plane
            break;
        }
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (imguiDemo)
            ImGui::ShowDemoWindow();
        else
        {
            ImGui::SliderFloat3("Camera Position", &cameraPos.x, -10.0f, 10.0f);
            ImGui::SliderFloat3("Direction Light Position", &lightPositionDir.x, -10.0f, 10.0f);
            ImGui::SliderFloat3("Orbit Light Position", &lightPositionOrbit.x, -10.0f, 10.0f);
            ImGui::SliderFloat("Orbit Light Radius", &lightRadius, 0.25f, 100.0f);
            ImGui::SliderFloat3("Spot Light Position", &lightPositionSpot.x, -10.0f, 10.0f);
            ImGui::SliderFloat("Spot Light Radius", &lightRadiusSpot, 0.25f, 10.0f);

            ImGui::RadioButton("Orthographic", (int*)&projection, 0); ImGui::SameLine();
            ImGui::RadioButton("Perspective", (int*)&projection, 1);

            ImGui::SliderFloat("Near", &near, -10.0f, 10.0f);
            ImGui::SliderFloat("Far", &far, -10.0f, 10.0f);
            if (projection == ORTHO)
            {
                ImGui::SliderFloat("Left", &left, -1.0f, -10.0f);
                ImGui::SliderFloat("Right", &right, 1.0f, 10.0f);
                ImGui::SliderFloat("Top", &top, 1.0f, 10.0f);
                ImGui::SliderFloat("Bottom", &bottom, -1.0f, -10.0f);
            }
            else if (projection == PERSP)
            {
                ImGui::SliderAngle("FoV", &fov, 10.0f, 90.0f);
            }
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        timeCurr = glfwGetTime();
        dt = timeCurr - timePrev;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll and process events */
        memcpy(gKeysPrev.data(), gKeysCurr.data(), GLFW_KEY_LAST * sizeof(int));
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Disable repeat events so keys are either up or down
    if (action == GLFW_REPEAT) return;

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    gKeysCurr[key] = action;

    // Key logging
    //const char* name = glfwGetKeyName(key, scancode);
    //if (action == GLFW_PRESS)
    //    printf("%s\n", name);
}

void error_callback(int error, const char* description)
{
    printf("GLFW Error %d: %s\n", error, description);
}

// Compile a shader
GLuint CreateShader(GLint type, const char* path)
{
    GLuint shader = GL_NONE;
    try
    {
        // Load text file
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(path);

        // Interpret the file as a giant string
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();

        // Verify shader type matches shader file extension
        const char* ext = strrchr(path, '.');
        switch (type)
        {
        case GL_VERTEX_SHADER:
            assert(strcmp(ext, ".vert") == 0);
            break;

        case GL_FRAGMENT_SHADER:
            assert(strcmp(ext, ".frag") == 0);
            break;
        default:
            assert(false, "Invalid shader type");
            break;
        }

        // Compile text as a shader
        std::string str = stream.str();
        const char* src = str.c_str();
        shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);

        // Check for compilation errors
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "Shader failed to compile: \n" << infoLog << std::endl;
        }
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "Shader (" << path << ") not found: " << e.what() << std::endl;
        assert(false);
    }

    return shader;
}

// Combine two compiled shaders into a program that can run on the GPU
GLuint CreateProgram(GLuint vs, GLuint fs)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        program = GL_NONE;
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    return program;
}

// Graphics debug callback
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

bool IsKeyDown(int key)
{
    return gKeysCurr[key] == GLFW_PRESS;
}

bool IsKeyUp(int key)
{
    return gKeysCurr[key] == GLFW_RELEASE;
}

bool IsKeyPressed(int key)
{
    return gKeysPrev[key] == GLFW_PRESS && gKeysCurr[key] == GLFW_RELEASE;
}

void Print(Matrix m)
{
    printf("%f %f %f %f\n", m.m0, m.m4, m.m8, m.m12);
    printf("%f %f %f %f\n", m.m1, m.m5, m.m9, m.m13);
    printf("%f %f %f %f\n", m.m2, m.m6, m.m10, m.m14);
    printf("%f %f %f %f\n\n", m.m3, m.m7, m.m11, m.m15);
}
