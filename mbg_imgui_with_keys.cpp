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

// Estrutura para capturar eventos
struct Event {
    std::string type; // "KEYBOARD" ou "MOUSE"
    std::string key; // Nome da tecla, se aplicável
    bool is_press;   // true para pressionar, false para soltar (apenas teclado)
    std::string action; // "MOVE", "LEFT_CLICK", "RIGHT_CLICK" (apenas mouse)
    int x, y;       // Posição do mouse, se aplicável
    int timestamp;  // Timestamp em microsegundos
};

std::vector<Event> events;
bool is_recording = false;
bool capture_mouse = false;
std::chrono::steady_clock::time_point recording_start;

// Função para registrar logs no console
void LogToConsole(const std::string& message) {
    console_logs.push_back(message);
}

// Função para salvar entradas e mouse no arquivo
void SaveInputsAndMouseToFile() {
    std::ofstream file("inputs_mouse.txt");
    int last_timestamp = 0;

    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.timestamp < b.timestamp; // Ordenar por timestamp
    });

    for (const auto& event : events) {
        int sleep_time = event.timestamp - last_timestamp;
        if (sleep_time > 0) {
            file << "sleep_us(" << sleep_time << ")\n";
        }
        if (event.type == "KEYBOARD") {
            if (event.is_press) {
                file << "holdKey(\"" << event.key << "\")\n";
            } else {
                file << "releaseKey(\"" << event.key << "\")\n";
            }
        } else if (event.type == "MOUSE") {
            if (event.action == "MOVE") {
                file << "mouseMove(" << event.x << ", " << event.y << ")\n";
            } else if (event.action == "LEFT_CLICK") {
                file << "mouseClick(\"LEFT\", " << event.x << ", " << event.y << ")\n";
            } else if (event.action == "RIGHT_CLICK") {
                file << "mouseClick(\"RIGHT\", " << event.x << ", " << event.y << ")\n";
            }
        }
        last_timestamp = event.timestamp;
    }

    file.close();
    LogToConsole("Inputs e mouse salvos em inputs_mouse.txt");
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
    int current_time = std::chrono::duration_cast<std::chrono::microseconds>(now - recording_start).count();

    for (int vk_code = 0x01; vk_code <= 0xFE; ++vk_code) {
        std::string key_name = GetKeyName(vk_code);
        if (key_name != "UNKNOWN") {
            static std::unordered_map<std::string, bool> key_states;

            bool is_pressed = (GetAsyncKeyState(vk_code) & 0x8000) != 0;
            if (is_pressed && !key_states[key_name]) {
                key_states[key_name] = true;
                events.push_back({"KEYBOARD", key_name, true, "", 0, 0, current_time});
                LogToConsole("Tecla pressionada: " + key_name);
            } else if (!is_pressed && key_states[key_name]) {
                key_states[key_name] = false;
                events.push_back({"KEYBOARD", key_name, false, "", 0, 0, current_time});
                LogToConsole("Tecla liberada: " + key_name);
            }
        }
    }
}

// Capturar inputs do mouse
void CaptureMouseInputs() {
    auto now = std::chrono::steady_clock::now();
    int current_time = std::chrono::duration_cast<std::chrono::microseconds>(now - recording_start).count();

    POINT mouse_pos;
    GetCursorPos(&mouse_pos);

    static POINT last_mouse_pos = {0, 0};
    if (mouse_pos.x != last_mouse_pos.x || mouse_pos.y != last_mouse_pos.y) {
        events.push_back({"MOUSE", "", false, "MOVE", mouse_pos.x, mouse_pos.y, current_time});
        last_mouse_pos = mouse_pos;
    }

    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        events.push_back({"MOUSE", "", true, "LEFT_CLICK", mouse_pos.x, mouse_pos.y, current_time});
    }

    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
        events.push_back({"MOUSE", "", true, "RIGHT_CLICK", mouse_pos.x, mouse_pos.y, current_time});
    }
}

// Renderiza a interface gráfica
void render_gui() {
    ImGui::Begin("MacroBubbleGum");
    ImGui::Text("=== Aviso ===");
    ImGui::Text("As teclas Shift e Ctrl não são suportadas.");

    if (ImGui::Button(is_recording ? "Parar" : "Gravar")) {
        is_recording = !is_recording;
        if (is_recording) {
            recording_start = std::chrono::steady_clock::now();
            events.clear();
            LogToConsole("Gravacao iniciada");
        } else {
            SaveInputsAndMouseToFile();
            LogToConsole("Gravacao parada");
        }
    }

    ImGui::Text("Status: %s", is_recording ? "Gravando" : "Parado");

    // Botão para ativar ou desativar captura do mouse
    if (ImGui::Checkbox("Capturar Mouse", &capture_mouse)) {
        LogToConsole(capture_mouse ? "Captura de mouse ativada" : "Captura de mouse desativada");
    }

    // Botão para limpar o console
    if (ImGui::Button("Limpar Console")) {
        console_logs.clear();
    }

    ImGui::Text("Console:");
    ImGui::BeginChild("ConsoleLogs", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& log : console_logs) {
        ImGui::TextUnformatted(log.c_str());
    }
    ImGui::EndChild();
    ImGui::End();

    if (is_recording) {
        CaptureInputs();
        if (capture_mouse) {
            CaptureMouseInputs();
        }
    }
}

int main() {
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
