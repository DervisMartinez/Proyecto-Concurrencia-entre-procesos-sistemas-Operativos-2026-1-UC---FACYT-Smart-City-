#!/bin/bash

# Script de prueba para Proyecto SO - Problema 2
echo "Iniciando compilacion..."
make clean
make

if [ $? -ne 0 ]; then
    echo "Error de compilacion"
    exit 1
fi

echo "============================================="
echo "EJECUTANDO PRUEBA MASIVA CON 100 VEHICULOS"
echo "============================================="

# Run the simulation and capture output
./smart_city --vehicles 100 > sim_output.log

if [ $? -eq 0 ]; then
    echo "Simulacion completada exitosamente sin Segmentation Fault."
else
    echo "ERROR: La simulacion fallo (codigo de salida $?)."
    exit 1
fi

# Validaciones sobre el log
echo "============================================="
echo "VALIDACIONES DE REGLAS DE TRANSITO EN LOG"
echo "============================================="

# Extraer metricas del reporte final
echo "Métricas obtenidas:"
grep -A 5 "REPORTE FINAL" sim_output.log

echo "============================================="
echo "Verificacion de vehiculos simultaneos (max 2):"
# Buscar si algun print reporta "Vehiculos dentro: 3" o mas
VIOLATIONS=$(grep -c "Vehiculos dentro: [3-9]" sim_output.log)
if [ "$VIOLATIONS" -eq 0 ]; then
    echo "PASO: Ninguna interseccion excedio la capacidad maxima (2)."
else
    echo "FALLO: $VIOLATIONS veces se excedio la capacidad maxima."
fi

echo "Script de prueba finalizado."
