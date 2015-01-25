#ifndef SAUCER_H
#define SAUCER_H

#include <pthread.h>
#include <signal.h>

#define MAXTHREADS 30

#define ROCKET 1
#define SAUCER 0

#define TRUE 1
#define FALSE 0

/* Directions for saucer and player cursor */
#define LEFT -1
#define RIGHT 1
#define STILL 0
#define UP -1
#define DOWN 1

/* Game initial state */
#define ROCKETS 10
#define LIVES 10

/* How many threads to create */
#define MAXROCKETS 20
#define MAXSAUCERS 20

#define DELIVERPACKAGE 1
#define EXTRALIVES 1
#define EXTRAROCKETS 0

#define MORELIVES 2
#define MOREROCKETS 5

sig_atomic_t fired;
sig_atomic_t launched;

sig_atomic_t height;
sig_atomic_t width;

sig_atomic_t score;
sig_atomic_t lives;
sig_atomic_t rocketsremain;
sig_atomic_t escaped;

sig_atomic_t drawsaucer;
sig_atomic_t carepackage;

/* Player position */
sig_atomic_t px;
sig_atomic_t py;

pthread_t rthreads[MAXTHREADS];
pthread_t sthreads[MAXTHREADS];

pthread_mutex_t mx;

/* Holds an onscreen object */
struct object {
        int x;
        int y;
        int type;
        int velocity;
        int direction;
        int id;
        int collision;
};

struct object rockets[MAXTHREADS];
struct object saucers[MAXTHREADS];

/* saucer.c functions */
void *time_handler();
void initiate_curses();
void set_initial();
int main(int, char *argv[]);
void reverse_launcher(int);
void deliver_package();
void check_package();
void check_char(char);
int game_over();
void exit_game();
void show_splash();
int within_screen(int, int);
void *update_rocket(void *arg);
void *update_saucer(void *arg);
int saucer_escaped(struct object*);

/* draw.c functions */
void draw_package(int, int);
void print_splash();
void refresh_pad();
void print_game_over();
void print_string(char*, int);
void print_footer();
void clear_footer();
int print_msg(int, char[]);
int print_scores(int, int);
void print_rocket(struct object*);
void print_saucer(struct object*);
void clear_rocket(struct object*);
void clear_saucer(struct object*);

/* threads.c functions */
void memset_lists();
void close_threads();
struct object add_rocket(int, int, int);
struct object add_saucer(int, int);
void launch_rocket(int, int, int);
void launch_saucer(int, int);

#endif
