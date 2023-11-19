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

int num_frames{};
float last_time{};
float center_x{ 0.0f };
float center_y{ 0.0f };
float zoom{ 1.0 };
float julia_real{ 0.355 };
float julia_imag{ 0.355 };
float fractalType{ 0.0 };

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
        if (zoom > 1.0f)
        {
            zoom = 1.0f;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        zoom = zoom * 0.96f;
        if (zoom < 0.00001f)
        {
            zoom = 0.00001f;
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
    std::sort(data.begin(), data.end());
    int lowest = 0;
    while (data[lowest] == 0.0f)
    {
        ++lowest;
    }

    int size = data.size();
    int length = size - lowest;
    glm::vec4 ranges = glm::vec4(data[lowest], data[lowest + length * 3 / 4 - 1], data[lowest + length * 7 / 8 - 1], data[size - 1]);
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
        our_shader.set_float("zoom", zoom);
        our_shader.set_float("center_x", center_x);
        our_shader.set_float("center_y", center_y);
        our_shader.set_vec4("color_ranges", ranges);
        our_shader.set_float("julia_real", julia_real);
        our_shader.set_float("julia_imag", julia_imag);
        our_shader.set_float("fractalType", fractalType);

        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Create a window in ImGui for controls
        ImGui::Begin("Controls");

        // Add example control
        if (ImGui::Button("Zoom In")) {
            zoom = zoom * 0.96f;
            if (zoom < 0.00001f)
            {
                zoom = 0.00001f;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Zoom Out")) {
            zoom = zoom * 1.04f;
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
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        glReadPixels(0, 0, screen_width, screen_height, GL_DEPTH_COMPONENT, GL_FLOAT, pixel_data.data());
        ranges = find_ranges(pixel_data);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}