#pragma once
// Mana Graphics Runtime - GLFW/OpenGL bindings
// This header provides the FFI bridge between Mana and native graphics APIs

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdint>

// Wrapper functions with mana_ prefix that handle pointer<->int64 casting
// These are called from the generated Mana code

inline int mana_glfwInit() {
    return glfwInit();
}

inline void mana_glfwTerminate() {
    glfwTerminate();
}

inline int64_t mana_glfwCreateWindow(int width, int height, const std::string& title, int64_t monitor, int64_t share) {
    GLFWwindow* win = glfwCreateWindow(width, height, title.c_str(),
                                        reinterpret_cast<GLFWmonitor*>(monitor),
                                        reinterpret_cast<GLFWwindow*>(share));
    return reinterpret_cast<int64_t>(win);
}

inline void mana_glfwDestroyWindow(int64_t window) {
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(window));
}

inline void mana_glfwMakeContextCurrent(int64_t window) {
    glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(window));
}

inline int mana_glfwWindowShouldClose(int64_t window) {
    return glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(window));
}

inline void mana_glfwSwapBuffers(int64_t window) {
    glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(window));
}

inline void mana_glfwPollEvents() {
    glfwPollEvents();
}

inline double mana_glfwGetTime() {
    return glfwGetTime();
}

inline void mana_glfwSwapInterval(int interval) {
    glfwSwapInterval(interval);
}

// GLAD loader wrapper - uses GLFW's proc address function
inline int mana_gladLoadGL() {
    return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

// OpenGL wrappers
inline void mana_glClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

inline void mana_glClear(int mask) {
    glClear(mask);
}

inline void mana_glViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

// ============================================================================
// Shader functions
// ============================================================================

inline int32_t mana_glCreateShader(int32_t type) {
    return glCreateShader(type);
}

inline void mana_glShaderSource(int32_t shader, const std::string& source) {
    const char* sources[] = { source.c_str() };
    glShaderSource(shader, 1, sources, nullptr);
}

inline void mana_glCompileShader(int32_t shader) {
    glCompileShader(shader);
}

inline int32_t mana_glGetShaderiv(int32_t shader, int32_t pname) {
    GLint result;
    glGetShaderiv(shader, pname, &result);
    return result;
}

inline void mana_glGetShaderInfoLog(int32_t shader) {
    char log[512];
    glGetShaderInfoLog(shader, 512, nullptr, log);
    printf("Shader error: %s\n", log);
}

inline void mana_glDeleteShader(int32_t shader) {
    glDeleteShader(shader);
}

inline int32_t mana_glCreateProgram() {
    return glCreateProgram();
}

inline void mana_glAttachShader(int32_t program, int32_t shader) {
    glAttachShader(program, shader);
}

inline void mana_glLinkProgram(int32_t program) {
    glLinkProgram(program);
}

inline int32_t mana_glGetProgramiv(int32_t program, int32_t pname) {
    GLint result;
    glGetProgramiv(program, pname, &result);
    return result;
}

inline void mana_glGetProgramInfoLog(int32_t program) {
    char log[512];
    glGetProgramInfoLog(program, 512, nullptr, log);
    printf("Program error: %s\n", log);
}

inline void mana_glUseProgram(int32_t program) {
    glUseProgram(program);
}

inline void mana_glDeleteProgram(int32_t program) {
    glDeleteProgram(program);
}

// ============================================================================
// Buffer functions
// ============================================================================

// We use a simple approach: generate one buffer/VAO at a time and return the ID
inline int32_t mana_glGenBuffer() {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    return static_cast<int32_t>(buffer);
}

inline int32_t mana_glGenVertexArray() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    return static_cast<int32_t>(vao);
}

inline void mana_glBindBuffer(int32_t target, int32_t buffer) {
    glBindBuffer(target, buffer);
}

inline void mana_glBindVertexArray(int32_t vao) {
    glBindVertexArray(vao);
}

// Buffer data from a float array - we pass pointer and size
inline void mana_glBufferDataFloats(int32_t target, int64_t dataPtr, int32_t count, int32_t usage) {
    const float* data = reinterpret_cast<const float*>(dataPtr);
    glBufferData(target, count * sizeof(float), data, usage);
}

inline void mana_glVertexAttribPointer(int32_t index, int32_t size, int32_t type, int32_t normalized, int32_t stride, int32_t offset) {
    glVertexAttribPointer(index, size, type, normalized, stride, reinterpret_cast<void*>(static_cast<intptr_t>(offset)));
}

inline void mana_glEnableVertexAttribArray(int32_t index) {
    glEnableVertexAttribArray(index);
}

inline void mana_glDisableVertexAttribArray(int32_t index) {
    glDisableVertexAttribArray(index);
}

inline void mana_glDrawArrays(int32_t mode, int32_t first, int32_t count) {
    glDrawArrays(mode, first, count);
}

inline void mana_glDeleteBuffer(int32_t buffer) {
    GLuint buf = static_cast<GLuint>(buffer);
    glDeleteBuffers(1, &buf);
}

inline void mana_glDeleteVertexArray(int32_t vao) {
    GLuint v = static_cast<GLuint>(vao);
    glDeleteVertexArrays(1, &v);
}

// ============================================================================
// Uniform functions
// ============================================================================

inline int32_t mana_glGetUniformLocation(int32_t program, const char* name) {
    return glGetUniformLocation(program, name);
}

inline void mana_glUniform1f(int32_t location, float v) {
    glUniform1f(location, v);
}

inline void mana_glUniform3f(int32_t location, float x, float y, float z) {
    glUniform3f(location, x, y, z);
}

inline void mana_glUniform4f(int32_t location, float x, float y, float z, float w) {
    glUniform4f(location, x, y, z, w);
}

// Matrix uniform - takes pointer to 16 floats
inline void mana_glUniformMatrix4fv(int32_t location, int64_t matrixPtr) {
    const float* matrix = reinterpret_cast<const float*>(matrixPtr);
    glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
}

// ============================================================================
// 3D Setup functions
// ============================================================================

inline void mana_glEnable(int32_t cap) {
    glEnable(cap);
}

inline void mana_glDisable(int32_t cap) {
    glDisable(cap);
}

inline int32_t mana_GL_DEPTH_TEST() { return GL_DEPTH_TEST; }
inline int32_t mana_GL_CULL_FACE() { return GL_CULL_FACE; }

// ============================================================================
// OpenGL constants (exposed as functions to avoid macro conflicts)
// ============================================================================

inline int32_t mana_GL_VERTEX_SHADER() { return GL_VERTEX_SHADER; }
inline int32_t mana_GL_FRAGMENT_SHADER() { return GL_FRAGMENT_SHADER; }
inline int32_t mana_GL_COMPILE_STATUS() { return GL_COMPILE_STATUS; }
inline int32_t mana_GL_LINK_STATUS() { return GL_LINK_STATUS; }
inline int32_t mana_GL_ARRAY_BUFFER() { return GL_ARRAY_BUFFER; }
inline int32_t mana_GL_STATIC_DRAW() { return GL_STATIC_DRAW; }
inline int32_t mana_GL_FLOAT() { return GL_FLOAT; }
inline int32_t mana_GL_FALSE() { return GL_FALSE; }
inline int32_t mana_GL_TRUE() { return GL_TRUE; }
inline int32_t mana_GL_TRIANGLES() { return GL_TRIANGLES; }
inline int32_t mana_GL_COLOR_BUFFER_BIT() { return GL_COLOR_BUFFER_BIT; }
inline int32_t mana_GL_DEPTH_BUFFER_BIT() { return GL_DEPTH_BUFFER_BIT; }

// ============================================================================
// Helper: Create a colored triangle (position + color interleaved)
// Returns VBO ID, fills with 3 vertices: (x,y,z, r,g,b) * 3
// ============================================================================
inline int32_t mana_createTriangleVBO() {
    float vertices[] = {
        // Position (x,y,z)    Color (r,g,b)
        -0.5f, -0.5f, 0.0f,    1.0f, 0.0f, 0.0f,  // Bottom-left (red)
         0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,  // Bottom-right (green)
         0.0f,  0.5f, 0.0f,    0.0f, 0.0f, 1.0f   // Top (blue)
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    return static_cast<int32_t>(vbo);
}

// Setup vertex attributes for interleaved position+color (6 floats per vertex)
inline void mana_setupTriangleAttributes() {
    // Position attribute (location 0): 3 floats, stride 24 bytes, offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (location 1): 3 floats, stride 24 bytes, offset 12 bytes
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// ============================================================================
// 3D Math - Matrix operations stored in static arrays
// ============================================================================

#include <cmath>

// Static storage for matrices (simple approach for FFI)
static float g_model_matrix[16];
static float g_view_matrix[16];
static float g_projection_matrix[16];
static float g_mvp_matrix[16];

// Identity matrix
inline void mana_mat4_identity(float* m) {
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// Matrix multiplication: result = a * b
inline void mana_mat4_multiply(float* result, const float* a, const float* b) {
    float temp[16];
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            temp[col * 4 + row] = 0.0f;
            for (int k = 0; k < 4; k++) {
                temp[col * 4 + row] += a[k * 4 + row] * b[col * 4 + k];
            }
        }
    }
    for (int i = 0; i < 16; i++) result[i] = temp[i];
}

// Perspective projection matrix
inline void mana_mat4_perspective(float* m, float fovY, float aspect, float nearZ, float farZ) {
    float tanHalfFov = tanf(fovY * 0.5f);
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0] = 1.0f / (aspect * tanHalfFov);
    m[5] = 1.0f / tanHalfFov;
    m[10] = -(farZ + nearZ) / (farZ - nearZ);
    m[11] = -1.0f;
    m[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
}

// Rotation around X axis
inline void mana_mat4_rotateX(float* m, float angle) {
    mana_mat4_identity(m);
    float c = cosf(angle);
    float s = sinf(angle);
    m[5] = c;  m[6] = s;
    m[9] = -s; m[10] = c;
}

// Rotation around Y axis
inline void mana_mat4_rotateY(float* m, float angle) {
    mana_mat4_identity(m);
    float c = cosf(angle);
    float s = sinf(angle);
    m[0] = c;  m[2] = -s;
    m[8] = s;  m[10] = c;
}

// Rotation around Z axis
inline void mana_mat4_rotateZ(float* m, float angle) {
    mana_mat4_identity(m);
    float c = cosf(angle);
    float s = sinf(angle);
    m[0] = c;  m[1] = s;
    m[4] = -s; m[5] = c;
}

// Translation matrix
inline void mana_mat4_translate(float* m, float x, float y, float z) {
    mana_mat4_identity(m);
    m[12] = x;
    m[13] = y;
    m[14] = z;
}

// ============================================================================
// High-level 3D helpers for Mana
// ============================================================================

// Set projection matrix (perspective)
inline void mana_setPerspective(float fovDegrees, float aspect, float nearZ, float farZ) {
    float fovRadians = fovDegrees * 3.14159265f / 180.0f;
    mana_mat4_perspective(g_projection_matrix, fovRadians, aspect, nearZ, farZ);
}

// Set view matrix (camera at position looking at origin)
inline void mana_setCamera(float eyeX, float eyeY, float eyeZ) {
    // Simple lookAt: camera at (eyeX, eyeY, eyeZ) looking at origin
    float temp[16];
    mana_mat4_translate(temp, -eyeX, -eyeY, -eyeZ);
    for (int i = 0; i < 16; i++) g_view_matrix[i] = temp[i];
}

// Set model rotation (Euler angles in radians)
inline void mana_setRotation(float rx, float ry, float rz) {
    float rotX[16], rotY[16], rotZ[16], temp[16];
    mana_mat4_rotateX(rotX, rx);
    mana_mat4_rotateY(rotY, ry);
    mana_mat4_rotateZ(rotZ, rz);
    mana_mat4_multiply(temp, rotY, rotX);
    mana_mat4_multiply(g_model_matrix, rotZ, temp);
}

// Compute MVP matrix and return pointer
inline int64_t mana_getMVP() {
    float temp[16];
    mana_mat4_multiply(temp, g_view_matrix, g_model_matrix);
    mana_mat4_multiply(g_mvp_matrix, g_projection_matrix, temp);
    return reinterpret_cast<int64_t>(g_mvp_matrix);
}

// ============================================================================
// Cube helper
// ============================================================================

inline int32_t mana_createCubeVBO() {
    // Cube vertices: position (x,y,z) + color (r,g,b) per vertex
    // 6 faces * 2 triangles * 3 vertices = 36 vertices
    float vertices[] = {
        // Front face (red)
        -0.5f, -0.5f,  0.5f,  1.0f, 0.3f, 0.3f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.3f, 0.3f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.3f, 0.3f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.3f, 0.3f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.3f, 0.3f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.3f, 0.3f,

        // Back face (cyan)
        -0.5f, -0.5f, -0.5f,  0.3f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.3f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.3f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.3f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.3f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.3f, 1.0f, 1.0f,

        // Top face (green)
        -0.5f,  0.5f, -0.5f,  0.3f, 1.0f, 0.3f,
        -0.5f,  0.5f,  0.5f,  0.3f, 1.0f, 0.3f,
         0.5f,  0.5f,  0.5f,  0.3f, 1.0f, 0.3f,
        -0.5f,  0.5f, -0.5f,  0.3f, 1.0f, 0.3f,
         0.5f,  0.5f,  0.5f,  0.3f, 1.0f, 0.3f,
         0.5f,  0.5f, -0.5f,  0.3f, 1.0f, 0.3f,

        // Bottom face (magenta)
        -0.5f, -0.5f, -0.5f,  1.0f, 0.3f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.3f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.3f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.3f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.3f, 1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.3f, 1.0f,

        // Right face (yellow)
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.3f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.3f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.3f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.3f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.3f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.3f,

        // Left face (blue)
        -0.5f, -0.5f, -0.5f,  0.3f, 0.3f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.3f, 0.3f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.3f, 0.3f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.3f, 0.3f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.3f, 0.3f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.3f, 0.3f, 1.0f,
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    return static_cast<int32_t>(vbo);
}

inline void mana_setupCubeAttributes() {
    // Same as triangle: position (location 0) + color (location 1)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

inline void mana_drawCube() {
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// ============================================================================
// Input handling
// ============================================================================

// Check if a key is currently pressed
inline int32_t mana_glfwGetKey(int64_t window, int32_t key) {
    return glfwGetKey(reinterpret_cast<GLFWwindow*>(window), key);
}

// Check if a mouse button is pressed
inline int32_t mana_glfwGetMouseButton(int64_t window, int32_t button) {
    return glfwGetMouseButton(reinterpret_cast<GLFWwindow*>(window), button);
}

// Get cursor position (returns via static variables for simplicity)
static double g_cursor_x = 0.0, g_cursor_y = 0.0;
inline void mana_glfwGetCursorPos(int64_t window) {
    glfwGetCursorPos(reinterpret_cast<GLFWwindow*>(window), &g_cursor_x, &g_cursor_y);
}
inline double mana_getCursorX() { return g_cursor_x; }
inline double mana_getCursorY() { return g_cursor_y; }

// Set input mode (for cursor lock/hide)
inline void mana_glfwSetInputMode(int64_t window, int32_t mode, int32_t value) {
    glfwSetInputMode(reinterpret_cast<GLFWwindow*>(window), mode, value);
}

// Key constants
inline int32_t mana_GLFW_KEY_W() { return GLFW_KEY_W; }
inline int32_t mana_GLFW_KEY_A() { return GLFW_KEY_A; }
inline int32_t mana_GLFW_KEY_S() { return GLFW_KEY_S; }
inline int32_t mana_GLFW_KEY_D() { return GLFW_KEY_D; }
inline int32_t mana_GLFW_KEY_Q() { return GLFW_KEY_Q; }
inline int32_t mana_GLFW_KEY_E() { return GLFW_KEY_E; }
inline int32_t mana_GLFW_KEY_SPACE() { return GLFW_KEY_SPACE; }
inline int32_t mana_GLFW_KEY_ESCAPE() { return GLFW_KEY_ESCAPE; }
inline int32_t mana_GLFW_KEY_UP() { return GLFW_KEY_UP; }
inline int32_t mana_GLFW_KEY_DOWN() { return GLFW_KEY_DOWN; }
inline int32_t mana_GLFW_KEY_LEFT() { return GLFW_KEY_LEFT; }
inline int32_t mana_GLFW_KEY_RIGHT() { return GLFW_KEY_RIGHT; }
inline int32_t mana_GLFW_KEY_TAB() { return GLFW_KEY_TAB; }
inline int32_t mana_GLFW_PRESS() { return GLFW_PRESS; }
inline int32_t mana_GLFW_RELEASE() { return GLFW_RELEASE; }
inline int32_t mana_GLFW_MOUSE_BUTTON_LEFT() { return GLFW_MOUSE_BUTTON_LEFT; }
inline int32_t mana_GLFW_MOUSE_BUTTON_RIGHT() { return GLFW_MOUSE_BUTTON_RIGHT; }

// Input mode constants
inline int32_t mana_GLFW_CURSOR() { return GLFW_CURSOR; }
inline int32_t mana_GLFW_CURSOR_NORMAL() { return GLFW_CURSOR_NORMAL; }
inline int32_t mana_GLFW_CURSOR_HIDDEN() { return GLFW_CURSOR_HIDDEN; }
inline int32_t mana_GLFW_CURSOR_DISABLED() { return GLFW_CURSOR_DISABLED; }

// ============================================================================
// Voxel rendering helpers
// ============================================================================

// Set model matrix with position and rotation
inline void mana_setModelTransform(float px, float py, float pz, float rx, float ry, float rz) {
    float rotX[16], rotY[16], rotZ[16], trans[16], temp1[16], temp2[16];
    mana_mat4_rotateX(rotX, rx);
    mana_mat4_rotateY(rotY, ry);
    mana_mat4_rotateZ(rotZ, rz);
    mana_mat4_translate(trans, px, py, pz);

    // Rotation: Z * Y * X
    mana_mat4_multiply(temp1, rotY, rotX);
    mana_mat4_multiply(temp2, rotZ, temp1);
    // Then translate
    mana_mat4_multiply(g_model_matrix, trans, temp2);
}

// Set model position only (no rotation) - faster for voxels
inline void mana_setModelPosition(float px, float py, float pz) {
    mana_mat4_translate(g_model_matrix, px, py, pz);
}

// Create a solid-color cube VBO (for voxel blocks)
// color: 0=grass(green top), 1=dirt(brown), 2=stone(gray), 3=water(blue), 4=sand(yellow)
inline int32_t mana_createBlockVBO(int32_t blockType) {
    // Colors for different block types
    float topR, topG, topB;
    float sideR, sideG, sideB;
    float bottomR, bottomG, bottomB;

    switch (blockType) {
        case 0: // Grass
            topR = 0.2f; topG = 0.8f; topB = 0.2f;       // Green top
            sideR = 0.55f; sideG = 0.35f; sideB = 0.15f; // Brown sides
            bottomR = 0.55f; bottomG = 0.35f; bottomB = 0.15f;
            break;
        case 1: // Dirt
            topR = 0.55f; topG = 0.35f; topB = 0.15f;
            sideR = 0.55f; sideG = 0.35f; sideB = 0.15f;
            bottomR = 0.45f; bottomG = 0.28f; bottomB = 0.1f;
            break;
        case 2: // Stone
            topR = 0.5f; topG = 0.5f; topB = 0.5f;
            sideR = 0.45f; sideG = 0.45f; sideB = 0.45f;
            bottomR = 0.4f; bottomG = 0.4f; bottomB = 0.4f;
            break;
        case 3: // Water
            topR = 0.2f; topG = 0.4f; topB = 0.9f;
            sideR = 0.15f; sideG = 0.35f; sideB = 0.85f;
            bottomR = 0.1f; bottomG = 0.3f; bottomB = 0.8f;
            break;
        case 4: // Sand
            topR = 0.95f; topG = 0.9f; topB = 0.55f;
            sideR = 0.9f; sideG = 0.85f; sideB = 0.5f;
            bottomR = 0.85f; bottomG = 0.8f; bottomB = 0.45f;
            break;
        default: // White
            topR = topG = topB = 1.0f;
            sideR = sideG = sideB = 0.9f;
            bottomR = bottomG = bottomB = 0.8f;
    }

    float vertices[] = {
        // Front face (side color)
        -0.5f, -0.5f,  0.5f,  sideR, sideG, sideB,
         0.5f, -0.5f,  0.5f,  sideR, sideG, sideB,
         0.5f,  0.5f,  0.5f,  sideR, sideG, sideB,
        -0.5f, -0.5f,  0.5f,  sideR, sideG, sideB,
         0.5f,  0.5f,  0.5f,  sideR, sideG, sideB,
        -0.5f,  0.5f,  0.5f,  sideR, sideG, sideB,

        // Back face (side color)
        -0.5f, -0.5f, -0.5f,  sideR, sideG, sideB,
        -0.5f,  0.5f, -0.5f,  sideR, sideG, sideB,
         0.5f,  0.5f, -0.5f,  sideR, sideG, sideB,
        -0.5f, -0.5f, -0.5f,  sideR, sideG, sideB,
         0.5f,  0.5f, -0.5f,  sideR, sideG, sideB,
         0.5f, -0.5f, -0.5f,  sideR, sideG, sideB,

        // Top face (top color)
        -0.5f,  0.5f, -0.5f,  topR, topG, topB,
        -0.5f,  0.5f,  0.5f,  topR, topG, topB,
         0.5f,  0.5f,  0.5f,  topR, topG, topB,
        -0.5f,  0.5f, -0.5f,  topR, topG, topB,
         0.5f,  0.5f,  0.5f,  topR, topG, topB,
         0.5f,  0.5f, -0.5f,  topR, topG, topB,

        // Bottom face (bottom color)
        -0.5f, -0.5f, -0.5f,  bottomR, bottomG, bottomB,
         0.5f, -0.5f, -0.5f,  bottomR, bottomG, bottomB,
         0.5f, -0.5f,  0.5f,  bottomR, bottomG, bottomB,
        -0.5f, -0.5f, -0.5f,  bottomR, bottomG, bottomB,
         0.5f, -0.5f,  0.5f,  bottomR, bottomG, bottomB,
        -0.5f, -0.5f,  0.5f,  bottomR, bottomG, bottomB,

        // Right face (side color)
         0.5f, -0.5f, -0.5f,  sideR * 0.9f, sideG * 0.9f, sideB * 0.9f,
         0.5f,  0.5f, -0.5f,  sideR * 0.9f, sideG * 0.9f, sideB * 0.9f,
         0.5f,  0.5f,  0.5f,  sideR * 0.9f, sideG * 0.9f, sideB * 0.9f,
         0.5f, -0.5f, -0.5f,  sideR * 0.9f, sideG * 0.9f, sideB * 0.9f,
         0.5f,  0.5f,  0.5f,  sideR * 0.9f, sideG * 0.9f, sideB * 0.9f,
         0.5f, -0.5f,  0.5f,  sideR * 0.9f, sideG * 0.9f, sideB * 0.9f,

        // Left face (side color, darker)
        -0.5f, -0.5f, -0.5f,  sideR * 0.8f, sideG * 0.8f, sideB * 0.8f,
        -0.5f, -0.5f,  0.5f,  sideR * 0.8f, sideG * 0.8f, sideB * 0.8f,
        -0.5f,  0.5f,  0.5f,  sideR * 0.8f, sideG * 0.8f, sideB * 0.8f,
        -0.5f, -0.5f, -0.5f,  sideR * 0.8f, sideG * 0.8f, sideB * 0.8f,
        -0.5f,  0.5f,  0.5f,  sideR * 0.8f, sideG * 0.8f, sideB * 0.8f,
        -0.5f,  0.5f, -0.5f,  sideR * 0.8f, sideG * 0.8f, sideB * 0.8f,
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    return static_cast<int32_t>(vbo);
}

// Simple terrain height function (returns 0-4 based on x,z)
inline int32_t mana_getTerrainHeight(int32_t x, int32_t z) {
    // Simple sine-based terrain
    float fx = static_cast<float>(x) * 0.5f;
    float fz = static_cast<float>(z) * 0.5f;
    float height = 2.0f + sinf(fx) * 1.5f + cosf(fz) * 1.5f + sinf(fx + fz) * 0.5f;
    return static_cast<int32_t>(height);
}

// First-person camera helpers
static float g_cam_x = 0.0f, g_cam_y = 5.0f, g_cam_z = 10.0f;
static float g_cam_yaw = 0.0f, g_cam_pitch = 0.0f;

inline void mana_setCameraFPS(float x, float y, float z, float yaw, float pitch) {
    g_cam_x = x; g_cam_y = y; g_cam_z = z;
    g_cam_yaw = yaw; g_cam_pitch = pitch;

    // Build view matrix: rotate then translate
    float rotY[16], rotX[16], trans[16], temp[16];
    mana_mat4_rotateY(rotY, -yaw);
    mana_mat4_rotateX(rotX, -pitch);
    mana_mat4_translate(trans, -x, -y, -z);

    mana_mat4_multiply(temp, rotX, rotY);
    mana_mat4_multiply(g_view_matrix, temp, trans);
}

inline float mana_getCamX() { return g_cam_x; }
inline float mana_getCamY() { return g_cam_y; }
inline float mana_getCamZ() { return g_cam_z; }

// Get forward/right vectors for movement
inline float mana_getForwardX() { return -sinf(g_cam_yaw); }
inline float mana_getForwardZ() { return -cosf(g_cam_yaw); }
inline float mana_getRightX() { return cosf(g_cam_yaw); }
inline float mana_getRightZ() { return -sinf(g_cam_yaw); }

// Mouse look helpers
static double g_last_mouse_x = 0.0, g_last_mouse_y = 0.0;
static bool g_mouse_captured = false;
static bool g_first_mouse = true;

inline void mana_captureMouse(int64_t window) {
    glfwSetInputMode(reinterpret_cast<GLFWwindow*>(window), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    g_mouse_captured = true;
    g_first_mouse = true;
}

inline void mana_releaseMouse(int64_t window) {
    glfwSetInputMode(reinterpret_cast<GLFWwindow*>(window), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    g_mouse_captured = false;
}

inline int32_t mana_isMouseCaptured() {
    return g_mouse_captured ? 1 : 0;
}

// Returns mouse delta and updates last position
static float g_mouse_delta_x = 0.0f, g_mouse_delta_y = 0.0f;

inline void mana_updateMouseDelta(int64_t window) {
    double x, y;
    glfwGetCursorPos(reinterpret_cast<GLFWwindow*>(window), &x, &y);

    if (g_first_mouse) {
        g_last_mouse_x = x;
        g_last_mouse_y = y;
        g_first_mouse = false;
    }

    g_mouse_delta_x = static_cast<float>(x - g_last_mouse_x);
    g_mouse_delta_y = static_cast<float>(g_last_mouse_y - y); // Inverted
    g_last_mouse_x = x;
    g_last_mouse_y = y;
}

inline float mana_getMouseDeltaX() { return g_mouse_delta_x; }
inline float mana_getMouseDeltaY() { return g_mouse_delta_y; }

// Tree generation helper
inline int32_t mana_shouldPlaceTree(int32_t x, int32_t z) {
    // Simple pseudo-random based on position
    int hash = (x * 374761393 + z * 668265263) ^ (x * z);
    hash = (hash >> 13) ^ hash;
    return (hash % 7 == 0) ? 1 : 0; // ~14% chance of tree
}

inline int32_t mana_getTreeHeight() {
    return 3 + (rand() % 2); // 3-4 blocks tall
}
