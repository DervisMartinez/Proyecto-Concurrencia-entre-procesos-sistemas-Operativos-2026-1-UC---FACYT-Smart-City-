# ==============================================================================
# Dockerfile - Entorno de compilación y ejecución para Smart City
# ==============================================================================
# Usa una imagen de GCC sobre Alpine Linux (muy liviana, ~200MB vs ~1GB de Ubuntu)
# que ya trae gcc, make y libc preinstalados. No necesita apt-get.
# ==============================================================================

FROM gcc:13-bookworm

# Crear el directorio de trabajo
WORKDIR /app

# Copiar todo el código fuente al contenedor
COPY *.c *.h Makefile ./

# Compilar el proyecto durante el build (así se detectan errores antes)
RUN make

# Comando por defecto: ejecutar la simulación base
CMD ["./smart_city"]
