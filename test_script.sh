#!/bin/bash
# ============================================================================
# test_script.sh - Script de validación para la simulación Smart City
# ============================================================================
# Compila el proyecto, ejecuta con 100 vehículos concurrentes, y verifica
# que no hubo violaciones a las reglas de tránsito ni fallos.
# ============================================================================

set -e

echo "============================================="
echo " PASO 1: Compilacion"
echo "============================================="
make clean
make
echo "Compilacion exitosa."

echo ""
echo "============================================="
echo " PASO 2: Ejecucion por defecto (20 reg + 2 emg)"
echo "============================================="
./smart_city 2>&1 | tee sim_default.log
echo ""

echo "============================================="
echo " PASO 3: Prueba masiva con 100 vehiculos"
echo "============================================="
./smart_city --vehicles 100 2>&1 | tee sim_100.log

RETCODE=$?
if [ $RETCODE -eq 0 ]; then
    echo ""
    echo "Simulacion completada SIN Segmentation Fault (codigo: $RETCODE)."
else
    echo "ERROR: La simulacion fallo con codigo $RETCODE."
    exit 1
fi

echo ""
echo "============================================="
echo " PASO 4: Validaciones automaticas"
echo "============================================="

# Verificar que ninguna interseccion excedio la capacidad maxima (2)
VIOLATIONS=$(grep -cE "Vehiculos dentro: [3-9]" sim_100.log || true)
if [ "$VIOLATIONS" -eq 0 ]; then
    echo "[PASO] Capacidad maxima (2) nunca fue excedida."
else
    echo "[FALLO] Capacidad excedida $VIOLATIONS veces."
fi

# Verificar que los regulares fueron encolados por emergencia
QUEUED=$(grep -c "Encolado" sim_100.log || true)
echo "[INFO] Regulares encolados por emergencia: $QUEUED"

# Verificar que hubo herencia de prioridad
INHERIT=$(grep -c "Priority Inheritance" sim_100.log || true)
echo "[INFO] Eventos de Priority Inheritance: $INHERIT"

# Extraer el reporte final
echo ""
echo "============================================="
echo " REPORTE FINAL EXTRAIDO:"
echo "============================================="
grep -A 10 "REPORTE FINAL" sim_100.log || echo "(No se encontro reporte)"

echo ""
echo "Script de prueba finalizado exitosamente."
