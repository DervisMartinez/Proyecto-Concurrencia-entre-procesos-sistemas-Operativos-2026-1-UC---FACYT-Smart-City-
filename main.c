/*
 * ============================================================================
 * main.c - Punto de entrada del simulador Smart City
 * ============================================================================
 *
 * Uso:
 *   ./smart_city                  → Simulación por defecto (20 reg + 2 emg)
 *   ./smart_city --vehicles 100   → 100 regulares + 5 emergencias
 *   ./smart_city --vehicles 50 --emergency 3  → 50 regulares + 3 emergencias
 *
 * El programa crea hilos para cada vehículo, los coordina mediante semáforos
 * POSIX en las intersecciones, y al final imprime un reporte de métricas.
 * ============================================================================
 */

#include "orchestrator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    /* Valores por defecto según el enunciado */
    int num_regular   = 20;  /* 20 vehículos autónomos regulares */
    int num_emergency = 2;   /* 2 vehículos de emergencia        */

    /* Parsear argumentos de línea de comandos */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--vehicles") == 0 && i + 1 < argc) {
            num_regular = atoi(argv[i + 1]);
            /* Escalar emergencias si hay muchos vehículos */
            if (num_regular >= 100) {
                num_emergency = 5;
            }
            i++;
        }
        else if (strcmp(argv[i], "--emergency") == 0 && i + 1 < argc) {
            num_emergency = atoi(argv[i + 1]);
            i++;
        }
    }

    /* Validación básica */
    if (num_regular < 1 || num_emergency < 0) {
        fprintf(stderr, "Uso: %s [--vehicles N] [--emergency M]\n", argv[0]);
        return 1;
    }

    /* Lanzar la simulación */
    run_simulation(num_regular, num_emergency);

    return 0;
}
