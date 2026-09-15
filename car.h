#ifndef CAR_H
#define CAR_H
#include "intersection.h"
#include <pthread.h>
#include <semaphore.h>

typedef struct{
    int number;
    int isEmergency; // 0 = Regular, 1 = Emergency
    Intersection** path;
    int pathLength;
    double travelTime;
    
    int* carCounter; // Pointer to integer

    sem_t* simSemaph; // Semaforo para entrar a la simulacion
} Car;

void* VehicleThread(void *arg);

#endif