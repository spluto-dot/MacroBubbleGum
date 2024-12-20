#include "globals.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>

// Estrutura para representar cada entrada de tecla e a duracao em frames
struct InputEntry {
    std::string key;
    int frames;
};

std::vector<InputEntry> inputs; // Vetor para armazenar as entradas de tecla

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

    // Define um array de teclas disponiveis
    static const char* keys[] = { "UP", "DOWN", "LEFT", "RIGHT", "A", "B", "X", "Y" };
    static int current_key = 0; // Tecla atualmente selecionada
    static int frame_count = 1; // Numero de frames que a tecla ficara ativa

    // Botoes de controle de simulacao (Play, Pause, Stop)
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
        saveInputsToFile(); // Salvar inputs ao parar
    }

    // Exibe o status atual (rodando, pausado ou parado)
    if (is_playing && !is_paused)
        ImGui::Text("Status: Playing");
    else if (is_paused)
        ImGui::Text("Status: Paused");
    else
        ImGui::Text("Status: Stopped");

    ImGui::Separator();

    // Secao para selecionar uma tecla e definir o numero de frames
    ImGui::Text("Selecione a Tecla e o Numero de Frames");
    ImGui::Combo("Tecla", &current_key, keys, IM_ARRAYSIZE(keys));
    ImGui::InputInt("Frames", &frame_count);
    if (frame_count < 1) frame_count = 1;

    // Adiciona a tecla a lista de inputs
    if (ImGui::Button("Adicionar Tecla")) {
        inputs.push_back({ keys[current_key], frame_count });
    }

    ImGui::Separator();

    // Exibe a lista de inputs
    ImGui::Text("Lista de Inputs:");
    for (size_t i = 0; i < inputs.size(); ++i) {
        ImGui::Text("Tecla: %s | Frames: %d", inputs[i].key.c_str(), inputs[i].frames);
    }

    ImGui::End();
}
