# Utilizar una imagen base estable de Ubuntu
FROM ubuntu:22.04

# Evitar prompts interactivos durante la instalación
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependencias esenciales (gcc, make, bash, y utilidades de compilación)
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    make \
    bash \
    && rm -rf /var/lib/apt/lists/*

# Crear el directorio de trabajo dentro del contenedor
WORKDIR /app

# Copiar todos los archivos del proyecto al contenedor
COPY . /app/

# Dar permisos de ejecución al script de prueba
RUN chmod +x test_script.sh

# Comando por defecto: Limpiar, compilar y ejecutar el script de prueba
CMD ["/bin/bash", "test_script.sh"]
