#include "orchestrator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int num_regular = 20;
    int num_emergency = 2;
    
    // Parse arguments simple
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--vehicles") == 0 && i + 1 < argc) {
            num_regular = atoi(argv[i+1]);
            // Escalar emergencias si son muchos vehiculos (opcional, dejamos en 2 si no se indica)
            if (num_regular >= 100) {
                num_emergency = 5;
            }
            i++;
        }
    }
    
    run_simulation(num_regular, num_emergency);
    
    return 0;
}
