#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <algorithm>
#include <vector>

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <GLM\glm.hpp>
#include <GLM\gtc\matrix_transform.hpp>
#include <GLM\gtc\type_ptr.hpp>

#include "Shader.h"

int screen_width{ 1080 };
int screen_height{ 1080 };

glm::vec4 color_0{glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)};
glm::vec4 color_1{glm::vec4(0.0f, 0.2f, 0.5f, 1.0f)};
glm::vec4 color_2{glm::vec4(1.0f, 0.8f, 0.1f, 1.0f)};
glm::vec4 color_3{glm::vec4(1.0f, 0.0f, 0.4f, 1.0f)};

int num_frames{};
float last_time{};
float center_x{ 0.0f };
float center_y{ 0.0f };
float zoom{ 1.0 };
float linear_zoom{ 1.0 };
float julia_real{ 0.355 };
float julia_imag{ 0.355 };
float fractalType{ 0.0 };

float sierpinski_xFactor{ 4.0 };
float sierpinski_yFactor{ 4.0 };
float sierpinski_xLower{ 0.3333 };
float sierpinski_xUpper{ 0.6666 };
float sierpinski_yLower{ 0.3333 };
float sierpinski_yUpper{ 0.6666 };

float vertices[] =
{
    //    x      y      z   
        -1.0f, -1.0f, -0.0f,
         1.0f,  1.0f, -0.0f,
        -1.0f,  1.0f, -0.0f,
         1.0f, -1.0f, -0.0f
};

unsigned int indices[] =
{
    //  2---,1
    //  | .' |
    //  0'---3
        0, 1, 2,
        0, 3, 1
};


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void process_input(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        center_y = center_y + 0.01f * zoom;
        if (center_y > 1.0f)
        {
            center_y = 1.0f;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        center_y = center_y - 0.01f * zoom;
        if (center_y < -1.0f)
        {
            center_y = -1.0f;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        center_x = center_x - 0.01f * zoom;
        if (center_x < -1.0f)
        {
            center_x = -1.0f;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        center_x = center_x + 0.01f * zoom;
        if (center_x > 1.0f)
        {
            center_x = 1.0f;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        zoom = zoom * 1.04f;
        linear_zoom = linear_zoom - 0.01f;
        if (zoom > 1.0f)
        {
            zoom = 1.0f;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        zoom = zoom * 0.96f;
        linear_zoom = linear_zoom + 0.01f;
        if (zoom < 0.00000001f)
        {
            zoom = 0.00000001f;
        }
    }
}

void countFPS()
{
    double current_time = glfwGetTime();
    num_frames++;
    if (current_time - last_time >= 1.0)
    {
        std::cout << 1000.0 / num_frames << "ms / frame\n";
        num_frames = 0;
        last_time += 1.0;
    }
}

glm::vec4 find_ranges(std::vector<float>& data)
{
    if (data.empty()) {
        return glm::vec4(0.0f); // Return a zero vector if data is empty
    }

    std::sort(data.begin(), data.end());
    int lowest = 0;
    while (lowest < data.size() && data[lowest] == 0.0f)
    {
        ++lowest;
    }

    int size = data.size();
    int length = size - lowest;
    glm::vec4 ranges(0.0f);

    if (lowest < size) {
        ranges[0] = data[lowest];
    }
    if (lowest + length * 3 / 4 - 1 < size) {
        ranges[1] = data[lowest + length * 3 / 4 - 1];
    }
    if (lowest + length * 7 / 8 - 1 < size) {
        ranges[2] = data[lowest + length * 7 / 8 - 1];
    }
    if (size - 1 < size) {
        ranges[3] = data[size - 1];
    }

    return ranges;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 16);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glEnable(GL_MULTISAMPLE);

    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Mandelbrot", NULL, NULL);

    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit())
    {
        std::cout << "Failed initializing GLEW\n";
    }

    glViewport(0, 0, screen_width, screen_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader our_shader("shader.vert", "shader.frag");

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    last_time = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

    std::vector<float> pixel_data(screen_width * screen_height, 0.0f);
    glm::vec4 ranges = glm::vec4(0.0001f, 0.33333f, 0.66667f, 1.00f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window))
    {
        glfwMakeContextCurrent(window);
        glClearColor(0.2f, 0.0f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        process_input(window);
        countFPS();

        our_shader.use_shader();
        our_shader.set_vec4("color_0", color_0);
        our_shader.set_vec4("color_1", color_1);
        our_shader.set_vec4("color_2", color_2);
        our_shader.set_vec4("color_3", color_3);
        our_shader.set_float("zoom", zoom);
        our_shader.set_float("linear_zoom", linear_zoom);
        our_shader.set_float("center_x", center_x);
        our_shader.set_float("center_y", center_y);
        our_shader.set_vec4("color_ranges", ranges);
        our_shader.set_float("julia_real", julia_real);
        our_shader.set_float("julia_imag", julia_imag);
        our_shader.set_float("fractalType", fractalType);
        
        our_shader.set_float("sierpinski_xFactor", sierpinski_xFactor);
        our_shader.set_float("sierpinski_yFactor", sierpinski_yFactor);
        our_shader.set_float("sierpinski_xLower", sierpinski_xLower);
        our_shader.set_float("sierpinski_xUpper", sierpinski_xUpper);
        our_shader.set_float("sierpinski_yLower", sierpinski_yLower);
        our_shader.set_float("sierpinski_yUpper", sierpinski_yUpper);

        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Create a window in ImGui for controls
        ImGui::Begin("Controls");

        // Add example control
        if (ImGui::Button("Zoom In")) {
            zoom = zoom * 0.96f;
            linear_zoom = linear_zoom + 0.01f;
            if (zoom < 0.00001f)
            {
                zoom = 0.00001f;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Zoom Out")) {
            zoom = zoom * 1.04f;
            linear_zoom = linear_zoom - 0.01f;
            if (zoom > 1.0f)
            {
                zoom = 1.0f;
            }
        }
        if (ImGui::Button("Reset")) {
            zoom = 1.0;
            center_x = 0.0;
            center_y = 0.0;
        }
        if (fractalType == 1) {
            if (ImGui::Button("-##Real")) {
                julia_real -= 0.001f;
            }
            ImGui::SameLine();
            ImGui::Text("Real");
            ImGui::SameLine();
            if (ImGui::Button("+##Real")) {
                julia_real += 0.001f;
            }
            if (ImGui::Button("-##Imag")) {
                julia_imag -= 0.001f;
            }
            ImGui::SameLine();
            ImGui::Text("Imaginary");
            ImGui::SameLine();
            if (ImGui::Button("+##Imag")) {
                julia_imag += 0.001f;
            }
        }
        if (fractalType == 2) {
            if (ImGui::Button("-##xFactor")) {
                sierpinski_xFactor -= 1;
            }
            ImGui::SameLine();
            ImGui::Text("X Factor");
            ImGui::SameLine();
            if (ImGui::Button("+##xFactor")) {
                sierpinski_xFactor += 1;
            }
            ImGui::SameLine();
            ImGui::Text(std::to_string(sierpinski_xFactor).c_str());

            if (ImGui::Button("-##yFactor")) {
                std::cout << sierpinski_yFactor;
                sierpinski_yFactor -= 1;
            }
            ImGui::SameLine();
            ImGui::Text("Y Factor");
            ImGui::SameLine();
            if (ImGui::Button("+##yFactor")) {
                sierpinski_yFactor += 1;
            }
            ImGui::SameLine();
            ImGui::Text(std::to_string(sierpinski_yFactor).c_str());

            if (ImGui::Button("-##yLower")) {
                sierpinski_yLower -= 0.01f;
            }
            ImGui::SameLine();
            ImGui::Text("Y Lower");
            ImGui::SameLine();
            if (ImGui::Button("+##yLower")) {
                sierpinski_yLower += 0.01f;
            }
            ImGui::SameLine();
            ImGui::Text(std::to_string(sierpinski_yLower).c_str());

            if (ImGui::Button("-##yUpper")) {
                sierpinski_yUpper -= 0.01f;
            }
            ImGui::SameLine();
            ImGui::Text("Y Upper");
            ImGui::SameLine();
            if (ImGui::Button("+##yUpper")) {
                sierpinski_yUpper += 0.01f;
            }
            ImGui::SameLine();
            ImGui::Text(std::to_string(sierpinski_yUpper).c_str());

            if (ImGui::Button("-##xLower")) {
                sierpinski_xLower -= 0.01f;
            }
            ImGui::SameLine();
            ImGui::Text("X Lower");
            ImGui::SameLine();
            if (ImGui::Button("+##xLower")) {
                sierpinski_xLower += 0.01f;
            }
            ImGui::SameLine();
            ImGui::Text(std::to_string(sierpinski_xLower).c_str());

            if (ImGui::Button("-##xUpper")) {
                sierpinski_xUpper -= 0.01f;
            }
            ImGui::SameLine();
            ImGui::Text("X Upper");
            ImGui::SameLine();
            if (ImGui::Button("+##xUpper")) {
                sierpinski_xUpper += 0.01f;
            }
            ImGui::SameLine();
            ImGui::Text(std::to_string(sierpinski_xUpper).c_str());
        }

        ImGui::End();
        ImGui::Begin("Type");
        if (ImGui::Button("Mandelbrot")) {
            std::cout << fractalType;
            fractalType = 0.0;
        }
        if (ImGui::Button("Julia")) {
            std::cout << fractalType;
            fractalType = 1.0;
        }
        if (ImGui::Button("Sierpinski Carpet")) {
            std::cout << fractalType;
            fractalType = 2.0;
        }
        if (ImGui::Button("Sierpinski Triangle")) {
            std::cout << fractalType;
            fractalType = 3.0;
        }
        ImGui::End();
        ImGui::Begin("Colors");

        float outputColor0[4] = { color_0[0], color_0[1], color_0[2], color_0[3] };
        float defaultColor = outputColor0[0];
        ImGui::ColorEdit3("Color 0", outputColor0);
        color_0 = glm::vec4(outputColor0[0], outputColor0[1], outputColor0[2], 255);

        float outputColor1[4] = { color_1[0], color_1[1], color_1[2], color_1[3] };
        ImGui::ColorEdit3("Color 1", outputColor1);
        color_1 = glm::vec4(outputColor1[0], outputColor1[1], outputColor1[2], 255);

        float outputColor2[4] = { color_2[0], color_2[1], color_2[2], color_2[3] };
        ImGui::ColorEdit3("Color 2", outputColor2);
        color_2 = glm::vec4(outputColor2[0], outputColor2[1], outputColor2[2], 255);

        float outputColor3[4] = { color_3[0], color_3[1], color_3[2], color_3[3] };
        ImGui::ColorEdit3("Color 3", outputColor3);
        color_3 = glm::vec4(outputColor3[0], outputColor3[1], outputColor3[2], 255);



        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
        if (fractalType == 1 || fractalType == 0) {
        glReadPixels(0, 0, screen_width, screen_height, GL_DEPTH_COMPONENT, GL_FLOAT, pixel_data.data());
            ranges = find_ranges(pixel_data);
        }
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}