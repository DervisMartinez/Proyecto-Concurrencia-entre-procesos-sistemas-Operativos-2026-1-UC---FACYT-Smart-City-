# Proyecto: Sincronización entre Procesos
## Problema 2: Sistema de Enrutamiento y Prioridad de Emergencias en una Ciudad Inteligente (Smart City)

### 1. Interpretación del Problema y Análisis Inicial
El presente proyecto plantea el desafío de diseñar un algoritmo concurrente para el control automatizado de tráfico en una cuadrícula de cinco (5) intersecciones críticas. Se interpretó que el sistema tradicional basado en semáforos de luz temporal debía ser reemplazado por un modelo de "red vehicular" donde los vehículos reservan su derecho de paso de manera concurrente. 

Para lograr esto, se concluyó que la arquitectura óptima debía basarse en el uso de **Hilos POSIX (pthreads)** para representar a cada vehículo (garantizando un alto nivel de concurrencia compartiendo el mismo espacio de memoria del proceso padre) y **Semáforos POSIX** como herramienta estricta de sincronización para evitar colisiones lógicas, condiciones de carrera y garantizar la integridad de los datos en las intersecciones.

### 2. Asunciones y Criterios de Diseño
Durante la fase de diseño, se tomaron decisiones fundamentales (asunciones) para acotar la simulación y hacerla consistente con la realidad y los requerimientos métricos:
*   **Igualdad de Rutas:** Se asume que tanto los vehículos regulares como los de emergencia deben recorrer la misma cantidad de intersecciones (3 cruces) para que el reporte estadístico de "Tiempo promedio de viaje" sea matemáticamente justo y comparable.
*   **Diferenciación de Velocidad:** Se estableció que los vehículos de emergencia transitan a mayor velocidad (simulación de uso de sirenas y vías despejadas) entre las intersecciones en comparación con el tráfico regular.
*   **Capacidad de las Intersecciones:** Cada intersección se limitó a procesar estrictamente un máximo de dos (2) vehículos no colisionantes de manera simultánea.
*   **Ausencia de Repetición Consecutiva:** Se programó el orquestador para evitar que un vehículo "cruce" la misma intersección dos veces consecutivas, ya que carece de sentido lógico en el tránsito urbano.

### 3. Sincronización: Recursos Críticos y Secciones Críticas
En el paradigma de programación concurrente aplicado, se definieron los siguientes elementos:

*   **Recurso Crítico:** Las cinco (5) intersecciones. Al tener una capacidad máxima limitada, múltiples hilos compiten por adquirir un espacio (slot) dentro de ellas simultáneamente.
*   **Sección Crítica:** El bloque de código donde un vehículo entra o sale de la intersección. Esto involucra la modificación concurrente de variables de estado (contadores de vehículos adentro, conteo de emergencias aproximándose, y métricas de deadlocks).
*   **Mecanismo de Sincronización (Patrón Monitor):** No se utilizaron simples bloqueos aislados, sino que se modeló cada intersección como un **Monitor**. Cada intersección cuenta con:
    1.  Un semáforo binario (`mutex`) inicializado en 1, que garantiza exclusión mutua estricta sobre la Sección Crítica.
    2.  Dos semáforos contadores de condición (`regular_queue` y `emergency_queue`), inicializados en 0, utilizados para bloquear y encolar hilos cuando la capacidad del recurso crítico ha sido alcanzada.

### 4. Escenarios de Concurrencia y Errores Ajustados
Durante el desarrollo y las pruebas de estrés, el equipo se enfrentó a escenarios complejos que requirieron la refactorización del código:

*   **Inversión de Prioridad (Priority Inversion):** Se presentó el escenario donde una ambulancia quedaba encolada esperando a que vehículos regulares lentos cruzaran la intersección. **Ajuste:** Se implementó el protocolo de *Herencia de Prioridades (Priority Inheritance)*. Cuando un vehículo regular se encuentra dentro de la sección crítica y detecta que se aproxima una emergencia, su hilo acelera drásticamente su tiempo de ejecución (usleep reducido) para liberar el recurso crítico de inmediato.
*   **Despertares Perdidos (Lost Wakeup) y Deadlocks Permanentes:** En las primeras versiones, la simulación se bloqueaba (cuelgue total del sistema en Linux). Se diagnosticó que múltiples hilos entraban a la condición de espera, perdiendo la señal de despertar (`sem_post`) debido a una mala gestión de contadores dentro de los ciclos `while`. **Ajuste:** Se implementó un control riguroso de contadores explícitos (`regular_waiting`, `regular_signaled`), asegurando que las señales no se envíen al vacío y que ningún hilo quede dormido infinitamente.
*   **Falta de Seguridad en Hilos del Generador Aleatorio:** La función estándar `rand()` colapsaba bajo alta concurrencia por no ser *thread-safe*, corrompiendo las rutas. **Ajuste:** Se desarrolló una función `thread_rand()` (Generador Congruencial Lineal) que utiliza una semilla independiente (`pthread_self()`) por hilo, garantizando portabilidad y seguridad concurrente.

### 5. Explicación de los Archivos del Proyecto
El sistema fue estructurado modularmente de la siguiente manera:

*   **`main.c`:** Punto de entrada. Procesa los argumentos de línea de comandos para modular la carga de la simulación (permitiendo ajustar el número de vehículos dinámicamente) y llama al orquestador.
*   **`orchestrator.h / orchestrator.c`:** Motor principal del sistema. Responsable de la creación dinámica de los hilos de vehículos (`pthread_create`), asignación de rutas aleatorias, coordinación del cierre del sistema (`pthread_join`) y recopilación de las métricas exigidas para el reporte final de consola.
*   **`intersection.h / intersection.c`:** Módulo de sincronización. Contiene la inicialización de los arreglos de semáforos y encapsula la lógica de solicitud de entrada (`enter_intersection`) y liberación del recurso (`leave_intersection`). Contiene la inteligencia de prioridad (si una emergencia pide paso, bloquea la entrada de regulares).
*   **`vehicles.h / vehicles.c`:** Define la rutina de ciclo de vida del hilo. Contiene los retardos simulados (`usleep`) para el tránsito y el cruce, e implementa la detección para activar la Herencia de Prioridad.
*   **`Makefile` / `Dockerfile`:** Archivos que garantizan la compilación y portabilidad del proyecto sin dependencia del entorno del usuario anfitrión.

### 6. Utilidad y Aprendizaje Obtenido
El desarrollo de este proyecto demostró de manera empírica la alta volatilidad de la programación concurrente. Se comprendió que el diseño de sistemas operativos y orquestadores en la vida real no solo requiere prevenir condiciones de carrera (Race Conditions), sino anticiparse a fenónemos como la inanición (Starvation) y los bloqueos mutuos (Deadlocks). La implementación práctica del patrón de diseño Monitor mediante semáforos POSIX afianzó el conocimiento teórico, validando que el control meticuloso del estado compartido es el pilar fundamental para la creación de software concurrente seguro, robusto y tolerante a altas demandas de recursos.
