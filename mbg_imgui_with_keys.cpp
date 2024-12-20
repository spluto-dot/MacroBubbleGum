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

// Estrutura para representar cada entrada de tecla e a duracao em ms
struct InputEntry {
    std::string key;
    int duration_ms;
};

std::vector<InputEntry> inputs; // Vetor para armazenar as entradas de tecla
bool is_recording = false;      // Flag para saber se está gravando
std::unordered_map<std::string, std::pair<std::chrono::steady_clock::time_point, bool>> active_keys; // Map para rastrear teclas pressionadas e seu status

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

    for (int vk_code = 0x01; vk_code <= 0xFE; ++vk_code) {
        std::string key_name = getKeyName(vk_code);
        if (key_name != "UNKNOWN") {
            if (GetAsyncKeyState(vk_code) & 0x8000) {
                // Tecla pressionada
                if (active_keys.find(key_name) == active_keys.end() || !active_keys[key_name].second) {
                    printf("Tecla pressionada: %s\n", key_name.c_str()); // Log de depuração
                    active_keys[key_name] = {now, true}; // Registrar o momento em que a tecla foi pressionada e marcar como ativa
                }
            } else {
                // Tecla liberada
                if (active_keys.find(key_name) != active_keys.end() && active_keys[key_name].second) {
                    auto press_time = active_keys[key_name].first;
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - press_time).count();
                    printf("Tecla liberada: %s, Duracao: %d ms\n", key_name.c_str(), static_cast<int>(duration)); // Log de depuração da duração
                    inputs.push_back({key_name, static_cast<int>(duration)});
                    active_keys[key_name].second = false; // Marcar a tecla como liberada
                }
            }
        }
    }
}

// Funcao para salvar os inputs em um arquivo
void saveInputsToFile() {
    FILE* file = fopen("inputs.txt", "w");
    if (file) {
        for (const auto& input : inputs) {
            fprintf(file, "holdKey(\"%s\")\n", input.key.c_str());
            fprintf(file, "sleep(%d)\n", input.duration_ms);
            fprintf(file, "releaseKey(\"%s\")\n", input.key.c_str());
        }
        fclose(file);
        printf("Inputs salvos em inputs.txt\n");
    } else {
        printf("Erro ao salvar inputs!\n");
    }
}

// Funcao que renderiza a interface grafica
void render_gui() {
    ImGui::Begin("MacroBubbleGum");

    // Botao de gravacao
    if (!is_recording) {
        if (ImGui::Button("Gravar")) {
            is_recording = true;
            inputs.clear();
            active_keys.clear();
            printf("Gravacao iniciada\n");
        }
    } else {
        if (ImGui::Button("Parar")) {
            is_recording = false;

            // Finaliza qualquer tecla ainda ativa
            auto now = std::chrono::steady_clock::now();
            for (const auto& [key, value] : active_keys) {
                if (value.second) {
                    auto press_time = value.first;
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - press_time).count();
                    printf("Tecla liberada (finalizando): %s, Duracao: %d ms\n", key.c_str(), static_cast<int>(duration)); // Log de depuração na finalização
                    inputs.push_back({key, static_cast<int>(duration)});
                }
            }
            active_keys.clear();

            saveInputsToFile();
            printf("Gravacao parada\n");
        }
    }

    ImGui::Separator();

    // Exibe o status atual
    if (is_recording)
        ImGui::Text("Status: Gravando");
    else
        ImGui::Text("Status: Parado");

    ImGui::End();

    // Captura inputs enquanto grava
    if (is_recording) {
        captureInputs();
    }
}
