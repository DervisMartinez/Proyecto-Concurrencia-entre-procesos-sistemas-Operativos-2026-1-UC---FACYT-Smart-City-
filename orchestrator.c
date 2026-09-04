#include "orchestrator.h"
#include "intersection.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void run_simulation(int num_regular, int num_emergency) {
    int total_vehicles = num_regular + num_emergency;
    pthread_t* threads = malloc(sizeof(pthread_t) * total_vehicles);
    VehicleArgs* args = malloc(sizeof(VehicleArgs) * total_vehicles);
    
    printf("[ORQUESTADOR] Iniciando simulacion con %d regulares y %d emergencias.\n", num_regular, num_emergency);
    
    init_intersections();
    
    // Seed random
    srand(time(NULL));
    
    int thread_idx = 0;
    
    // Lanza vehiculos regulares
    for (int i = 0; i < num_regular; i++) {
        args[thread_idx].id = thread_idx + 1;
        args[thread_idx].is_emergency = 0;
        args[thread_idx].num_intersections_to_cross = 3; // cada vehiculo cruza 3 intersecciones
        args[thread_idx].route = malloc(sizeof(int) * 3);
        
        // Generar ruta aleatoria (previniendo cruzar la misma inmediatamente)
        for (int j = 0; j < 3; j++) {
            args[thread_idx].route[j] = rand() % NUM_INTERSECTIONS;
        }
        
        pthread_create(&threads[thread_idx], NULL, vehicle_routine, &args[thread_idx]);
        thread_idx++;
        
        // Espaciado aleatorio para generacion dinamica
        usleep((rand() % 20 + 5) * 1000); 
    }
    
    // Lanzar vehiculos de emergencia en algun momento
    for (int i = 0; i < num_emergency; i++) {
        args[thread_idx].id = thread_idx + 1;
        args[thread_idx].is_emergency = 1;
        args[thread_idx].num_intersections_to_cross = 5; // Emergencias cruzan todas
        args[thread_idx].route = malloc(sizeof(int) * 5);
        
        // Emergencia cruza las 5 intersecciones
        for (int j = 0; j < 5; j++) {
            args[thread_idx].route[j] = j; // Ruta 0, 1, 2, 3, 4
        }
        
        pthread_create(&threads[thread_idx], NULL, vehicle_routine, &args[thread_idx]);
        thread_idx++;
        usleep(50 * 1000); // 50ms de diferencia entre emergencias
    }
    
    // Esperar a que terminen
    long total_time_reg = 0;
    long total_time_emg = 0;
    
    for (int i = 0; i < total_vehicles; i++) {
        pthread_join(threads[i], NULL);
        if (args[i].is_emergency) {
            total_time_emg += args[i].total_travel_time_ms;
        } else {
            total_time_reg += args[i].total_travel_time_ms;
        }
        free(args[i].route);
    }
    
    // Contabilizar metricas globales desde intersections
    int total_deadlocks_avoided = 0;
    int total_queued_by_emg = 0;
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        total_deadlocks_avoided += intersections[i].deadlocks_avoided;
        total_queued_by_emg += intersections[i].regular_queued_by_emergency;
    }
    
    // Imprimir el reporte final
    printf("\n=======================================================\n");
    printf("REPORTE FINAL DE LA SIMULACION (ORQUESTADOR)\n");
    printf("=======================================================\n");
    
    if (num_regular > 0) {
        printf("a) Tiempo promedio vehiculos regulares: %ld ms\n", total_time_reg / num_regular);
    }
    if (num_emergency > 0) {
        printf("a) Tiempo promedio vehiculos emergencia: %ld ms\n", total_time_emg / num_emergency);
    }
    printf("b) Contador de 'Deadlocks evitados' (situacion 4-way): %d\n", total_deadlocks_avoided);
    printf("c) Vehiculos regulares encolados por emergencia: %d\n", total_queued_by_emg);
    printf("=======================================================\n");
    
    destroy_intersections();
    free(threads);
    free(args);
}
