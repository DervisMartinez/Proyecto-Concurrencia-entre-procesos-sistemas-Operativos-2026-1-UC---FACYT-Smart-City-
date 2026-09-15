#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <pthread.h>
#include <semaphore.h>

typedef struct{
    int number;
    int capacity; // = INTER_SIZE
    int avoidedDealocks; // = 0

    int carsCrossed; // = 0
    int crossing; // = 0
    int emergencyCrossing; // = 0
    int carsWaiting; // = 0
    int carsArriving;

    sem_t semaph, prioSemaph; // Semaforo regular, semaforo de emergencia
    pthread_mutex_t mutex, miscmutex; // Mutex para bloquear paso a vehiculos regulares

} Intersection;

void* SendCar(void* arg);
void* SendEmergency(void* arg);

#endif