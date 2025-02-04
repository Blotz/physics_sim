// opengl
#include <glad/gl.h>
// glfw
#include <GLFW/glfw3.h>
// imgui
#include <imgui.h>
#include <backends/imgui_impl_glfw.cpp>
#include <backends/imgui_impl_opengl3.cpp>
// opengl maths
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// stdlib
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


// Function prototypes
void setupImGui(GLFWwindow* window);
void cleanupImGui();
void renderScene(GLuint shaderProgram, const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& colors);
void generateSphere(float radius, int rings, int sectors, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& colors);
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);
std::string readShaderSource(const char* filePath);
GLuint compileShader(GLenum type, const std::string& source);


GLuint VAO, VBO, colorBuffer;
const GLuint WIDTH = 800, HEIGHT = 600;


int main(void) {
    // Initialize GLFW
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "[glad] GL with GLFW", NULL, NULL);
	if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Load openGL
	int version = gladLoadGL(glfwGetProcAddress);
    printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    // Setup Dear ImGui context
    setupImGui(window);
	// Our state
	ImGuiIO& io = ImGui::GetIO(); (void)io;
    bool show_demo_window = false;
	bool show_log_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Create shader program
	GLuint shaderProgram = createShaderProgram("/home/dionysus/dox/coding/c++/physics_sim/src/vertex_shader.glsl", "/home/dionysus/dox/coding/c++/physics_sim/src/fragment_shader.glsl");
	// Generate sphere data
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;
    generateSphere(1.0f, 20, 20, vertices, colors);

    // Create and bind VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create and bind VBO for vertices
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Create and bind VBO for colors
    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
		// Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render opengl scene aka background
		renderScene(shaderProgram, vertices, colors);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		if (show_log_window)
			ImGui::ShowDebugLogWindow(&show_log_window);

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Manager");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Logging Window", &show_log_window);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

	// Cleanup
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &colorBuffer);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    cleanupImGui();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void renderScene(GLuint shaderProgram, const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& colors) {
    glUseProgram(shaderProgram);

    // Set up the model, view, and projection matrices
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f)); // Move back
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Pass matrices to the shader
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Bind the VAO and draw the sphere
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());
    glBindVertexArray(0);
}

void generateSphere(float radius, int rings, int sectors, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& colors) {
    float const R = 1.0f / (float)(rings - 1);
    float const S = 1.0f / (float)(sectors - 1);
    int r, s;

    for (r = 0; r < rings; r++) {
        for (s = 0; s < sectors; s++) {
            float y = sin(-M_PI_2 + M_PI * r * R); // Y coordinate
            float x = cosf(M_PI * s * S) * cosf(M_PI * r * R); // X coordinate
            float z = sinf(M_PI * s * S) * cosf(M_PI * r * R); // Z coordinate

            vertices.push_back(glm::vec3(x * radius, y * radius, z * radius));
            colors.push_back(glm::vec3((float)r / rings, (float)s / sectors, 0.5f)); // Color based on position
        }
    }
}


void setupImGui(GLFWwindow* window) {
    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130"); // Use the appropriate GLSL version
}

void cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// Function to read shader source code from a file
std::string readShaderSource(const char* filePath) {
    std::ifstream shaderFile(filePath);
    std::stringstream shaderStream;

    if (shaderFile.is_open()) {
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
    } else {
        std::cerr << "Could not open shader file: " << filePath << std::endl;
    }

    return shaderStream.str();
}

// Function to compile a shader
GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string infoLog(logLength, ' ');
        glGetShaderInfoLog(shader, logLength, nullptr, &infoLog[0]);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
    }

    return shader;
}

// Function to create a shader program
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    // Read shader source code
    std::string vertexSource = readShaderSource(vertexPath);
    std::string fragmentSource = readShaderSource(fragmentPath);

    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    // Create shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Clean up shaders (they are no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}