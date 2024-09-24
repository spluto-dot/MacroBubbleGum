
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

void render_gui();

// Input simulation state
bool is_playing = false;
bool is_paused = false;

// Main function for setting up GLFW and ImGui
int main(int, char**)
{
    // Setup GLFW
    if (!glfwInit())
        return -1;

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Input Simulator", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll events
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render the GUI
        render_gui();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
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

    // Play button
    if (ImGui::Button("Play"))
    {
        is_playing = true;
        is_paused = false;
    }

    // Pause button
    ImGui::SameLine();
    if (ImGui::Button("Pause"))
    {
        if (is_playing) is_paused = true;
    }

    // Stop button
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
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

    // Placeholder for Input Table
    ImGui::Text("Input Table (under development)");

    ImGui::End();
}
