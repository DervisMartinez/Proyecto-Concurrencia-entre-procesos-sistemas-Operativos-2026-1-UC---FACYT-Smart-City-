#include "vehicles.h"
#include "intersection.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

long get_current_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void* vehicle_routine(void* arg) {
    VehicleArgs* v = (VehicleArgs*)arg;
    long start_time = get_current_time_ms();
    
    const char* tipo = v->is_emergency ? "EMERGENCIA" : "REGULAR";
    printf("[TIMESTAMP] [INFO] [VEHICULO %d] [%s] Inicia recorrido de %d intersecciones.\n", 
           v->id, tipo, v->num_intersections_to_cross);
           
    for (int i = 0; i < v->num_intersections_to_cross; i++) {
        int intersection_id = v->route[i];
        
        // Simular tiempo de viaje hasta la interseccion
        usleep((rand() % 50 + 10) * 1000); // 10ms - 60ms
        
        enter_intersection(intersection_id, v->is_emergency, v->id);
        
        // Simular tiempo cruzando la interseccion
        if (v->is_emergency) {
            usleep(20 * 1000); // 20ms
        } else {
            // Priority Inheritance Implementation
            // Si un vehiculo regular esta en la seccion critica y una emergencia se aproxima,
            // hereda la prioridad temporalmente acelerando su cruce para liberar el recurso rapido.
            if (intersections[intersection_id].emergency_approaching > 0) {
                printf("[TIMESTAMP] [INFO] [VEHICULO %d] [REGULAR] ¡Acelerando cruce por ambulancia (Priority Inheritance)!\n", v->id);
                usleep(5 * 1000); // Cruce expeditado
            } else {
                usleep((rand() % 50 + 30) * 1000); // 30ms - 80ms normal
            }
        }
        
        leave_intersection(intersection_id, v->is_emergency, v->id);
    }
    
    long end_time = get_current_time_ms();
    v->total_travel_time_ms = end_time - start_time;
    
    printf("[TIMESTAMP] [INFO] [VEHICULO %d] [%s] Llego a su destino. Tiempo total: %ld ms.\n", 
           v->id, tipo, v->total_travel_time_ms);
           
    return NULL;
}
