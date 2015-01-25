/**
 * Handles the drawing of objects to the ncurses terminal
 *
 * @author Michael Raypold
 */

#include "saucer.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/**
 * Draw a care package for the player to pick up by 'running it over'
 * @param type whether the care package is health or rockets
 * @param posx the x position to place the care package
 */
void draw_package(int type, int posx) {
    char ehealth[] = "@";
    char erockets[] = "*";

    if(type == EXTRALIVES)
        mvprintw(py, posx, ehealth);

    if(type == EXTRAROCKETS)
        mvprintw(py, posx, erockets);

}

/**
 * Prints all the game instructions to the ncurses window
 */
void print_splash() {
    char *spacer = " ";
	char *name = "Saucers";
	char *goal = "Do not let any saucers escape!";

	char *spacebar = "Spacebar : Fire Rockets";
	char *arrow = "Arrow Keys : Move Launcher";

	char *scoring = "Score is incremented every second";
	char *launching = "The rate of saucer arrival gradually increases";
    char *ehealth = "Pickup @ packages for two extra lives";
    char *erockets = "Pickup * packages for 5 extra rockets";
    char *more = "Destroying a saucer grants more rockets";

	char *start = "Press s to begin";
	char *exitmsg = "Press q to exit at any time";

	int i = 2;

	print_string(name, i++);
	print_string(spacer, i++);
	print_string(goal, i++);
	print_string(spacer, i++);
	print_string(spacebar, i++);
	print_string(arrow, i++);
	print_string(spacer, i++);
	print_string(scoring, i++);
	print_string(launching, i++);
    print_string(ehealth, i++);
    print_string(erockets, i++);
    print_string(more, i++);
	print_string(spacer, i++);
	print_string(start, i++);
	print_string(exitmsg, i);
}

/**
 * Refreshes the rocket launch pad in response to user input
 */
 void refresh_pad() {               
    pthread_mutex_lock(&mx);

    check_package();

    /* Clear the left and right side of the launcher */
    mvaddch(py, px+LEFT, ' ');
    mvaddch(py, px+3*RIGHT, ' ');

    /* Print the new lancher */
    mvprintw(py, px, "_|_");

    pthread_mutex_unlock(&mx);
}

/**
 * Display the user scores after game over.
 */
void print_game_over() {
    char *over = "Game Over!";
    char *exitcond = "Press q to quit";
    char *spacer = " ";
    char *esc = "Aliens Escaped: ";
    char *scr = "Score: ";
    
    char *es = (char *) malloc(sizeof(char *));
    char *ss = (char *) malloc(sizeof(char *));

    sprintf(es, "%d", escaped);
    sprintf(ss, "%d", score);

    char *econcat = (char *)malloc(1 + strlen(esc) + strlen(es));
    strcpy(econcat, esc);
    strcat(econcat, es);

    char *sconcat = (char *)malloc(1 + strlen(scr) + strlen(ss));
    strcpy(sconcat, scr);
    strcat(sconcat, ss);

    int i = 10;

    print_string(over, i++);
    print_string(exitcond, i++);
    print_string(spacer, i++);
    print_string(econcat, i++);
    print_string(sconcat, i);

    free(econcat);
    free(sconcat);
    free(es);
    free(ss);
}

/**
 * A helper function that prints a string to the center of the screen
 * @param str the string to print
 * @param yline the vertical position to place the string at
 */
void print_string(char *str, int yline) {
    int len = strlen(str);
    mvprintw(yline, width/2 - len/2, "%s", str);
}

/**
 * Print a footer at the bottom of the screen with scoring information
 * and game statistics.
 */
 void print_footer() {
    char alienmsg[] = "Lives: ";
    char rocketmsg[] = "Rockets Remaining: ";
    char scoremsg[] = "Score: ";

    int posx = 0;

    pthread_mutex_lock(&mx);

    clear_footer();

    posx+= print_msg(posx, alienmsg);
    posx+= print_scores(posx, lives);

    posx+= print_msg(posx, rocketmsg);
    posx+= print_scores(posx, rocketsremain);

    posx+= print_msg(posx, scoremsg);
    posx+= print_scores(posx, score);

    pthread_mutex_unlock(&mx);
}

/**
 * Clears the entire bottom row of the screen
 */
void clear_footer(){
    int i = 0;
    for(i = 0; i <= width; i++)
        mvaddch(height-1, i, ' ');
}

/**
 * Helper function for print_footer() that prints message at
 * the bottom of the screen to help reduce repeated code.
 *
 * @return the new cursor postion for the next message to print
 */
 int print_msg(int posx, char  msg[]) {
    mvprintw(LINES -1, posx, msg);

    return strlen(msg);
}

/**
 * Helper function for print_footer() that converts an int
 * to a char[] and prints it to the bottom of the screen.
 *
 * @return the new cursor position for the next message to print
 */
 int print_scores(int posx, int number) {
    int padding = 5;
    char msg[padding];

    memset(msg, '\0', padding);
    sprintf(msg, "%d", number);

    return print_msg(posx, msg) + padding;
}

/**
 * Print the rocket to the ncurses window
 * @parm The struct containing the coordinates to print
 */
 void print_rocket(struct object *ptr) {
    if(ptr->collision == FALSE)
        mvaddch(ptr->y, ptr->x, '^');
}

/**
 * Print the saucer to the ncurses window
 * @param The struct containing coordinates to print
 */
 void print_saucer(struct object *ptr) {
    if(ptr->collision == FALSE){
        mvaddch(ptr->y, ptr->x, '>');
        mvaddch(ptr->y, ptr->x-1, '-');
        mvaddch(ptr->y, ptr->x-2, '-');
        mvaddch(ptr->y, ptr->x-3, '-');
        mvaddch(ptr->y, ptr->x-4, '<');
    }
}

/**
 * Clear the rocket struct from the ncurses window
 * @param The struct containing the coordinates to clear
 */
 void clear_rocket(struct object *ptr) {
    int oldy = ptr->y + ptr->velocity;
    mvaddch(oldy, ptr->x, ' ');
    mvaddch(oldy+1, ptr->x, ' ');
}

/**
 * Clear the saucer struct form the ncurses window
 * @param The struct containing the coordinates to clear
 */
 void clear_saucer(struct object *ptr) {
    int oldx = ptr->x + ptr->velocity;
    int i = 0;
    for(i = 0; i <= 7; i++)
        mvaddch(ptr->y, oldx-i, ' ');
}
