#include "intersection.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void* SendCar(void* arg)
{
    Intersection* intersect = (Intersection *)arg;

    pthread_mutex_lock(&intersect->miscmutex);
    intersect->carsArriving++;
    pthread_mutex_unlock(&intersect->miscmutex);
    sleep(1);

    int wasWaiting = 0;
    int wasBlockedByEmergency = 0;

    pthread_mutex_lock(&intersect->miscmutex);
    if(intersect->carsArriving == 4)
    {
        printf("| INTERSECTION %d | DEADLOCK AVOIDED |\n", intersect->number);
        intersect->avoidedDealocks++;
    }
    intersect->carsArriving--;
    pthread_mutex_unlock(&intersect->miscmutex);

    // Semaforo de vehiculos regulares
    if(sem_trywait(&intersect->semaph) != 0) // Si nos va a bloquear, aumentar el contador de 'en espera'
    {
        pthread_mutex_lock(&intersect->miscmutex);
        intersect->carsWaiting++;
        wasWaiting++;
        pthread_mutex_unlock(&intersect->miscmutex);
        sem_wait(&intersect->semaph);
    }

    if(wasWaiting == 1) // Decrementar el contador de 'en espera' si estaba esperando
    {
        pthread_mutex_lock(&intersect->miscmutex);
        intersect->carsWaiting--;
        pthread_mutex_unlock(&intersect->miscmutex);
    }

    /* Este mutex impide el acceso de vehiculos regulares a la interseccion hasta que hayan pasado
    todos los vehiculos de emergencia*/
    if(intersect->emergencyCrossing > 0)
    {
        pthread_mutex_lock(&intersect->miscmutex);
        intersect->carsWaiting++;
        wasBlockedByEmergency++;
        pthread_mutex_unlock(&intersect->miscmutex);

        pthread_mutex_lock(&intersect->mutex);
    }

    // Semaforo de vehiculos de emergencia
    sem_wait(&intersect->prioSemaph);

    // Liberar el mutex si no hay vehiculos de emergencia cruzando
    if(wasBlockedByEmergency > 0)
    {
        pthread_mutex_lock(&intersect->miscmutex);
        intersect->carsWaiting--;
        pthread_mutex_unlock(&intersect->miscmutex);
        pthread_mutex_unlock(&intersect->mutex);
    }

    // Cruzar
    intersect->crossing++;
    sleep(1);
    intersect->crossing--;

    // Liberar ambos semaforos
    sem_post(&intersect->prioSemaph);
    sem_post(&intersect->semaph);

    intersect->carsCrossed--;

}

void* SendEmergency(void *arg)
{
    Intersection* intersect = (Intersection *)arg;

    pthread_mutex_lock(&intersect->miscmutex);
    intersect->carsArriving++;
    pthread_mutex_unlock(&intersect->miscmutex);
    sleep(1);

    pthread_mutex_lock(&intersect->miscmutex);
    if(intersect->carsArriving == 4)
    {
        printf("| INTERSECTION %d | DEADLOCK AVOIDED |\n", intersect->number);
        intersect->avoidedDealocks++;
    }
    intersect->carsArriving--;
    pthread_mutex_unlock(&intersect->miscmutex);

    // Bloquear acceso a vehiculos regulares
    if(intersect->emergencyCrossing == 0)
    {
        pthread_mutex_lock(&intersect->mutex);
        pthread_mutex_lock(&intersect->miscmutex);
        printf("| INTERSECTION %d | BLOCKING REGULAR ACCESS | REGULAR VEHICLES WAITING: %d |\n", intersect->number, intersect->carsWaiting);
        pthread_mutex_unlock(&intersect->miscmutex);
    }

    intersect->emergencyCrossing++;

    if(sem_trywait(&intersect->prioSemaph) != 0)
    {
        printf("| INTERSECTION %d | EMERGENCY VEHICLE(S) AWAITING CLEARANCE |\n", intersect->number);
        sem_wait(&intersect->prioSemaph);
    }

    while(intersect->crossing > 0);

    pthread_mutex_lock(&intersect->miscmutex);
    pthread_mutex_unlock(&intersect->miscmutex);
    sleep(1);

    sem_post(&intersect->prioSemaph);

    intersect->emergencyCrossing--;
    // Permitir acceso a vehiculos regulares si no hay vehiculos de emergencia cruzando
    if(intersect->emergencyCrossing == 0)
    {
        printf("| INTERSECTION %d | ALLOWING REGULAR ACCESS |\n", intersect->number);
        pthread_mutex_unlock(&intersect->mutex);
    }

}