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

// Estrutura para armazenar os logs do console
std::vector<std::string> console_logs;

// Estrutura para capturar entradas
struct KeyEvent {
    std::string key;
    bool is_press;
};

struct MouseEvent {
    int x, y;
    bool left_click;
    bool right_click;
};

std::vector<KeyEvent> key_events;
std::vector<MouseEvent> mouse_events;

bool is_recording = false;
bool capture_mouse = false;
std::chrono::steady_clock::time_point recording_start;
int frame_interval_us = 16670; // Default para 60fps

// Função para registrar logs no console
void LogToConsole(const std::string& message) {
    console_logs.push_back(message);
}

// Função para salvar entradas no arquivo
void SaveInputsToFile() {
    std::ofstream file("inputs.txt");
    int frame_duration = selected_frame_duration; // Duração do frame (ex: 16670)
    int current_time = 0; // Tempo atual no arquivo de saída

    for (const auto& event : events) {
        // Gera os frames fixos até alcançar o próximo evento
        while (current_time + frame_duration <= event.timestamp) {
            file << "sleep_us(" << frame_duration << ")\n";
            current_time += frame_duration; // Incrementa o tempo em steps fixos
        }

        // Escreve o evento de pressionar/soltar tecla
        if (event.is_press) {
            file << "holdKey(\"" << event.key << "\")\n";
        } else {
            file << "releaseKey(\"" << event.key << "\")\n";
        }

        // Atualiza o tempo atual para o momento do evento
        current_time = event.timestamp;
    }

    // Preenche os frames restantes até o próximo múltiplo de `frame_duration`
    while (current_time % frame_duration != 0) {
        file << "sleep_us(" << frame_duration << ")\n";
        current_time += frame_duration;
    }

    file.close();
    LogToConsole("Inputs salvos em inputs.txt");
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

// Capturar inputs do teclado e mouse
void CaptureInputs() {
    // Capturar teclado
    for (int vk_code = 0x01; vk_code <= 0xFE; ++vk_code) {
        std::string key_name = GetKeyName(vk_code);
        if (key_name != "UNKNOWN") {
            static std::unordered_map<std::string, bool> key_states;

            bool is_pressed = (GetAsyncKeyState(vk_code) & 0x8000) != 0;
            if (is_pressed && !key_states[key_name]) {
                key_states[key_name] = true;
                key_events.push_back({key_name, true});
                LogToConsole("Tecla pressionada: " + key_name);
            } else if (!is_pressed && key_states[key_name]) {
                key_states[key_name] = false;
                key_events.push_back({key_name, false});
                LogToConsole("Tecla liberada: " + key_name);
            }
        }
    }

    // Capturar mouse, se habilitado
    if (capture_mouse) {
        POINT cursor_pos;
        GetCursorPos(&cursor_pos);
        bool left_click = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        bool right_click = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        mouse_events.push_back({cursor_pos.x, cursor_pos.y, left_click, right_click});
    }
}

// Renderiza a interface gráfica
void render_gui() {
    ImGui::Begin("MacroBubbleGum");
    ImGui::Text("=== Aviso ===");
    ImGui::Text("Shift e Ctrl nao sao suportados.");

    if (ImGui::Button(is_recording ? "Parar" : "Gravar")) {
        is_recording = !is_recording;
        if (is_recording) {
            recording_start = std::chrono::steady_clock::now();
            key_events.clear();
            mouse_events.clear();
            LogToConsole("Gravacao iniciada");
        } else {
            SaveInputsToFile();
            LogToConsole("Gravacao parada");
        }
    }

    ImGui::Text("Status: %s", is_recording ? "Gravando" : "Parado");

    if (ImGui::Checkbox("Capturar Mouse", &capture_mouse)) {
        LogToConsole("Estado de captura do mouse alterado.");
    }

    // Botão para limpar o console
    if (ImGui::Button("Limpar Console")) {
        console_logs.clear();
    }

    ImGui::Text("Console:");
    ImGui::BeginChild("ConsoleLogs", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::SetScrollHereY(1.0f); // Ajustar o scroll para acompanhar o texto
    for (const auto& log : console_logs) {
        ImGui::TextUnformatted(log.c_str());
    }
    ImGui::EndChild();
    ImGui::End();

    if (is_recording) {
        CaptureInputs();
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
