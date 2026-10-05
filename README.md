# 🚦 Proyecto Sincronización entre Procesos: Smart City

**Universidad de Carabobo (UC)**  
**Facultad Experimental de Ciencias y Tecnología (FACYT)**  
**Asignatura:** Sistemas Operativos – CAO503  
**Profesora:** Mirella Herrera  
**Semestre:** 1-2026  

---

## 📖 Descripción del Proyecto

Este proyecto resuelve el **Problema 2: Sistema de Enrutamiento y Prioridad de Emergencias en una Ciudad Inteligente**. Consiste en un algoritmo concurrente escrito en Lenguaje C que reemplaza los semáforos de luz tradicionales por un sistema de coordinación vehicular automatizado. 

En esta simulación, cada vehículo actúa como un **Hilo POSIX (pthread)** independiente, y cada intersección opera como un **Recurso Compartido** estrictamente protegido mediante **Semáforos POSIX** bajo el patrón de diseño Monitor.

### 🚀 Características Principales
- **Concurrencia Nativa:** Implementación basada puramente en `pthreads` para vehículos regulares y de emergencia.
- **Herencia de Prioridades (Priority Inheritance):** Prevención explícita del problema de la inversión de prioridad. Los vehículos regulares aceleran su cruce si detectan una emergencia aproximándose.
- **Prevención de Deadlocks y Lost Wakeups:** Control de capacidad máxima (2 vehículos por cruce) y banderas estrictas de contadores en los ciclos de bloqueo para evitar inanición (starvation) y esperas circulares.
- **Generador LCG Thread-Safe:** Uso de `thread_rand()` para garantizar que la generación de rutas aleatorias sea segura sin depender del estado global de las librerías estándar.

---

## 📂 Estructura del Código

- `main.c` - Punto de entrada del programa. Administra los argumentos CLI de la simulación.
- `orchestrator.c / .h` - Orquestador central. Despliega los hilos, recolecta las métricas e imprime el reporte.
- `intersection.c / .h` - Motor de sincronización (Monitores). Administra las secciones críticas y los semáforos.
- `vehicles.c / .h` - Comportamiento de los hilos (tránsito, esperas de semáforo, herencia de prioridad).
- `Makefile` - Script de automatización para la compilación (GCC).
- `Dockerfile` - Entorno de contenedor ligero (`gcc:13-bookworm`) para compilación y despliegue aislado.
- `Documentacion_Proyecto.md` - Informe detallado con el análisis técnico y de sincronización.

---

## 🛠️ Cómo compilar y ejecutar

### Opción 1: Compilación Nativa (Linux / MacOS / MSYS2-MinGW)
Asegúrese de tener `gcc` y `make` instalados.
```bash
# 1. Limpiar y compilar el proyecto
make clean
make

# 2. Ejecutar la simulación por defecto (20 regulares, 2 emergencias)
./smart_city

# 3. Ejecutar una prueba de estrés (ej. 100 vehículos)
./smart_city --vehicles 100
```

### Opción 2: Ejecución aislada con Docker (Recomendado)
Para evitar problemas de dependencias en el sistema anfitrión:
```bash
# 1. Construir la imagen
docker build -t smart_city .

# 2. Correr simulación base
docker run --rm smart_city

# 3. Correr simulación de estrés
docker run --rm smart_city ./smart_city --vehicles 100
```

---

## 📊 Reporte de Simulación
Al finalizar la ejecución, el orquestador aplica un `pthread_join` general y emite un reporte que valida:
1. El **tiempo de viaje total promedio** (demostrando que las emergencias tienen un tiempo sustancialmente menor).
2. El contador de **deadlocks evitados** (escenarios "4-way" controlados).
3. Número de **vehículos regulares encolados** cediendo el paso por el protocolo de emergencia.

---
*Desarrollado en lenguaje C para el aprendizaje práctico de la gestión de recursos críticos y programación concurrente.*
