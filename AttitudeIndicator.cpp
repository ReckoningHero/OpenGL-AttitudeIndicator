// AttitudeIndicator.cpp (Modern OpenGL Version, No GLM)
// Uses shaders, VBOs, and manual transformation matrices

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float pitch = 0.0f;
float roll = 0.0f;

// Vertex Shader
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(aPos, 0.0, 1.0);
}
)glsl";

// Fragment Shader
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)glsl";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pitch += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pitch -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        roll -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        roll += 0.5f;
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}

unsigned int createShaderProgram() {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void createTransformMatrix(float pitch, float roll, float* outMatrix) {
    float radians = roll * 3.1415926f / 180.0f;
    float cosA = cosf(radians);
    float sinA = sinf(radians);
    float translateY = pitch / 90.0f;

    float mat[16] = {
        cosA,  sinA, 0.0f, 0.0f,
       -sinA,  cosA, 0.0f, 0.0f,
        0.0f,  0.0f, 1.0f, 0.0f,
        0.0f,  translateY, 0.0f, 1.0f
    };

    for (int i = 0; i < 16; ++i)
        outMatrix[i] = mat[i];
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Attitude Indicator", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    unsigned int shaderProgram = createShaderProgram();

    float skyVertices[] = {
        -1.0f,  0.0f,
         1.0f,  0.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    float groundVertices[] = {
        -1.0f,  0.0f,
         1.0f,  0.0f,
         1.0f, -1.0f,
        -1.0f, -1.0f
    };

    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    unsigned int VAO[2], VBO[2], EBO;
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(1, &EBO);

    for (int i = 0; i < 2; ++i) {
        glBindVertexArray(VAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyVertices), (i == 0 ? skyVertices : groundVertices), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    glUseProgram(shaderProgram);
    int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    int colorLoc = glGetUniformLocation(shaderProgram, "color");

    float transformMatrix[16];

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        createTransformMatrix(pitch, roll, transformMatrix);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transformMatrix);

        glUniform3f(colorLoc, 0.2f, 0.4f, 0.8f); // sky
        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glUniform3f(colorLoc, 0.5f, 0.3f, 0.1f); // ground
        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
