/*
 * ============================================================================
 * orchestrator.c - Orquestador Central de Tráfico
 * ============================================================================
 *
 * Este módulo administra todo el ciclo de vida de la simulación:
 *
 *   1. Inicializa las 5 intersecciones (semáforos y contadores)
 *   2. Genera dinámicamente los vehículos regulares como hilos
 *   3. Genera los vehículos de emergencia como hilos
 *   4. Espera a que TODOS los hilos terminen (pthread_join)
 *   5. Recopila las métricas de cada intersección y cada vehículo
 *   6. Imprime el REPORTE FINAL con los datos exigidos por el enunciado
 *   7. Libera toda la memoria y destruye los semáforos
 *
 * ============================================================================
 */

#include "orchestrator.h"
#include "intersection.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

/*
 * thread_rand() - Generador aleatorio thread-safe y portable
 * ----------------------------------------------------------
 * Misma implementación que en vehicles.c. Funciona en Linux, Windows
 * (MinGW) y macOS sin depender de funciones POSIX como rand_r().
 */
static int thread_rand(unsigned int* seed) {
    *seed = (*seed) * 1103515245 + 12345;
    return (int)((*seed / 65536) % 32768);
}

/*
 * generate_route()
 * ----------------
 * Genera una ruta aleatoria de 'length' intersecciones, evitando que
 * dos intersecciones consecutivas sean la misma (no tiene sentido
 * que un carro "cruce" la misma intersección dos veces seguidas).
 */
static void generate_route(int* route, int length, unsigned int* seed) {
    route[0] = thread_rand(seed) % NUM_INTERSECTIONS;
    for (int j = 1; j < length; j++) {
        do {
            route[j] = thread_rand(seed) % NUM_INTERSECTIONS;
        } while (route[j] == route[j - 1]); /* Evitar repetición consecutiva */
    }
}

/*
 * run_simulation()
 * ----------------
 * Función principal del orquestador. Ejecuta toda la simulación.
 */
void run_simulation(int num_regular, int num_emergency) {
    int total_vehicles = num_regular + num_emergency;
    unsigned int seed = (unsigned int)time(NULL);

    /* Reservar memoria para los hilos y sus argumentos */
    pthread_t*   threads = malloc(sizeof(pthread_t) * total_vehicles);
    VehicleArgs* args    = malloc(sizeof(VehicleArgs) * total_vehicles);

    if (!threads || !args) {
        fprintf(stderr, "[ORQUESTADOR] Error: no se pudo reservar memoria.\n");
        free(threads);
        free(args);
        return;
    }

    printf("=======================================================\n");
    printf("[ORQUESTADOR] Iniciando simulacion:\n");
    printf("  - Intersecciones: %d (capacidad maxima: %d cada una)\n",
           NUM_INTERSECTIONS, MAX_CAPACITY);
    printf("  - Vehiculos regulares: %d\n", num_regular);
    printf("  - Vehiculos de emergencia: %d\n", num_emergency);
    printf("=======================================================\n\n");

    /* Paso 1: Inicializar las intersecciones (semáforos y contadores) */
    init_intersections();

    int idx = 0; /* Índice del siguiente vehículo a crear */

    /*
     * Paso 2: Crear los hilos de vehículos REGULARES
     * -----------------------------------------------
     * Se crean de forma DINÁMICA con un pequeño retraso entre cada uno
     * para simular que los vehículos no aparecen todos al mismo instante.
     * Cada regular cruza 3 intersecciones aleatorias.
     */
    for (int i = 0; i < num_regular; i++) {
        args[idx].id                         = idx + 1;
        args[idx].is_emergency               = 0;
        args[idx].num_intersections_to_cross  = 3;
        args[idx].route                      = malloc(sizeof(int) * 3);
        args[idx].total_travel_time_ms       = 0;

        generate_route(args[idx].route, 3, &seed);

        /* Crear el hilo del vehículo regular */
        pthread_create(&threads[idx], NULL, vehicle_routine, &args[idx]);
        idx++;

        /* Retraso dinámico entre creación de vehículos (5-25ms) */
        usleep((unsigned int)(thread_rand(&seed) % 20 + 5) * 1000);
    }

    /*
     * Paso 3: Crear los hilos de vehículos de EMERGENCIA
     * ---------------------------------------------------
     * Las emergencias cruzan las 5 intersecciones en orden (0→1→2→3→4)
     * para maximizar la interacción con los regulares.
     */
    for (int i = 0; i < num_emergency; i++) {
        args[idx].id                         = idx + 1;
        args[idx].is_emergency               = 1;
        args[idx].num_intersections_to_cross  = NUM_INTERSECTIONS;
        args[idx].route                      = malloc(sizeof(int) * NUM_INTERSECTIONS);
        args[idx].total_travel_time_ms       = 0;

        /* Ruta fija: recorre todas las intersecciones en orden */
        for (int j = 0; j < NUM_INTERSECTIONS; j++) {
            args[idx].route[j] = j;
        }

        pthread_create(&threads[idx], NULL, vehicle_routine, &args[idx]);
        idx++;

        /* Separación entre emergencias (50ms) */
        usleep(50 * 1000);
    }

    /*
     * Paso 4: Esperar a que TODOS los hilos terminen
     * -----------------------------------------------
     * pthread_join() bloquea al orquestador hasta que el hilo indicado
     * haya terminado su ejecución. Recogemos las métricas de tiempo.
     */
    long total_time_regular   = 0;
    long total_time_emergency = 0;

    for (int i = 0; i < total_vehicles; i++) {
        pthread_join(threads[i], NULL);

        /* Acumular tiempos para calcular promedios */
        if (args[i].is_emergency) {
            total_time_emergency += args[i].total_travel_time_ms;
        } else {
            total_time_regular += args[i].total_travel_time_ms;
        }
    }

    /*
     * Paso 5: Recopilar métricas globales desde las intersecciones
     * ------------------------------------------------------------
     * Cada intersección lleva sus propios contadores de deadlocks evitados
     * y regulares encolados por emergencia. Los sumamos.
     */
    int total_deadlocks_avoided = 0;
    int total_queued_by_emg     = 0;

    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        total_deadlocks_avoided += intersections[i].deadlocks_avoided;
        total_queued_by_emg     += intersections[i].regular_queued_by_emergency;
    }

    /*
     * Paso 6: Imprimir el REPORTE FINAL
     * ----------------------------------
     * El enunciado pide:
     *   a) Tiempo promedio regulares vs emergencias
     *   b) Deadlocks evitados
     *   c) Regulares encolados por emergencia
     */
    printf("\n=======================================================\n");
    printf("         REPORTE FINAL DE LA SIMULACION\n");
    printf("              (ORQUESTADOR CENTRAL)\n");
    printf("=======================================================\n");

    if (num_regular > 0) {
        printf("a) Tiempo promedio vehiculos REGULARES:     %ld ms\n",
               total_time_regular / num_regular);
    }
    if (num_emergency > 0) {
        printf("   Tiempo promedio vehiculos EMERGENCIA:    %ld ms\n",
               total_time_emergency / num_emergency);
    }
    printf("b) Deadlocks evitados (situacion 4-way):    %d\n",
           total_deadlocks_avoided);
    printf("c) Regulares encolados por emergencia:      %d\n",
           total_queued_by_emg);
    printf("   Total vehiculos simulados:               %d\n",
           total_vehicles);
    printf("=======================================================\n");

    /*
     * Paso 7: Liberar recursos
     * ------------------------
     * Liberar las rutas dinámicas, los arreglos y destruir semáforos.
     */
    for (int i = 0; i < total_vehicles; i++) {
        free(args[i].route);
    }
    destroy_intersections();
    free(threads);
    free(args);
}
