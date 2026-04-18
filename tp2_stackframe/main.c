#include <stdio.h>
#include <stdlib.h>

// Declaramos la función ASM externa
extern int gini_calc(float gini);

int main(int argc, char *argv[]) {
    float gini;

    // Si recibe un argumento lo usa, sino usa valor por defecto
    if (argc > 1) {
        gini = atof(argv[1]);  // convierte string a float
    } else {
        gini = 42.7;           // valor por defecto para testing
    }

    printf("GINI recibido: %.1f\n", gini);

    // Llama a la función ASM
    int resultado = gini_calc(gini);

    printf("GINI convertido a entero + 1: %d\n", resultado);

    return 0;
}
