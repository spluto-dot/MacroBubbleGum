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
        if (key_name != "UNKNOWN") {
            if (GetAsyncKeyState(vk_code) & 0x8000) {
                // Tecla pressionada
                if (active_keys.find(key_name) == active_keys.end() || !active_keys[key_name]) {
                    printf("Tecla pressionada: %s\n", key_name.c_str());
                    inputs.push_back({key_name, "hold", elapsed_time});
                    active_keys[key_name] = true; // Marcar a tecla como ativa
                }
            } else {
                // Tecla liberada
                if (active_keys.find(key_name) != active_keys.end() && active_keys[key_name]) {
                    printf("Tecla liberada: %s\n", key_name.c_str());
                    inputs.push_back({key_name, "release", elapsed_time});
                    active_keys[key_name] = false; // Marcar a tecla como liberada
                }
            }
        }
    }
}

// Funcao para salvar os inputs em um arquivo
void saveInputsToFile() {
    FILE* file = fopen("inputs.txt", "w");
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
            start_time = std::chrono::steady_clock::now(); // Iniciar o contador
            printf("Gravacao iniciada\n");
        }
    } else {
        if (ImGui::Button("Parar")) {
            is_recording = false;

            // Finaliza qualquer tecla ainda ativa
            auto now = std::chrono::steady_clock::now();
            int elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
            for (const auto& [key, is_active] : active_keys) {
                if (is_active) {
                    printf("Tecla liberada (finalizando): %s\n", key.c_str());
                    inputs.push_back({key, "release", elapsed_time});
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
