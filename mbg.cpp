
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

bool is_playing = false; // Estado para saber se está rodando
bool is_paused = false; // Estado para saber se está pausado

// Função para renderizar a interface gráfica
void render_gui()
{
    ImGui::Begin("MacroBubbleGum");

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

    // Espaço reservado para Tabela de Inputs (em desenvolvimento)
    ImGui::Text("Tabela de Inputs (em desenvolvimento)");

    ImGui::End();
}

// Função principal para configurar o GLFW e o ImGui
int main(int, char**)
{
    // Inicializa o GLFW (para janelas e eventos de entrada)
    if (!glfwInit())
        return -1;

    // Cria uma janela com contexto OpenGL
    GLFWwindow* window = glfwCreateWindow(1280, 720, "MacroBubbleGum", NULL, NULL);
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
