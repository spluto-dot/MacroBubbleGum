#include <stdio.h>
#include <windows.h>

// Definições de teclas
#define P1_RIGHT 0x44 // 'D' key

// Funções para simular ações de pressionar e soltar teclas
void holdKey(int key) {
    printf("holdKey(P1_RIGHT)\n");
    keybd_event(key, 0, 0, 0);
}

void releaseKey(int key) {
    printf("releaseKey(P1_RIGHT)\n");
    keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
}

void sleep(int ms) {
    printf("sleep(%d)\n", ms);
    Sleep(ms);
}

int main() {
    // Exemplo de captura e execução
    printf("// MacroBubbleGum Input Script\n\n");
    holdKey(P1_RIGHT);
    sleep(30);
    releaseKey(P1_RIGHT);
    sleep(30);
    holdKey(P1_RIGHT);
    sleep(30);
    releaseKey(P1_RIGHT);
    sleep(100);

    printf("Script gerado com sucesso!\n");
    return 0;
}
