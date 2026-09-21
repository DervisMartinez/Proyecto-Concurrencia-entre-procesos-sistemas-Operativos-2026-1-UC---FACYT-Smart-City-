/*
 * ============================================================================
 * orchestrator.h - Definición del Orquestador Central de Tráfico
 * ============================================================================
 *
 * El Orquestador es responsable de:
 *   1. Inicializar las intersecciones (recursos compartidos)
 *   2. Crear todos los hilos de vehículos (regulares + emergencias)
 *   3. Esperar a que todos terminen (pthread_join)
 *   4. Calcular e imprimir las métricas del reporte final
 *   5. Liberar todos los recursos
 * ============================================================================
 */
#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include "vehicles.h"

/*
 * run_simulation()
 * ----------------
 * Ejecuta la simulación completa con el número de vehículos indicado.
 *
 * Parámetros:
 *   num_regular   - Cantidad de vehículos regulares a crear (ej: 20)
 *   num_emergency - Cantidad de vehículos de emergencia a crear (ej: 2)
 */
void run_simulation(int num_regular, int num_emergency);

#endif /* ORCHESTRATOR_H */
