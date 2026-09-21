/*
 * ============================================================================
 * vehicles.h - Definiciones para los hilos de vehículos
 * ============================================================================
 *
 * Cada vehículo (regular o de emergencia) es un HILO INDEPENDIENTE (pthread).
 * Esta estructura contiene los datos que se le pasan a cada hilo al crearlo.
 * ============================================================================
 */
#ifndef VEHICLES_H
#define VEHICLES_H

#include <pthread.h>
#include <sys/time.h>

/*
 * VehicleArgs: Estructura de argumentos para cada hilo de vehículo.
 *
 * Se crea UNA por vehículo en el orquestador y se pasa como argumento
 * a pthread_create(). El hilo la lee durante su ejecución y al finalizar
 * escribe el tiempo total de viaje para que el orquestador lo recoja.
 */
typedef struct {
    int  id;                         /* ID único del vehículo (1, 2, 3...)    */
    int  is_emergency;               /* 1 = emergencia, 0 = regular          */
    int  num_intersections_to_cross; /* Cuántas intersecciones debe cruzar   */
    int* route;                      /* Arreglo dinámico con los IDs de las
                                        intersecciones a cruzar en orden     */

    /* Métrica de salida (escrita por el hilo al terminar) */
    long total_travel_time_ms;       /* Tiempo total de viaje en milisegundos*/
} VehicleArgs;

/*
 * vehicle_routine()
 * -----------------
 * Función que ejecuta cada hilo de vehículo. Recorre su ruta, entrando
 * y saliendo de cada intersección. Se pasa como argumento a pthread_create().
 */
void* vehicle_routine(void* arg);

#endif /* VEHICLES_H */
