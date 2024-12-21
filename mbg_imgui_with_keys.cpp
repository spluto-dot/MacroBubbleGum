#include "globals.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <windows.h> // Para capturar teclas
#include <chrono> // Para medir duracao
#include <ctime>  // Para formatar data e hora

// Estrutura para representar cada entrada de tecla e o evento
struct InputEvent {
    std::string key;
    std::string action; // "hold" ou "release"
    int timestamp_ms;
};

std::vector<InputEvent> inputs; // Vetor para armazenar os eventos de entrada
bool is_recording = false;      // Flag para saber se está gravando
std::unordered_map<std::string, bool> active_keys; // Map para rastrear teclas pressionadas
std::chrono::steady_clock::time_point start_time; // Tempo inicial da gravação
std::vector<std::string> log_messages; // Mensagens para o console integrado

// Adiciona mensagens ao console interno
void addLogMessage(const std::string& message) {
    log_messages.push_back(message);
    if (log_messages.size() > 100) {
        log_messages.erase(log_messages.begin()); // Limitar o tamanho do log
    }
}

// Mapear as teclas para nomes legíveis
std::string getKeyName(int vk_code) {
    if ((vk_code >= 0x30 && vk_code <= 0x39) || (vk_code >= 0x41 && vk_code <= 0x5A)) {
        return std::string(1, static_cast<char>(vk_code)); // Letras e numeros
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
void captureInputs() {
    auto now = std::chrono::steady_clock::now();
    int elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

    for (int vk_code = 0x01; vk_code <= 0xFE; ++vk_code) {
        std::string key_name = getKeyName(vk_code);
        if (key_name != "UNKNOWN" && vk_code != VK_SHIFT && vk_code != VK_CONTROL) {
            if (GetAsyncKeyState(vk_code) & 0x8000) {
                // Tecla pressionada
                if (active_keys.find(key_name) == active_keys.end() || !active_keys[key_name]) {
                    addLogMessage("Tecla pressionada: " + key_name);
                    inputs.push_back({key_name, "hold", elapsed_time});
                    active_keys[key_name] = true; // Marcar a tecla como ativa
                }
            } else {
                // Tecla liberada
                if (active_keys.find(key_name) != active_keys.end() && active_keys[key_name]) {
                    addLogMessage("Tecla liberada: " + key_name);
                    inputs.push_back({key_name, "release", elapsed_time});
                    active_keys[key_name] = false; // Marcar a tecla como liberada
                }
            }
        }
    }
}

// Gera o nome do arquivo baseado na data e hora
std::string generateFileName() {
    auto now = std::time(nullptr);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%d%m%Y%H%M.txt", std::localtime(&now));
    return std::string(buffer);
}

// Funcao para salvar os inputs em um arquivo
void saveInputsToFile() {
    std::string filename = generateFileName();
    FILE* file = fopen(filename.c_str(), "w");
    if (file) {
        int last_timestamp = 0;
        for (const auto& input : inputs) {
            int sleep_time = input.timestamp_ms - last_timestamp;
            if (sleep_time > 0) {
                fprintf(file, "sleep(%d)\n", sleep_time);
            }

            if (input.action == "hold") {
                fprintf(file, "holdKey(\"%s\")\n", input.key.c_str());
            } else if (input.action == "release") {
                fprintf(file, "releaseKey(\"%s\")\n", input.key.c_str());
            }

            last_timestamp = input.timestamp_ms;
        }
        fclose(file);
        addLogMessage("Inputs salvos em " + filename);
    } else {
        addLogMessage("Erro ao salvar inputs!");
    }
}

// Funcao que renderiza a interface grafica
void render_gui() {
    ImGui::Begin("MacroBubbleGum");

    // Botao de gravacao
    if (!is_recording) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Botao vermelho
        if (ImGui::Button("Gravar")) {
            is_recording = true;
            inputs.clear();
            active_keys.clear();
            start_time = std::chrono::steady_clock::now(); // Iniciar o contador
            addLogMessage("Gravacao iniciada");
        }
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Botao cinza
        if (ImGui::Button("Parar")) {
            is_recording = false;

            // Finaliza qualquer tecla ainda ativa
            auto now = std::chrono::steady_clock::now();
            int elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
            for (const auto& [key, is_active] : active_keys) {
                if (is_active) {
                    addLogMessage("Tecla liberada (finalizando): " + key);
                    inputs.push_back({key, "release", elapsed_time});
                }
            }
            active_keys.clear();

            saveInputsToFile();
            addLogMessage("Gravacao parada");
        }
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    // Exibe o status atual e o tempo decorrido
    if (is_recording) {
        auto now = std::chrono::steady_clock::now();
        int elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        ImGui::Text("Status: Gravando (%d ms)", elapsed_time);
    } else {
        ImGui::Text("Status: Parado");
    }

    ImGui::Separator();

    // Exibir log interno
    ImGui::Text("Console:");
    ImGui::BeginChild("Log", ImVec2(0, 150), true);
    for (const auto& message : log_messages) {
        ImGui::TextWrapped("%s", message.c_str());
    }
    ImGui::EndChild();

    ImGui::End();

    // Captura inputs enquanto grava
    if (is_recording) {
        captureInputs();
    }

    // Janela separada para mensagem de aviso
    ImGui::Begin("Aviso");
    ImGui::Text("Shift e Ctrl nao sao suportados.");
    ImGui::End();
}
