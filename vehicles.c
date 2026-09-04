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
        // Para emergencias cruzamos mas rapido
        if (v->is_emergency) {
            usleep(20 * 1000); // 20ms
        } else {
            // Si el vehiculo regular detecta que hay emergencia esperando, puede abortar/acelerar
            // Aqui simulamos que cruza normalmente
            usleep((rand() % 50 + 30) * 1000); // 30ms - 80ms
        }
        
        leave_intersection(intersection_id, v->is_emergency, v->id);
    }
    
    long end_time = get_current_time_ms();
    v->total_travel_time_ms = end_time - start_time;
    
    printf("[TIMESTAMP] [INFO] [VEHICULO %d] [%s] Llego a su destino. Tiempo total: %ld ms.\n", 
           v->id, tipo, v->total_travel_time_ms);
           
    return NULL;
}
