#include "intersection.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

Intersection intersections[NUM_INTERSECTIONS];

void init_intersections() {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        intersections[i].id = i;
        sem_init(&intersections[i].mutex, 0, 1);
        sem_init(&intersections[i].regular_sem, 0, 0);
        sem_init(&intersections[i].emergency_sem, 0, 0);
        
        intersections[i].vehicles_inside = 0;
        intersections[i].emergency_approaching = 0;
        
        intersections[i].regular_waiting_count = 0;
        intersections[i].regular_signaled_count = 0;
        
        intersections[i].emergency_waiting_count = 0;
        intersections[i].emergency_signaled_count = 0;
        
        intersections[i].deadlocks_avoided = 0;
        intersections[i].regular_queued_by_emergency = 0;
    }
}

void destroy_intersections() {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        sem_destroy(&intersections[i].mutex);
        sem_destroy(&intersections[i].regular_sem);
        sem_destroy(&intersections[i].emergency_sem);
    }
}

void enter_intersection(int id, int is_emergency, int vehicle_id) {
    Intersection* is = &intersections[id];
    sem_wait(&is->mutex);
    
    if (is_emergency) {
        is->emergency_approaching++;
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [EMERGENCIA] Solicitando acceso a Interseccion %d. Vehiculos dentro: %d\n", vehicle_id, id, is->vehicles_inside);
        
        is->emergency_waiting_count++;
        while (is->vehicles_inside > 0) {
            sem_post(&is->mutex);
            sem_wait(&is->emergency_sem);
            sem_wait(&is->mutex);
            if (is->emergency_signaled_count > 0) is->emergency_signaled_count--;
        }
        is->emergency_waiting_count--;
        is->vehicles_inside++;
        
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [EMERGENCIA] Entrando a Interseccion %d. (Cruce bloqueado para regulares)\n", vehicle_id, id);
    } 
    else {
        is->regular_waiting_count++;
        int deadlock_checked = 0;
        
        while (is->emergency_approaching > 0 || is->vehicles_inside >= MAX_CAPACITY) {
            if (is->emergency_approaching > 0) {
                is->regular_queued_by_emergency++;
            } else if (!deadlock_checked && is->regular_waiting_count >= 4 && is->vehicles_inside == 0) {
                is->deadlocks_avoided++;
                printf("[TIMESTAMP] [WARN] [SISTEMA] Deadlock evitado en Interseccion %d (4 vehiculos esperando). Cediendo paso ordenadamente.\n", id);
                deadlock_checked = 1;
            }
            
            sem_post(&is->mutex);
            sem_wait(&is->regular_sem);
            sem_wait(&is->mutex);
            if (is->regular_signaled_count > 0) is->regular_signaled_count--;
        }
        
        is->regular_waiting_count--;
        is->vehicles_inside++;
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [REGULAR] Entrando a Interseccion %d. Vehiculos dentro: %d\n", vehicle_id, id, is->vehicles_inside);
        
        // Cascaded wakeup
        if (is->emergency_approaching == 0 && is->vehicles_inside < MAX_CAPACITY && is->regular_waiting_count > is->regular_signaled_count) {
            is->regular_signaled_count++;
            sem_post(&is->regular_sem);
        }
    }
    
    sem_post(&is->mutex);
}

void leave_intersection(int id, int is_emergency, int vehicle_id) {
    Intersection* is = &intersections[id];
    sem_wait(&is->mutex);
    
    is->vehicles_inside--;
    
    if (is_emergency) {
        is->emergency_approaching--;
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [EMERGENCIA] Saliendo de Interseccion %d. Libera bloqueo.\n", vehicle_id, id);
    } else {
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [REGULAR] Saliendo de Interseccion %d.\n", vehicle_id, id);
    }
    
    // Wake up policy
    if (is->emergency_waiting_count > is->emergency_signaled_count && is->vehicles_inside == 0) {
        is->emergency_signaled_count++;
        sem_post(&is->emergency_sem);
    } 
    else if (is->emergency_approaching == 0 && is->regular_waiting_count > is->regular_signaled_count && is->vehicles_inside < MAX_CAPACITY) {
        is->regular_signaled_count++;
        sem_post(&is->regular_sem);
    }
    
    sem_post(&is->mutex);
}
