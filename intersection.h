#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <semaphore.h>

#define NUM_INTERSECTIONS 5
#define MAX_CAPACITY 2

typedef struct {
    int id;
    sem_t mutex;
    sem_t regular_sem;
    sem_t emergency_sem;
    
    int vehicles_inside;
    int emergency_approaching;
    int regular_waiting_count;
    int emergency_waiting_count;
    
    int deadlocks_avoided; 
    int regular_queued_by_emergency;
} Intersection;

extern Intersection intersections[NUM_INTERSECTIONS];

void init_intersections();
void enter_intersection(int id, int is_emergency, int vehicle_id);
void leave_intersection(int id, int is_emergency, int vehicle_id);
void destroy_intersections();

#endif // INTERSECTION_H
