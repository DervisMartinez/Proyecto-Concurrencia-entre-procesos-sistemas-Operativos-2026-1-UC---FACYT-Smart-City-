#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>

#include <unistd.h>

#include "orchestrator.h"

#define N_INTERSECTIONS 4
#define INTER_SIZE 2
#define N_CARS 90
#define N_EMERGENCIES 10
#define N_REGULAR_CONCURRENT 20
#define N_EMERGENCY_CONCURRENT 2
#define CAR_PATH_MINIMUM 2
#define CAR_PATH_MAX 4

// Vector de intersecciones
Intersection intersectionArray[N_INTERSECTIONS];
sem_t regularSimSemaph;
sem_t emergencySimSemaph;
int carsDone = 0;

void InitializeVehicle(Car *carPointer, int isEmergency, int number)
{
    // Generar un vehiculo con una ruta aleatoria
    carPointer->number = number;
    carPointer->isEmergency = isEmergency;
    carPointer->travelTime = 0;
    carPointer->pathLength = (rand()%(CAR_PATH_MAX-CAR_PATH_MINIMUM+1))+CAR_PATH_MINIMUM;
    carPointer->carCounter = &carsDone;

    // Generar ruta aleatoria
    // La ruta es un vector de punteros a intersecciones
    carPointer->path = malloc(sizeof(Intersection*) * carPointer->pathLength);

    if(isEmergency)
        carPointer->simSemaph = &emergencySimSemaph;
    else
        carPointer->simSemaph = &regularSimSemaph;

    for(int j = 0; j < carPointer->pathLength; j++)
    {
        int randomNumber = rand()%N_INTERSECTIONS;
        carPointer->path[j] = &intersectionArray[randomNumber];
    }
}

void InitializeIntersection(Intersection *intersectionPointer, int number)
{
    intersectionPointer->number = number;
    intersectionPointer->capacity = INTER_SIZE;
    intersectionPointer->avoidedDealocks = 0;

    intersectionPointer->carsCrossed = 0;
    intersectionPointer->crossing = 0;
    intersectionPointer->emergencyCrossing = 0;
    intersectionPointer->carsWaiting = 0;
    intersectionPointer->carsArriving = 0;

    sem_init(&intersectionPointer->semaph, 0, INTER_SIZE);
    sem_init(&intersectionPointer->prioSemaph, 0, INTER_SIZE);
    pthread_mutex_init(&intersectionPointer->mutex, NULL);
    pthread_mutex_init(&intersectionPointer->miscmutex, NULL);
}

double GetAverageTravelTime(Car* carArray, int arrayLength)
{
    double result = 0;
    for(int i = 0; i < arrayLength; i++)
    {
        result += carArray[i].travelTime;
    }
    result = result/arrayLength;
    return result;
}

void Orchestrate()
{
    srand(time(NULL));

    sem_init(&regularSimSemaph, 0, N_REGULAR_CONCURRENT);
    sem_init(&emergencySimSemaph, 0, N_EMERGENCY_CONCURRENT);

    pthread_t carThreads[N_CARS];
    Car carList[N_CARS];
    pthread_t emergencyThreads[N_EMERGENCIES];
    Car emergencyList[N_EMERGENCIES];

    // Initialize intersections
    for(int i = 0; i < N_INTERSECTIONS; i++)
    {
        InitializeIntersection(&intersectionArray[i], i);
    }

    // Generar vehiculos regulares
    for(int i = 0; i < N_CARS; i++)
    {
        InitializeVehicle(&carList[i], 0, i);
    }
    // Generar vehiculos de emergencia
    for(int i = 0; i < N_EMERGENCIES; i++)
    {
        InitializeVehicle(&emergencyList[i], 1, N_CARS+i+1);
    }


    // Enviar a todos los vehiculos a la vez
    for(int i = 0; i < N_CARS; i++)
    {
        pthread_create(&carThreads[i], NULL, VehicleThread, &carList[i]);
    }

    // Enviar vehiculos de emergencia
    for(int i = 0; i < N_EMERGENCIES; i++)
    {
        pthread_create(&emergencyThreads[i], NULL, VehicleThread, &emergencyList[i]);
    }

    // Esperar a que todos los vehiculos lleguen a su destino
    while(carsDone < N_CARS+N_EMERGENCIES);


    // Mostrar resultados finales por pantalla
    int totalDeadlocks = 0;
    for(int i = 0; i < N_INTERSECTIONS; i++)
    {
        printf("DEADLOCKS AVOIDED IN INTERSECTION %d: %d\n", intersectionArray[i].number, intersectionArray[i].avoidedDealocks);
        totalDeadlocks += intersectionArray[i].avoidedDealocks;
    }
    printf("TOTAL DEADLOCKS AVOIDED: %d\n", totalDeadlocks);

    // Tiempo de viaje promedio de vehiculos regulares
    double averageTravelTime = GetAverageTravelTime(carList, N_CARS);
    printf("AVERAGE TRAVEL TIME FOR VEHICLES %lf\n", averageTravelTime);

    // Tiempo de viaje promedio de vehiculos de emergencia
    averageTravelTime = GetAverageTravelTime(emergencyList, N_EMERGENCIES);
    printf("AVERAGE TRAVEL TIME FOR EMERGENCY VEHICLES %lf\n", averageTravelTime);

}