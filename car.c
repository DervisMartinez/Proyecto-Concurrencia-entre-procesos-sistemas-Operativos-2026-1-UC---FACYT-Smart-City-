#include "car.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* VehicleThread(void *arg)
{
    Car* car = (Car *)arg;

    /* Puntero al semaforo de simulacion, limita el numero de vehiculos concurrentes a 20 regulares
    y 2 de emergencia */
    sem_wait(car->simSemaph);

    if(car->isEmergency)
        printf("| EMERGENCY CAR %d | ENTERING SIMULATION |\n", car->number);
    else
        printf("| CAR %d | ENTERING SIMULATION |\n", car->number);

    time_t initialTime;
    time(&initialTime);
    
    for(int i = 0; i < car->pathLength; i++)
    {
        if(car->isEmergency)
        {
            printf("| EMERGENCY CAR %d | ENTERING INTERSECTION %d |\n", car->number, car->path[i]->number);
            SendEmergency(car->path[i]);
        }
        else
        {
            printf("| CAR %d | ENTERING INTERSECTION %d |\n", car->number, car->path[i]->number);
            SendCar(car->path[i]);
        }
    }

    time_t finalTime;
    time(&finalTime);

    car->travelTime = difftime(finalTime, initialTime);
    if(car->isEmergency)
        printf("| EMERGENCY CAR %d | FINAL TRAVEL TIME %lf |\n", car->number, car->travelTime);
    else
        printf("| CAR %d | FINAL TRAVEL TIME %lf |\n", car->number, car->travelTime);

    // Liberar la memoria del camino cuando termine de recorrerlo
    free(car->path);
    car->path = NULL;
    // Aumentar el contador de vehiculos que llegaron a su destino
    *(car->carCounter) = *(car->carCounter)+1;

    // Liberar semaforo de simulacion
    sem_post(car->simSemaph);
}