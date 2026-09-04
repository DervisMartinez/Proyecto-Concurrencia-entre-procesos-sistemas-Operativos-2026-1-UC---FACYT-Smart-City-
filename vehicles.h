#ifndef VEHICLES_H
#define VEHICLES_H

#include <pthread.h>
#include <sys/time.h>

// Struct for passing arguments to vehicle threads
typedef struct {
    int id;
    int is_emergency;
    int num_intersections_to_cross;
    int* route; // Array of intersection IDs
    
    // Output metrics
    long total_travel_time_ms;
} VehicleArgs;

void* vehicle_routine(void* arg);

#endif // VEHICLES_H
