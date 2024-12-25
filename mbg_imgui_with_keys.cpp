#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <windows.h>
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>

// Estrutura para armazenar os logs do console
std::vector<std::string> console_logs;

// Estrutura para capturar entradas
struct KeyEvent {
    std::string key;
    bool is_press;
    long long timestamp;
};

std::vector<KeyEvent> events;
bool is_recording = false;
bool capture_mouse = false;
std::chrono::steady_clock::time_point recording_start;

// Frame duration configurável (padrão: 60fps)
long long frame_duration = 16670; // 60fps

// Função para registrar logs no console
void LogToConsole(const std::string& message) {
    console_logs.push_back(message);
}

// Função para salvar entradas no arquivo
void SaveInputsToFile() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
#ifdef _WIN32
    localtime_s(&now_tm, &now_time);
#else
    localtime_r(&now_time, &now_tm);
#endif
    char filename[32];
    std::strftime(filename, sizeof(filename), "%Y%m%d%H%M%S.txt", &now_tm);

    std::ofstream file(filename);
    long long total_elapsed_us = 0;

    for (const auto& event : events) {
        while (total_elapsed_us + frame_duration <= event.timestamp) {
            file << "sleep_us(" << frame_duration << ")\n";
            total_elapsed_us += frame_duration;
        }
        if (event.is_press) {
            file << "holdKey(\"" << event.key << "\") // Timestamp: " << event.timestamp << " microseconds\n";
        } else {
            file << "releaseKey(\"" << event.key << "\") // Timestamp: " << event.timestamp << " microseconds\n";
        }
        total_elapsed_us = event.timestamp;
    }

    file.close();
    LogToConsole("Inputs salvos em " + std::string(filename));
}

// Mapear as teclas para nomes legíveis
std::string GetKeyName(int vk_code) {
    if ((vk_code >= 0x30 && vk_code <= 0x39) || (vk_code >= 0x41 && vk_code <= 0x5A)) {
        return std::string(1, static_cast<char>(vk_code)); // Letras e números
    }
    switch (vk_code) {
        case VK_UP: return "UP";
        case VK_DOWN: return "DOWN";
        case VK_LEFT: return "LEFT";
        case VK_RIGHT: return "RIGHT";
        case VK_SPACE: return "SPACE";
        case VK_RETURN: return "ENTER";
        default: return "UNKNOWN";
    }
}

// Capturar inputs do teclado
void CaptureInputs() {
    auto now = std::chrono::steady_clock::now();
    long long current_time = std::chrono::duration_cast<std::chrono::microseconds>(now - recording_start).count();

    for (int vk_code = 0x01; vk_code <= 0xFE; ++vk_code) {
        std::string key_name = GetKeyName(vk_code);
        if (key_name != "UNKNOWN") {
            static std::unordered_map<std::string, bool> key_states;

            bool is_pressed = (GetAsyncKeyState(vk_code) & 0x8000) != 0;
            if (is_pressed && !key_states[key_name]) {
                // Tecla pressionada
                key_states[key_name] = true;
                events.push_back({key_name, true, current_time});
                LogToConsole("Tecla pressionada: " + key_name);
            } else if (!is_pressed && key_states[key_name]) {
                // Tecla liberada
                key_states[key_name] = false;
                events.push_back({key_name, false, current_time});
                LogToConsole("Tecla liberada: " + key_name);
            }
        }
    }
}

// Capturar posição e cliques do mouse
void CaptureMouse() {
    auto now = std::chrono::steady_clock::now();
    long long current_time = std::chrono::duration_cast<std::chrono::microseconds>(now - recording_start).count();

    POINT mouse_pos;
    GetCursorPos(&mouse_pos);

    static POINT last_pos = {0, 0};
    static bool left_pressed = false;
    static bool right_pressed = false;

    if (mouse_pos.x != last_pos.x || mouse_pos.y != last_pos.y) {
        events.push_back({"MOUSE_MOVE", true, current_time});
        LogToConsole("Mouse movido para: (" + std::to_string(mouse_pos.x) + ", " + std::to_string(mouse_pos.y) + ")");
        last_pos = mouse_pos;
    }

    bool left_now = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (left_now != left_pressed) {
        left_pressed = left_now;
        events.push_back({left_now ? "MOUSE_LEFT_DOWN" : "MOUSE_LEFT_UP", left_now, current_time});
        LogToConsole(left_now ? "Botão esquerdo pressionado" : "Botão esquerdo liberado");
    }

    bool right_now = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (right_now != right_pressed) {
        right_pressed = right_now;
        events.push_back({right_now ? "MOUSE_RIGHT_DOWN" : "MOUSE_RIGHT_UP", right_now, current_time});
        LogToConsole(right_now ? "Botão direito pressionado" : "Botão direito liberado");
    }
}

// Renderiza a interface gráfica
void render_gui() {
    ImGui::Begin("MacroBubbleGum");
    ImGui::Text("=== Aviso ===");
    ImGui::Text("Shift e Ctrl não são suportadas.");

    if (ImGui::Button(is_recording ? "Parar" : "Gravar")) {
        is_recording = !is_recording;
        if (is_recording) {
            recording_start = std::chrono::steady_clock::now();
            events.clear();
            LogToConsole("Gravação iniciada");
        } else {
            SaveInputsToFile();
            LogToConsole("Gravação parada");
        }
    }

    ImGui::Text("Status: %s", is_recording ? "Gravando" : "Parado");

    if (ImGui::Checkbox("Capturar Mouse", &capture_mouse)) {
        LogToConsole(capture_mouse ? "Captura de mouse ativada" : "Captura de mouse desativada");
    }

    // Botões para selecionar a taxa de quadros
    if (ImGui::Button("60fps (16670us)")) frame_duration = 16670;
    ImGui::SameLine();
    if (ImGui::Button("59.94fps (16683us)")) frame_duration = 16683;
    ImGui::SameLine();
    if (ImGui::Button("57.52416fps (17380us)")) frame_duration = 17380;
    ImGui::SameLine();
    if (ImGui::Button("30fps (33333us)")) frame_duration = 33333;

    // Botão para limpar o console
    if (ImGui::Button("Limpar Console")) {
        console_logs.clear();
    }

    ImGui::Text("Console:");
    ImGui::BeginChild("ConsoleLogs", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& log : console_logs) {
        ImGui::TextUnformatted(log.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();

    if (is_recording) {
        CaptureInputs();
        if (capture_mouse) CaptureMouse();
    }
}

int main() {
    // Inicialização do GLFW
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "MacroBubbleGum", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    while (!glfwWindowShouldClose(window)) {
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
