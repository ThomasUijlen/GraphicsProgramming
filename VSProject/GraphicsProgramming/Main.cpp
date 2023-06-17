#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <FastNoiseLite/FastNoiseLite.h>

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Mesh {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
glm::mat4 updateCameraView();
int init(GLFWwindow*& window);
void loadTextFromFile(const char* filename, char*& text);
unsigned int loadTexture(const char* filename);
void createBox(GLuint& vao, GLuint& ebo, int& size, int& indexCount);
Mesh createTerrain(int width, int depth, float scale);
GLuint compileShader(GLenum shaderType, const char* shaderSource);
GLuint createShaders(const char* vertexShaderFilename, const char* fragmentShaderFilename);
void renderSkybox(GLFWwindow* window, GLuint skyboxProgram, GLuint squareVAO, int squareIndexCount, glm::mat4 view, glm::mat4 projection, glm::vec3 lightDirection);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPosition = glm::vec3(0, 2.5f, -3.0f);
float cameraSpeed = 0.05f; // adjust accordingly
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float pitch = 0.0f;
float yaw = -90.0f;
bool firstMouse = true;

int main() {
    GLFWwindow* window;
    int res = init(window);
    if (res != 0) return res;

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLuint squareVAO;
    GLuint squareEBO;
    int squareSize;
    int squareIndexCount;
    createBox(squareVAO, squareEBO, squareSize, squareIndexCount);

    // Create shaders and get the program ID
    GLuint materialProgram = createShaders(
        "Shaders/SimpleVertexShader.shader",
        "Shaders/SimpleFragmentShader.shader"
    );
    GLuint skyboxProgram = createShaders(
        "Shaders/SkyVertexShader.shader",
        "Shaders/SkyFragmentShader.shader"
    );

    // Load texture
    unsigned int boxTex = loadTexture("Textures/BoxDiffuse.png");
    unsigned int boxNormal = loadTexture("Textures/BoxNormal.png");

    glUseProgram(materialProgram);
    glUniform1i(glGetUniformLocation(materialProgram, "boxTexture"), 0);
    glUniform1i(glGetUniformLocation(materialProgram, "boxNormal"), 1);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glm::mat4 world = glm::mat4(1.0f);
    world = glm::rotate(world, glm::radians(45.0f), glm::vec3(0, 1, 0));
    world = glm::scale(world, glm::vec3(1, 1, 1));
    world = glm::translate(world, glm::vec3(0, 0, 0));

    glm::mat4 projection = glm::perspective(45.0f, SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    glm::vec3 ambientLightColor = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDirection = glm::normalize(glm::vec3(1.0f, -1.0f, 0.0f));

    Mesh terrainMesh = createTerrain(50, 50, 10.0f);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Also clear the depth buffer

        glm::mat4 view = updateCameraView();

        renderSkybox(window, skyboxProgram, squareVAO, squareIndexCount, view, projection, lightDirection);

        // Use the shader program
        glUseProgram(materialProgram);

        glUniformMatrix4fv(glGetUniformLocation(materialProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
        glUniformMatrix4fv(glGetUniformLocation(materialProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(materialProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Update the light position
        glUniform3fv(glGetUniformLocation(materialProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
        glUniform3fv(glGetUniformLocation(materialProgram, "ambientLightColor"), 1, glm::value_ptr(ambientLightColor));
        glUniform3fv(glGetUniformLocation(materialProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));
        glUniform1f(glGetUniformLocation(materialProgram, "shininess"), 32.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, boxTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, boxNormal);

        glBindVertexArray(squareVAO);
        glDrawElements(GL_TRIANGLES, squareIndexCount, GL_UNSIGNED_INT, 0);

        glBindVertexArray(terrainMesh.vao);
        glDrawElements(GL_TRIANGLES, terrainMesh.indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteTextures(1, &boxTex);
    glDeleteVertexArrays(1, &squareVAO);
    glDeleteBuffers(1, &squareEBO);
    glDeleteProgram(materialProgram);

    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed = cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        speed = 2.0f * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPosition -= speed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPosition += speed * cameraUp;
}



void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // adjust to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

glm::mat4 updateCameraView()
{
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    // Calculate the new view matrix
    glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, glm::vec3(0.0f, 1.0f, 0.0f));

    return view;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int init(GLFWwindow*& window) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GraphicsProgramming", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    return 0;
}

void renderSkybox(GLFWwindow* window, GLuint skyboxProgram, GLuint squareVAO, int squareIndexCount, glm::mat4 view, glm::mat4 projection, glm::vec3 lightDirection) {
    // Disable depth writing (we always want the skybox behind everything else)
    glDepthMask(GL_FALSE);

    // Culling is not necessary for skybox (it's always viewed from the inside)
    glDisable(GL_CULL_FACE);

    // Use the skybox shader
    glUseProgram(skyboxProgram);

    // Remove the translation from the view matrix
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));

    glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
    glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(skyboxProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));

    // Draw the skybox cube
    glBindVertexArray(squareVAO);
    glDrawElements(GL_TRIANGLES, squareIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Restore the previous OpenGL state
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);

    glUseProgram(0); // Unbind the shader program
}


void createBox(GLuint& vao, GLuint& ebo, int& size, int& indexCount)
{
    // Vertex positions, colors, UV coordinates, and normals
    float vertices[] = {
        // positions            //colors            // tex coords   // normals          //tangents      //bitangents
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f
    };

    unsigned int indices[] = {  // note that we start from 0!
        // DOWN
        0, 1, 2,   // first triangle
        0, 2, 3,    // second triangle
        // BACK
        14, 6, 7,   // first triangle
        14, 7, 15,    // second triangle
        // RIGHT
        20, 4, 5,   // first triangle
        20, 5, 21,    // second triangle
        // LEFT
        16, 8, 9,   // first triangle
        16, 9, 17,    // second triangle
        // FRONT
        18, 10, 11,   // first triangle
        18, 11, 19,    // second triangle
        // UP
        22, 12, 13,   // first triangle
        22, 13, 23,    // second triangle
    };


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create the Vertex Buffer Object (VBO) and copy vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    int stride = (3 + 3 + 2 + 3 + 3 + 3) * sizeof(float);

    // Specify the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);  // Positions
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));  // Colors
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));  // UVs
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, stride, (void*)(8 * sizeof(float)));  // Normals
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_TRUE, stride, (void*)(11 * sizeof(float)));  // Tangents
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 3, GL_FLOAT, GL_TRUE, stride, (void*)(14 * sizeof(float)));  // Bitangents
    glEnableVertexAttribArray(5);


    // Create the Element Buffer Object (EBO) and copy index data to it
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Unbind the VAO
    glBindVertexArray(0);

    // Set the size and index count
    size = sizeof(vertices) / sizeof(vertices[0]);
    indexCount = sizeof(indices) / sizeof(indices[0]);
}

Mesh createTerrain(int width, int depth, float scale) {
    Mesh mesh;
    mesh.vertices.resize(width * depth);
    mesh.indices.resize((width - 1) * (depth - 1) * 6);

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            int index = z * width + x;
            float height = noise.GetNoise(x * scale, z * scale);
            mesh.vertices[index].position = glm::vec3(x, height, z);
            mesh.vertices[index].uv = glm::vec2(x / float(width), z / float(depth));
        }
    }

    for (int z = 0; z < depth - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            int index = z * (width - 1) + x;
            mesh.indices[index * 6 + 0] = z * width + x;
            mesh.indices[index * 6 + 1] = (z + 1) * width + x;
            mesh.indices[index * 6 + 2] = z * width + x + 1;
            mesh.indices[index * 6 + 3] = (z + 1) * width + x;
            mesh.indices[index * 6 + 4] = (z + 1) * width + x + 1;
            mesh.indices[index * 6 + 5] = z * width + x + 1;
        }
    }

    for (int z = 1; z < depth - 1; ++z) {
        for (int x = 1; x < width - 1; ++x) {
            glm::vec3& normal = mesh.vertices[z * width + x].normal;
            normal = glm::vec3(0, 1, 0);
            float heightLeft = mesh.vertices[z * width + x - 1].position.y;
            float heightRight = mesh.vertices[z * width + x + 1].position.y;
            float heightDown = mesh.vertices[(z - 1) * width + x].position.y;
            float heightUp = mesh.vertices[(z + 1) * width + x].position.y;
            normal.x = heightLeft - heightRight;
            normal.z = heightDown - heightUp;
            normal = glm::normalize(normal);
        }
    }

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), &mesh.indices[0], GL_STATIC_DRAW);

    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // Vertex Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    glBindVertexArray(0);

    return mesh;
}



GLuint compileShader(GLenum shaderType, const char* shaderSource)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    // Check shader compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar* infoLog = new GLchar[infoLogLength];
        glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
        delete[] infoLog;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createShaders(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
    // Load vertex shader
    char* vertexShaderSource = nullptr;
    loadTextFromFile(vertexShaderFilename, vertexShaderSource);
    if (vertexShaderSource == nullptr)
    {
        std::cerr << "Failed to load vertex shader: " << vertexShaderFilename << std::endl;
        return 0;
    }

    // Compile vertex shader
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    delete[] vertexShaderSource;
    if (vertexShader == 0)
    {
        std::cerr << "Failed to compile vertex shader." << std::endl;
        return 0;
    }

    // Load fragment shader
    char* fragmentShaderSource = nullptr;
    loadTextFromFile(fragmentShaderFilename, fragmentShaderSource);
    if (fragmentShaderSource == nullptr)
    {
        std::cerr << "Failed to load fragment shader: " << fragmentShaderFilename << std::endl;
        glDeleteShader(vertexShader);
        return 0;
    }

    // Compile fragment shader
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    delete[] fragmentShaderSource;
    if (fragmentShader == 0)
    {
        std::cerr << "Failed to compile fragment shader." << std::endl;
        glDeleteShader(vertexShader);
        return 0;
    }

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check program linking status
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint infoLogLength;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar* infoLog = new GLchar[infoLogLength];
        glGetProgramInfoLog(shaderProgram, infoLogLength, nullptr, infoLog);
        std::cerr << "Shader program linking error:\n" << infoLog << std::endl;
        delete[] infoLog;
        glDeleteProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // Cleanup
    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


void loadTextFromFile(const char* filename, char*& text)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Determine the file size
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Allocate memory for the text buffer
    text = new char[fileSize + 1];

    // Read the file contents into the text buffer
    if (!file.read(text, fileSize))
    {
        std::cerr << "Failed to read file: " << filename << std::endl;
        delete[] text;
        text = nullptr;
        return;
    }

    // Null-terminate the text buffer
    text[fileSize] = '\0';

    // Close the file
    file.close();
}

unsigned int loadTexture(const char* filename)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture wrapping and filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image file
    int width, height, numChannels;
    stbi_set_flip_vertically_on_load(true); // Flip image vertically as OpenGL expects the top-left corner to be the origin
    unsigned char* data = stbi_load(filename, &width, &height, &numChannels, 0);
    if (data)
    {
        // Determine the image format based on the number of channels
        GLenum format;
        if (numChannels == 1)
            format = GL_RED;
        else if (numChannels == 3)
            format = GL_RGB;
        else if (numChannels == 4)
            format = GL_RGBA;

        // Generate the texture and bind it
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free the image data
        stbi_image_free(data);
    }
    else
    {
        // If loading the image failed, print an error message and return 0
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}
