#include "globals.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// Definição das variáveis globais
bool is_playing = false; // Estado para saber se está rodando
bool is_paused = false;  // Estado para saber se está pausado

// Declaração da função `render_gui` (implementada em outro arquivo)
void render_gui();

int main(int, char**)
{
    // Inicializa o GLFW (para janelas e eventos de entrada)
    if (!glfwInit()) {
        return -1;
    }

    // Cria uma janela com contexto OpenGL
    GLFWwindow* window = glfwCreateWindow(1280, 720, "MacroBubbleGum", NULL, NULL);
    if (!window) {
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
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Verifica eventos de entrada

        // Inicia uma nova frame do ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Renderiza a interface gráfica
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
