/*
 * ============================================================================
 * intersection.h - Definiciones para el control de intersecciones
 * ============================================================================
 * 
 * Cada intersección es un RECURSO COMPARTIDO protegido por semáforos POSIX.
 * 
 * Modelo de sincronización utilizado: "Monitor con semáforos"
 *   - Un semáforo binario (mutex) protege las variables de estado.
 *   - Semáforos de condición (regular_queue, emergency_queue) bloquean a los
 *     hilos que no pueden entrar hasta que se les señalice explícitamente.
 *   - Contadores de señales pendientes (signaled) evitan señales perdidas
 *     (lost wakeup) que son la causa #1 de cuelgues/deadlocks en este tipo
 *     de problemas.
 * ============================================================================
 */
#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <semaphore.h>

/* Parámetros de la simulación según el enunciado */
#define NUM_INTERSECTIONS 5   /* 5 intersecciones de alta densidad            */
#define MAX_CAPACITY      2   /* Solo 2 vehículos no colisionantes a la vez   */

/*
 * Estructura que representa una intersección.
 * Cada intersección actúa como un "monitor" implementado con semáforos.
 */
typedef struct {
    int id;                       /* Identificador de la intersección (0-4)   */

    /* --- Semáforos de sincronización --- */
    sem_t mutex;                  /* Semáforo binario: exclusión mutua sobre
                                     todas las variables de estado internas   */
    sem_t regular_queue;          /* Cola de espera para vehículos regulares.
                                     Inicializado en 0: los hilos hacen
                                     sem_wait() aquí para bloquearse          */
    sem_t emergency_queue;        /* Cola de espera para vehículos de emergencia.
                                     Inicializado en 0: misma lógica          */

    /* --- Variables de estado protegidas por mutex --- */
    int vehicles_inside;          /* Cuántos vehículos están DENTRO ahora     */
    int emergency_approaching;    /* Cuántas emergencias están acercándose.
                                     Mientras sea > 0, NO entran regulares    */

    int regular_waiting;          /* Regulares bloqueados en regular_queue     */
    int regular_signaled;         /* Señales pendientes de consumir en
                                     regular_queue (evita lost wakeup)        */

    int emergency_waiting;        /* Emergencias bloqueadas en emergency_queue */
    int emergency_signaled;       /* Señales pendientes en emergency_queue    */

    /* --- Contadores de métricas para el reporte --- */
    int deadlocks_avoided;        /* Veces que se detectó/evitó un posible
                                     deadlock (ej: 4-way simultáneo)          */
    int regular_queued_by_emergency; /* Veces que un regular fue encolado
                                        por paso de emergencia               */
} Intersection;

/* Arreglo global de intersecciones (definido en intersection.c) */
extern Intersection intersections[NUM_INTERSECTIONS];

/* --- API pública --- */
void init_intersections(void);
void destroy_intersections(void);
void enter_intersection(int intersection_id, int is_emergency, int vehicle_id);
void leave_intersection(int intersection_id, int is_emergency, int vehicle_id);

#endif /* INTERSECTION_H */
