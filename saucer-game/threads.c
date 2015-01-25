/**
 * Handles the creation and insertion of rockets and saucers into the arrays
 * and the creation of pthreads.
 *
 * @author Michael Raypold
 */

#include "saucer.h"
#include <stdlib.h>
#include <string.h>

/**
 * Clear the lists that will hold the rockets and saucers
 */
void memset_lists(){
    memset(rockets, 0, sizeof(rockets));
    memset(saucers, 0, sizeof(saucers));
}

/**
 * Cancel all open threads for the saucers and rockets
 */
void close_threads(){
    int i = 0;

    for(i = 0; i < MAXTHREADS; i++){
        pthread_cancel(rthreads[i]);
        pthread_cancel(sthreads[i]);
    }
}

/**
 * Create a new rocket at the specified coordinates
 * @param px the x coordinate the rocket will start at
 * @param py the y coordinate the rocket will start at
 * @param id the id (number of rockets % MAXTHREADS) launched
 */
struct object add_rocket(int px, int py, int id){
    struct object rocket;

    rocket.x = px;
    rocket.y = py;
    rocket.type = ROCKET;
    rocket.velocity = 1;
    rocket.direction = UP;
    rocket.id = id;
    rocket.collision = FALSE;

    return rocket;
}

/*
 * Create a rocket and initiate the pthread.
 * @param id or the array position to place the rocket and thread
 * @param px the x coordinate of the rocket
 * @param py the y coordinate of the rocket
 */
void launch_rocket(int id, int px, int py){
    pthread_mutex_lock(&mx);

    int i = id % MAXTHREADS;

    rockets[i] = add_rocket(px, py, i);
    pthread_create(&rthreads[i], NULL, update_rocket, 
        &rockets[i]);

    pthread_mutex_unlock(&mx);
}

/**
 * Create a new saucer at the specified coordinates
 * @param py the y coordinate the rocket will start at
 * @param id the id (number of saucers % MAXTHREADS) launched
 */
struct object add_saucer(int py, int id){
    struct object saucer;

    saucer.x = 0;
    saucer.y = py;
    saucer.type = SAUCER;
    saucer.velocity = 1;
    saucer.direction = RIGHT;
    saucer.id = id;
    saucer.collision = FALSE;

    return saucer;
}

/**
 * Create a new saucer and initiate the pthread
 * @param id or the array position to place the rocket and thread
 * @param py the randomly chosen launch position of the saucer
 */
void launch_saucer(int id, int py){
    pthread_mutex_lock(&mx);

    int i = id % MAXTHREADS;

    saucers[i] = add_saucer(py, i);
    pthread_create(&sthreads[i], NULL, update_saucer,
        &saucers[i]);

    pthread_mutex_unlock(&mx);

}