#include "intersection.h"
#include <stdio.h>
#include <stdlib.h>

/* Arreglo global de las 5 intersecciones */
Intersection intersections[NUM_INTERSECTIONS];

/*
 * init_intersections()
 * --------------------
 * Inicializa todas las intersecciones: semáforos en su valor inicial y
 * contadores en cero. Se llama UNA sola vez antes de crear los hilos.
 */
void init_intersections(void) {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        intersections[i].id = i;

        /* mutex = 1: semáforo binario, solo 1 hilo puede modificar estado */
        sem_init(&intersections[i].mutex, 0, 1);

        /* Colas de espera inicializadas en 0: nadie puede pasar hasta que
           alguien haga sem_post() explícitamente para despertarlo */
        sem_init(&intersections[i].regular_queue, 0, 0);
        sem_init(&intersections[i].emergency_queue, 0, 0);

        intersections[i].vehicles_inside       = 0;
        intersections[i].emergency_approaching = 0;

        intersections[i].regular_waiting   = 0;
        intersections[i].regular_signaled  = 0;

        intersections[i].emergency_waiting  = 0;
        intersections[i].emergency_signaled = 0;

        intersections[i].deadlocks_avoided          = 0;
        intersections[i].regular_queued_by_emergency = 0;
    }
}

/*
 * destroy_intersections()
 * -----------------------
 * Libera los recursos de semáforos. Se llama al final, después de que todos
 * los hilos hayan terminado (pthread_join completado).
 */
void destroy_intersections(void) {
    for (int i = 0; i < NUM_INTERSECTIONS; i++) {
        sem_destroy(&intersections[i].mutex);
        sem_destroy(&intersections[i].regular_queue);
        sem_destroy(&intersections[i].emergency_queue);
    }
}

/*
 * signal_waiters()
 * ----------------
 * Función auxiliar interna. Después de que un vehículo sale o entra,
 * evalúa si hay hilos bloqueados que ahora pueden pasar y les envía
 * la señal (sem_post) correspondiente.
 *
 * POLÍTICA DE DESPERTAR (prioridad absoluta para emergencias):
 *   1. Si hay emergencias esperando Y la intersección está VACÍA → despertar
 *      UNA emergencia (las emergencias necesitan la intersección vacía).
 *   2. Si NO hay emergencias (ni esperando ni acercándose) Y hay espacio
 *      (vehicles_inside < MAX_CAPACITY) Y hay regulares esperando →
 *      despertar UN regular.
 *
 * El contador "signaled" evita enviar más señales de las necesarias.
 */
static void signal_waiters(Intersection* inter) {
    /* Prioridad 1: Emergencias esperando + intersección vacía */
    if (inter->emergency_waiting > inter->emergency_signaled
        && inter->vehicles_inside == 0) {
        inter->emergency_signaled++;
        sem_post(&inter->emergency_queue);
    }
    /* Prioridad 2: Regulares esperando + hay espacio + no hay emergencias */
    else if (inter->emergency_approaching == 0
             && inter->regular_waiting > inter->regular_signaled
             && inter->vehicles_inside < MAX_CAPACITY) {
        inter->regular_signaled++;
        sem_post(&inter->regular_queue);
    }
}

/*
 * enter_intersection()
 * --------------------
 * Un vehículo (regular o emergencia) solicita entrar a una intersección.
 *
 * Flujo para EMERGENCIA:
 *   1. Marca emergency_approaching++ para que los regulares dejen de entrar.
 *   2. Si hay vehículos adentro, se bloquea en emergency_queue hasta que
 *      la intersección se vacíe completamente.
 *   3. Entra (vehicles_inside++).
 *
 * Flujo para REGULAR:
 *   1. Verifica si puede entrar: NO hay emergencias Y hay capacidad.
 *   2. Si no puede, se bloquea en regular_queue.
 *   3. Cuando es despertado, re-verifica la condición (por eso el while).
 *   4. Entra (vehicles_inside++).
 *   5. Si todavía hay espacio, despierta a otro regular en cascada.
 */
void enter_intersection(int intersection_id, int is_emergency, int vehicle_id) {
    Intersection* inter = &intersections[intersection_id];

    /* ── Adquirir mutex (sección crítica) ── */
    sem_wait(&inter->mutex);

    if (is_emergency) {
        /*
         * FLUJO DE EMERGENCIA
         * -------------------
         * Al incrementar emergency_approaching, NINGÚN regular nuevo podrá
         * entrar. Los que ya están dentro terminarán y al salir señalizarán
         * a la emergencia.
         */
        inter->emergency_approaching++;
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [EMERGENCIA] "
               "Solicitando acceso a Interseccion %d. Vehiculos dentro: %d\n",
               vehicle_id, intersection_id, inter->vehicles_inside);

        /* Esperar a que la intersección se vacíe completamente */
        while (inter->vehicles_inside > 0) {
            inter->emergency_waiting++;
            sem_post(&inter->mutex);           /* Soltar mutex antes de dormir */
            sem_wait(&inter->emergency_queue); /* BLOQUEARSE hasta ser señalizado */
            sem_wait(&inter->mutex);           /* Re-adquirir mutex */
            inter->emergency_waiting--;
            if (inter->emergency_signaled > 0)
                inter->emergency_signaled--;   /* Consumir la señal */
        }

        /* La intersección está vacía → la emergencia entra */
        inter->vehicles_inside++;
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [EMERGENCIA] "
               "Entrando a Interseccion %d. (Cruce bloqueado para regulares)\n",
               vehicle_id, intersection_id);
    }
    else {
        /*
         * FLUJO DE VEHÍCULO REGULAR
         * -------------------------
         * Condición para entrar:
         *   - No hay emergencias acercándose (emergency_approaching == 0)
         *   - Hay capacidad (vehicles_inside < MAX_CAPACITY)
         */

        /* Esperar si hay emergencia o la intersección está llena */
        while (inter->emergency_approaching > 0
               || inter->vehicles_inside >= MAX_CAPACITY) {

            /* Registrar métricas según la razón del bloqueo */
            if (inter->emergency_approaching > 0) {
                inter->regular_queued_by_emergency++;
                printf("[TIMESTAMP] [WARN] [VEHICULO %d] [REGULAR] "
                       "Encolado en Interseccion %d por paso de emergencia.\n",
                       vehicle_id, intersection_id);
            } else {
                printf("[TIMESTAMP] [INFO] [VEHICULO %d] [REGULAR] "
                       "Esperando en Interseccion %d (capacidad llena %d/%d).\n",
                       vehicle_id, intersection_id,
                       inter->vehicles_inside, MAX_CAPACITY);
            }

            /* Detección de potencial deadlock (4 regulares bloqueados) */
            if (inter->regular_waiting >= 3 && inter->vehicles_inside == MAX_CAPACITY) {
                inter->deadlocks_avoided++;
                printf("[TIMESTAMP] [WARN] [SISTEMA] "
                       "Deadlock potencial evitado en Interseccion %d "
                       "(%d vehiculos esperando, %d dentro). "
                       "Cediendo paso ordenadamente.\n",
                       intersection_id,
                       inter->regular_waiting + 1,
                       inter->vehicles_inside);
            }

            inter->regular_waiting++;
            sem_post(&inter->mutex);          /* Soltar mutex antes de dormir */
            sem_wait(&inter->regular_queue);  /* BLOQUEARSE hasta ser señalizado */
            sem_wait(&inter->mutex);          /* Re-adquirir mutex */
            inter->regular_waiting--;
            if (inter->regular_signaled > 0)
                inter->regular_signaled--;    /* Consumir la señal */
        }

        /* Condición satisfecha → el regular entra */
        inter->vehicles_inside++;
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [REGULAR] "
               "Entrando a Interseccion %d. Vehiculos dentro: %d/%d\n",
               vehicle_id, intersection_id,
               inter->vehicles_inside, MAX_CAPACITY);

        /*
         * DESPERTAR EN CASCADA:
         * Si acabo de entrar y todavía hay espacio, despierto a otro
         * regular que esté esperando. Esto evita que solo se despierte
         * uno cuando hay 2 plazas libres.
         */
        signal_waiters(inter);
    }

    /* ── Soltar mutex ── */
    sem_post(&inter->mutex);
}

/*
 * leave_intersection()
 * --------------------
 * Un vehículo sale de la intersección. Decrementa vehicles_inside y
 * evalúa a quién despertar (emergencias tienen prioridad).
 */
void leave_intersection(int intersection_id, int is_emergency, int vehicle_id) {
    Intersection* inter = &intersections[intersection_id];

    /* ── Adquirir mutex ── */
    sem_wait(&inter->mutex);

    inter->vehicles_inside--;

    if (is_emergency) {
        inter->emergency_approaching--;
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [EMERGENCIA] "
               "Saliendo de Interseccion %d. Libera bloqueo.\n",
               vehicle_id, intersection_id);
    } else {
        printf("[TIMESTAMP] [INFO] [VEHICULO %d] [REGULAR] "
               "Saliendo de Interseccion %d. Vehiculos dentro: %d/%d\n",
               vehicle_id, intersection_id,
               inter->vehicles_inside, MAX_CAPACITY);
    }

    /*
     * Señalizar a los hilos bloqueados según la política de prioridad.
     * signal_waiters() se encarga de la lógica:
     *   - Primero emergencias (si la intersección quedó vacía)
     *   - Luego regulares (si hay espacio y no hay emergencias)
     */
    signal_waiters(inter);

    /* ── Soltar mutex ── */
    sem_post(&inter->mutex);
}
