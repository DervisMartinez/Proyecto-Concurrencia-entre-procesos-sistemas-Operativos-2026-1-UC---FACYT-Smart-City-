/*
 * vehicles.c - Comportamiento de los hilos de vehículos
 * ============================================================================
 *
 * Cada vehículo (hilo) recorre una ruta predefinida de intersecciones.
 * Para cada intersección de su ruta:
 *   1. Viaja hasta la intersección (simulado con usleep)
 *   2. Pide acceso (enter_intersection) - puede bloquearse aquí
 *   3. Cruza la intersección (simulado con usleep)
 *   4. Sale (leave_intersection) - puede despertar a otros hilos
 *
 * La diferencia entre regular y emergencia es:
 *   - Las emergencias tienen prioridad absoluta: la intersección se vacía
 *     antes de dejarlas pasar.
 *   - Los regulares implementan "herencia de prioridad": si detectan una
 *     emergencia mientras cruzan, aceleran su cruce para liberar rápido.
 * ============================================================================
 */

#include "vehicles.h"
#include "intersection.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/*
 * thread_rand() - Generador aleatorio thread-safe y portable
 */
static int thread_rand(unsigned int* seed) {
    *seed = (*seed) * 1103515245 + 12345;
    return (int)((*seed / 65536) % 32768);
}

/*
 * get_current_time_ms()
 * ---------------------
 * Devuelve el tiempo actual en milisegundos desde epoch.
 * Se usa para medir el tiempo de viaje de cada vehículo.
 */
static long get_current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000);
}

/*
 * vehicle_routine()
 * -----------------
 * Función principal que ejecuta cada hilo de vehículo.
 *
 * Parámetro: void* arg → puntero a VehicleArgs con la configuración
 *            del vehículo (ID, tipo, ruta, etc.)
 *
 * Retorna: NULL (no se usa el valor de retorno del hilo)
 */
void* vehicle_routine(void* arg) {
    VehicleArgs* v = (VehicleArgs*)arg;

    /* Semilla propia para rand_r() → thread-safe (cada hilo tiene la suya) */
    unsigned int seed = (unsigned int)(time(NULL) ^ (long)pthread_self() ^ v->id);

    /* Registrar el tiempo de inicio para calcular el tiempo total */
    long start_time = get_current_time_ms();

    /* Determinar el tipo para los mensajes de log */
    const char* tipo = v->is_emergency ? "EMERGENCIA" : "REGULAR";

    printf("[TIMESTAMP] [INFO] [VEHICULO %d] [%s] "
           "Inicia recorrido de %d intersecciones.\n",
           v->id, tipo, v->num_intersections_to_cross);

    /* ── Recorrer cada intersección de la ruta ── */
    for (int i = 0; i < v->num_intersections_to_cross; i++) {
        int inter_id = v->route[i];

        /*
         * FASE 1: Viajar hasta la intersección
         * Simulamos el tiempo de tránsito entre intersecciones.
         * thread_rand() es thread-safe (usa semilla propia, no estado global).
         */
        usleep((unsigned int)(thread_rand(&seed) % 50 + 10) * 1000); /* 10-60 ms */

        /*
         * FASE 2: Solicitar acceso a la intersección
         * Esta llamada puede BLOQUEAR al hilo si:
         *   - La intersección está llena (2 vehículos dentro)
         *   - Hay una emergencia aproximándose
         */
        enter_intersection(inter_id, v->is_emergency, v->id);

        /*
         * FASE 3: Cruzar la intersección (estamos DENTRO, ocupando capacidad)
         *
         * HERENCIA DE PRIORIDAD (Priority Inheritance):
         * Si un regular está cruzando y detecta que una emergencia se
         * está acercando, "hereda" la prioridad alta y acelera su cruce.
         * Esto evita la INVERSIÓN DE PRIORIDAD: que la emergencia espere
         * mucho porque un regular lento la bloquea.
         */
        if (v->is_emergency) {
            /* Las emergencias cruzan rápido (20ms) */
            usleep(20 * 1000);
        } else {
            /* Regular: verificar si hay emergencia acercándose */
            if (intersections[inter_id].emergency_approaching > 0) {
                printf("[TIMESTAMP] [INFO] [VEHICULO %d] [REGULAR] "
                       "Acelerando cruce en Interseccion %d "
                       "(Priority Inheritance por emergencia)!\n",
                       v->id, inter_id);
                usleep(5 * 1000); /* Cruce expeditado: solo 5ms */
            } else {
                /* Cruce normal: 30-80ms */
                usleep((unsigned int)(thread_rand(&seed) % 50 + 30) * 1000);
            }
        }

        /*
         * FASE 4: Salir de la intersección
         * Libera una plaza y puede despertar a hilos bloqueados.
         */
        leave_intersection(inter_id, v->is_emergency, v->id);
    }

    /* ── Calcular el tiempo total de viaje ── */
    long end_time = get_current_time_ms();
    v->total_travel_time_ms = end_time - start_time;

    printf("[TIMESTAMP] [INFO] [VEHICULO %d] [%s] "
           "Llego a su destino. Tiempo total: %ld ms.\n",
           v->id, tipo, v->total_travel_time_ms);

    return NULL;
}
