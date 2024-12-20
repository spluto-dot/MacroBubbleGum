#include "globals.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <windows.h> // Para capturar teclas

// Estrutura para representar cada entrada de tecla e a duracao em frames
struct InputEntry {
    std::string key;
    int frames;
};

std::vector<InputEntry> inputs; // Vetor para armazenar as entradas de tecla
bool is_recording = false;      // Flag para saber se está gravando

// Mapear as teclas para nomes legíveis
std::string getKeyName(int vk_code) {
    switch (vk_code) {
        case VK_UP: return "UP";
        case VK_DOWN: return "DOWN";
        case VK_LEFT: return "LEFT";
        case VK_RIGHT: return "RIGHT";
        case 'A': return "A";
        case 'B': return "B";
        case 'X': return "X";
        case 'Y': return "Y";
        default: return "UNKNOWN";
    }
}

// Capturar inputs do teclado
void captureInputs() {
    for (int vk_code = 0x01; vk_code <= 0xFE; ++vk_code) {
        if (GetAsyncKeyState(vk_code) & 0x8000) {
            std::string key_name = getKeyName(vk_code);
            if (!key_name.empty() && (inputs.empty() || inputs.back().key != key_name)) {
                inputs.push_back({key_name, 1}); // Adiciona a tecla pressionada com 1 frame
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
            fprintf(file, "sleep(%d)\n", input.frames);
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
            printf("Gravacao iniciada\n");
        }
    } else {
        if (ImGui::Button("Parar")) {
            is_recording = false;
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
