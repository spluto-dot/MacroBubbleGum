
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>

// Estrutura para representar cada entrada de tecla e a duração em frames
struct InputEntry {
    std::string key;
    int frames;
};

std::vector<InputEntry> inputs; // Vetor para armazenar as entradas de tecla
bool is_playing = false; // Estado para saber se está rodando
bool is_paused = false; // Estado para saber se está pausado

// Função principal para configurar o GLFW e o ImGui
int main(int, char**)
{
    // Inicializa o GLFW (para janelas e eventos de entrada)
    if (!glfwInit())
        return -1;

    // Cria uma janela com contexto OpenGL
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Input Simulator", NULL, NULL);
    if (!window)
    {
        glfwTerminate(); // Se a janela falhar, encerra o GLFW
        return -1;
    }
    glfwMakeContextCurrent(window); // Define o contexto atual
    glfwSwapInterval(1); // Sincroniza com o monitor

    // Inicializa o contexto do ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Configurações do ImGui para OpenGL e GLFW
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Loop principal da aplicação
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents(); // Verifica eventos de entrada

        // Inicia uma nova frame do ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Função que renderiza a interface gráfica
        render_gui();

        // Renderiza o frame
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f); // Cor de fundo da janela
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window); // Troca os buffers (exibe na tela)
    }

    // Finaliza o ImGui e o GLFW
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// Função que renderiza a interface gráfica
void render_gui()
{
    ImGui::Begin("Input Simulator");

    // Define um array de teclas disponíveis
    static const char* keys[] = { "UP", "DOWN", "LEFT", "RIGHT", "A", "B", "X", "Y" };
    static int current_key = 0; // Tecla atualmente selecionada
    static int frame_count = 1; // Número de frames que a tecla ficará ativa

    // Botões de controle de simulação (Play, Pause, Stop)
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

    // Exibe o status atual (rodando, pausado ou parado)
    if (is_playing && !is_paused)
        ImGui::Text("Status: Playing");
    else if (is_paused)
        ImGui::Text("Status: Paused");
    else
        ImGui::Text("Status: Stopped");

    ImGui::Separator();

    // Seção para selecionar uma tecla e definir o número de frames
    ImGui::Text("Selecione a Tecla e o Número de Frames");
    ImGui::Combo("Tecla", &current_key, keys, IM_ARRAYSIZE(keys));
    ImGui::InputInt("Frames", &frame_count);
    if (frame_count < 1) frame_count = 1;

    // Adiciona a tecla à lista de inputs
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
