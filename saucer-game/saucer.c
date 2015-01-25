/**
 * A simple multithreaded game where the objective is to prevent
 * the saucers from escaping to the right of the screen.
 *
 * General strategy is to have a single thread (the main thread)
 * responsible for drawing the UI. This is synchronized with a lock.
 * The array of saucers and rockets are each accessed through mutex
 * locks to detect for collissions.
 *
 * @author Michael Raypold
 */

#include "saucer.h"
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>


/**
 * Launches a saucer at every time interval.
 * The time period between rocket launches is dependent on a players score
 * Every DELIVERPACKAGE seconds a new care package is given to the player
 */
 void *time_handler() {
    int deliver = 0;

    while(1){
        usleep(2000000 - score * 500);
        score++;
        drawsaucer = TRUE;

        deliver++;

        if(deliver == DELIVERPACKAGE){
            carepackage = TRUE;
            deliver = 0;
        }
    }
}

/**
 * Initiates the ncurses window and sets the height and width
 */
void initiate_curses() {
    initscr();
    curs_set(FALSE);

    /* Set keyboard input options */
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    noecho();

    getmaxyx(stdscr, height, width);
}

/**
 * Set the initial values for the state of the game
 */
void set_initial() {
    score = 0;
    lives = LIVES;
    rocketsremain = ROCKETS;
    escaped = 0;

    drawsaucer = TRUE;
    carepackage = FALSE;

    /* Player position */
    px = 0;
    py = 0;
}

/**
 * Initialize the saucer game.
 */
int main(int arc, char *argv[]){
    int c;

    height = 0, width = 0;
    fired = 0; launched = 0;

    pthread_mutex_init(&mx, NULL);
    set_initial();

    /* Initiate ncurses and user the height/width to set player position */
    initiate_curses();
    px = width/2;
    py = height-2;

    show_splash();

    memset_lists();

    srand(time(NULL));
    pthread_t timethread;
    pthread_create(&timethread, NULL, time_handler, NULL);

    refresh_pad();

   /*
    * Main game loop. Waits for termination event.
    */
    while(1) {
        usleep(1000);

        if((c = getch()) == ERR){}
        else{

            if(game_over() == TRUE)
                break;

            /* User input */
            if(c == KEY_LEFT) {
                px+= LEFT;
                reverse_launcher(RIGHT);
            }
            else if(c == KEY_RIGHT) {
                px+= RIGHT;
                reverse_launcher(LEFT);
            }
            else if(c == ' ') {
                launch_rocket(fired++, px+1, py);
                rocketsremain--;
            }
            else if(c == 'q'){
                break;
            }
        }

        /* Get the starting position of the saucer and start the thread */
        if(drawsaucer == TRUE){
            int sy = (rand()%(height/2 - 1)) + 1;
            launch_saucer(launched++, sy);
            drawsaucer = FALSE;
        }
        
        if(carepackage == TRUE){
            deliver_package();
            carepackage = FALSE;
        }

        check_package();
        print_footer();

    };

    pthread_cancel(timethread);

    exit_game();

    return 0;
}

/**
 * A helper function to reverse the direction of the launcher at the screen edges
 * @param int the direction to reverse the launcher
 */
void reverse_launcher(int direction){
    if(within_screen(px+2, py) == FALSE)
        px+=direction;
    else refresh_pad();
}

/**
 * Choose a position to place the care package and have the player pick it up.
 * Since DELIVERPACKAGE = 1 and EXTRALIVES = 0, rand() is mod 2 to get the range
 */
void deliver_package(){
    int t = rand()%2;
    int posx = rand()%width;

    draw_package(t, posx);
}

/**
 * Checks if the launch pad has picked up a care package
 */
void check_package(){
    check_char(mvinch(py, px + LEFT));
    check_char(mvinch(py, px + 3*RIGHT));
}

/**
 * Checks if the char that was run over by the launcher is a package
 * @param the char to check if it is a lives or health package.
 */
void check_char(char ch){
    if(ch == '@'){
        score++;
        lives+=MORELIVES;
    }

    if(ch == '*'){
        score++;
        rocketsremain+=MOREROCKETS;
    }
}

/**
 * Checks if the exit conditions for the game are satisfied
 */
int game_over(){
    if(rocketsremain <= 0)
        return TRUE;

    if(lives <= 0)
        return TRUE;

    return FALSE;
}

/**
 * Exit the game and display the exit splash screen
 */
void exit_game(){
    pthread_mutex_lock(&mx);

    close_threads();
    erase();

    print_game_over();

    int c = 0;
    while(1){
        if((c = getch()) == ERR){}
        else{
            if(c == 'q')
                break;
        }
        
    }

    endwin();

    pthread_mutex_unlock(&mx);

    exit(0);
}

/**
 * Show the slpash screen at the startup of the game.
 */
void show_splash() {
    pthread_mutex_lock(&mx);

    int c = 0;
    int exitgame = FALSE;

    erase();

    print_splash();

    while(1){
        if((c = getch()) == ERR){}
        else{
            if(c == 's')
                break;
            if(c == 'q'){
                exitgame = TRUE;
                break;
            }
        }
    }
    
    erase();
    
    pthread_mutex_unlock(&mx);

    if(exitgame == TRUE)
        exit_game();
}

/**
 * Determines if a point is outside the boundaries of the screen
 */
 int within_screen(int x, int y) {
    int row, col;
    getmaxyx(stdscr, row, col);

    if(x < 0 || x > col)
        return FALSE;

    if(y < 0 || y > row)
        return FALSE;

    return TRUE;
}

/**
 * Inifinite loop that updates the rocket until destroyed.
 * @param arg a rocket struct to animate and display in ncurses
 */
 void *update_rocket(void *arg) {
    struct object *rocket = arg;

    while(1) {
        usleep(100000 * rocket->velocity);

        pthread_mutex_lock(&mx);

        clear_rocket(rocket);

        if(rocket->collision == FALSE)
            print_rocket(rocket);

        /* Check if rocket off screen and destroy it and exit thread */
        if(rocket->y <= -1){
            rocket->collision = TRUE;
        }

        rocket->y+=UP;

        pthread_mutex_unlock(&mx);

        if(rocket->collision == TRUE)
            pthread_exit(NULL);
    }

    pthread_exit(NULL);
}

/**
 * Infinite loop that updates the saucer until destroyed.
 * Collision detection is handled in the saucer loops; not the rocket loops.
 * @param arg a saucer struct to animate and display in ncurses
 */
 void *update_saucer(void *arg) {
    struct object *saucer = arg;

    while(1) {
        usleep(100000 * saucer->velocity);

        pthread_mutex_lock(&mx);

        clear_saucer(saucer);

        /* Check for collisions with the rockets */
        int i = 0, j = 0;

        for(i = 0; i < MAXTHREADS; i++){
            if(saucer->y == rockets[i].y){

                for(j = 0; j < 5; j++){
                    if(saucer->x - j == rockets[i].x){
                        saucer->collision = TRUE;
                        rockets[i].collision = TRUE;
                        rocketsremain++;
                        score++;
                    }
                }
            }
        }

        if(saucer->collision == FALSE)
            print_saucer(saucer);

        if(saucer_escaped(saucer) == TRUE)
            saucer->collision = TRUE;

        saucer->x+=RIGHT;

        pthread_mutex_unlock(&mx);

        if(saucer->collision == TRUE)
            pthread_exit(NULL);
    }

    pthread_exit(NULL);
}

/**
 * Check if a saucer has escaped
 * @param ptr to a saucer structure
 */
int saucer_escaped(struct object *ptr){
    if(ptr->x - 5 > width){
        lives--;
        escaped++;
        return TRUE;
    }
    else
        return FALSE;
}

