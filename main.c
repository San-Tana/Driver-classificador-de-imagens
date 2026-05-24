#include <stdio.h>
#include "driver.h"

// Verifica se um arquivo existe e pode ser aberto antes de tentar usá-lo.
int verificar_arquivo(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("ERRO: arquivo %s nao encontrado.\n", path);
        return 0;
    }
    fclose(f);
    return 1;
}

int main(void) {
    const char *peso = "data/w_in_q.bin";
    const char *bias = "data/b_q.bin";
    const char *beta = "data/beta_q.bin";
    char img[32];
    int i, resultado, esperado, acertos;

    // Verifica se os arquivos de parâmetros da rede existem antes de qualquer coisa
    if (!verificar_arquivo(peso)) return 1;
    printf("Pesos carregados.\n");
    if (!verificar_arquivo(bias)) return 1;
    printf("Bias carregado.\n");
    if (!verificar_arquivo(beta)) return 1;
    printf("Beta carregado.\n\n");

    // Mapeia a ponte HPS-FPGA e inicializa o coprocessador
    mapear_fpga();
    reset_coprocessador();

    // Parâmetros da rede são enviados uma única vez — não mudam entre imagens
    enviar_bias(bias);
    enviar_beta(beta);
    enviar_peso(peso);

    acertos = 0;

    // Loop principal: para cada uma das 100 imagens, envia e classifica
    for (i = 0; i <= 99; i++) {
        snprintf(img, sizeof(img), "data/%d.bin", i);
        if (!verificar_arquivo(img)) return 1;

        enviar_img(img);
        resultado = iniciar_inferencia();

        // O dígito esperado é determinado pelo índice: imagens 0-9 são do dígito 0,
        // 10-19 do dígito 1, e assim por diante
        esperado = i / 10;

        if (resultado == esperado) {
            printf("Imagem %d (digito %d): Predito %d OK\n", i, esperado, resultado);
            acertos++;
        } else {
            printf("Imagem %d (digito %d): Predito %d ERRO\n", i, esperado, resultado);
        }

        // Reset ao final de cada inferência para limpar o estado do coprocessador
        reset_coprocessador();
    }

    // Exibe o resultado final, como são 100 imagens, acertos == porcentagem
    printf("\nAcertos: %d/100\n", acertos);
    printf("Acuracia: %d%%\n", acertos);

    return 0;
}
