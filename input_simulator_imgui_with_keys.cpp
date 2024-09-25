
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>

void render_gui();

struct InputEntry {
    std::string key;
    int frames;
};

std::vector<InputEntry> inputs;
bool is_playing = false;
bool is_paused = false;

// Main function for setting up GLFW and ImGui
int main(int, char**)
{
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Input Simulator", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render_gui();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// GUI rendering function
void render_gui()
{
    ImGui::Begin("Input Simulator");

    static const char* keys[] = { "UP", "DOWN", "LEFT", "RIGHT", "A", "B", "X", "Y" };
    static int current_key = 0;
    static int frame_count = 1;

    // Play, Pause, Stop buttons
    if (ImGui::Button("Play")) {
        is_playing = true;
        is_paused = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        if (is_playing) is_paused = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        is_playing = false;
        is_paused = false;
    }

    // Display current status
    if (is_playing && !is_paused)
        ImGui::Text("Status: Playing");
    else if (is_paused)
        ImGui::Text("Status: Paused");
    else
        ImGui::Text("Status: Stopped");

    ImGui::Separator();

    // Key selection and frame count input
    ImGui::Text("Select Key and Number of Frames");
    ImGui::Combo("Key", &current_key, keys, IM_ARRAYSIZE(keys));
    ImGui::InputInt("Frames", &frame_count);
    if (frame_count < 1) frame_count = 1;

    // Add key to the input list
    if (ImGui::Button("Add Input")) {
        inputs.push_back({ keys[current_key], frame_count });
    }

    ImGui::Separator();

    // Display input list
    ImGui::Text("Input List:");
    for (size_t i = 0; i < inputs.size(); ++i) {
        ImGui::Text("Key: %s | Frames: %d", inputs[i].key.c_str(), inputs[i].frames);
    }

    ImGui::End();
}
