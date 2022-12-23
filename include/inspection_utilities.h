#include <ncurses.h>
#include <string.h>
#include <unistd.h> 
#include <math.h>
#include <time.h>
#include <stdlib.h>

typedef struct {
	chtype 	ls, rs, ts, bs, 
	 	tl, tr, bl, br;
}HOIST_BORDER;

typedef struct {
	int startx, starty;
	int height, width;
	HOIST_BORDER border;
}HOIST;

typedef struct {
    int x, y;
    int is_set;
}CONTAINER;

int HOIST_X_LIM = 40;
int HOIST_Y_LIM = 10;
int BTN_SIZE = 7;

// Hoist structure variable
HOIST hoist;
// Ptrs to button windows
WINDOW *stp_button, *rst_button;
// Mouse event var
MEVENT event;
// Container variable to draw random containers within the hoist's workspace
CONTAINER container;

// Initialize hoist structure and parameters
void make_hoist() {

	hoist.height = HOIST_Y_LIM;
	hoist.width = HOIST_X_LIM;
	hoist.starty = (LINES - hoist.height)/2 + 4;	
	hoist.startx = (COLS - hoist.width)/2;

	hoist.border.ls = ACS_VLINE;
	hoist.border.rs = ACS_VLINE;
	hoist.border.ts = ACS_HLINE;
	hoist.border.bs = '-';
	hoist.border.tl = ACS_ULCORNER;
	hoist.border.tr = ACS_URCORNER;
	hoist.border.bl = ACS_VLINE;
	hoist.border.br = ACS_VLINE;
}

// Create button windows
void make_buttons() {

    int stp_button_startx = (COLS - 2 * BTN_SIZE - 5) / 2;
    int rst_button_startx = (COLS - BTN_SIZE + 11) / 2;
    int buttons_starty = 4;

    stp_button = newwin(BTN_SIZE / 2, BTN_SIZE, buttons_starty, stp_button_startx);
    rst_button = newwin(BTN_SIZE / 2, BTN_SIZE, buttons_starty, rst_button_startx);
}

// Utility methods to spawn random container within the hoist's workspace
void spawn_random_container() {

    srand(time(NULL));
    container.x = rand() % HOIST_X_LIM;
    container.y = HOIST_Y_LIM - 1;
    container.is_set = TRUE;
}

void draw_hoist() {	
	
    int x, y, w, h;

	x = hoist.startx;
	y = hoist.starty;
	w = hoist.width;
	h = hoist.height;

    // Draw top corners and horizontal structure
    mvaddch(y - 1, x - 1, hoist.border.tl);
    mvaddch(y - 1, x + w, hoist.border.tr);
    mvhline(y - 1, x, hoist.border.ts, w);

    // Draw bottom corners and side structures
    mvaddch(y + h - 1, x - 1, hoist.border.bl);
    mvaddch(y + h - 1, x + w, hoist.border.br);
    mvvline(y, x - 1, hoist.border.ls, h);
    mvvline(y, x + w, hoist.border.rs, h);

    // Draw reference floor
    mvhline(y + h, x - 5, hoist.border.bs, w + 11);

    refresh();
}

// Print message with end-effector real coordinates on top of hoist drawing
void draw_end_effector_msg(float x, float y) {

    for(int j = 0; j < COLS; j++) {
        mvaddch(hoist.starty - 2, j, ' ');
    }

    char msg[100];
    sprintf(msg, "Hoist end-effector coordinates: (%05.2f, %.2f)", x, y);

    attron(A_BOLD);
    mvprintw(hoist.starty - 2, (COLS - strlen(msg)) / 2 + 1, msg);
    attroff(A_BOLD);
}

// Method to draw container (if set) withing hoist's workspace
void draw_container() {

    attron(A_BOLD | COLOR_PAIR(2));
    mvaddch(hoist.starty + container.y, hoist.startx + container.x, '#');
    attroff(A_BOLD | COLOR_PAIR(2));
}

// Draw hoist's end-effector within the structure
void draw_hoist_end_effector_at(float ee_x, float ee_y) {

    // First, empty all drawn content
     for(int j = 0; j < hoist.width; j++) {
        for(int i = 0; i < hoist.height; i++) {
            mvaddch(hoist.starty + i, hoist.startx + j, ' ');
        }
    }

    // Convert  real coordinates to lower integer...
    int ee_x_int = floor(ee_x);
    int ee_y_int = floor(ee_y);

    // If ee_y_int > 0, we need to draw the ee cable...
    for(int i = 0; i < ee_y_int; i++) {
        mvaddch(hoist.starty + i, hoist.startx + ee_x_int, '\'');
    }

    attron(A_BOLD | COLOR_PAIR(1));
    mvaddch(hoist.starty + ee_y_int, hoist.startx + ee_x_int, ACS_DARROW);
    attroff(A_BOLD | COLOR_PAIR(1));

    if(container.is_set) {
        draw_container();
    }
}

// Utility method to check for end-effector within limits
void check_ee_within_limits(float* ee_x, float* ee_y) {

    // Checks for horizontal axis
    if(*ee_x <= 0) {
        *ee_x = 0;
    }
    else if(*ee_x >= HOIST_X_LIM) {
        *ee_x = HOIST_X_LIM - 1;
    }
   
    // Checks for vertical axis
    if(*ee_y <= 0) {
        *ee_y = 0;
    }
    else if(*ee_y >= HOIST_Y_LIM) {
        *ee_y = HOIST_Y_LIM - 1;
    }
}

// Draw button with colored background
void draw_btn(WINDOW *btn, char label, int color) {

    wbkgd(btn, COLOR_PAIR(color));
    wmove(btn, BTN_SIZE / 4, BTN_SIZE / 2);

    attron(A_BOLD);
    waddch(btn, label);
    attroff(A_BOLD);

    wrefresh(btn);
}

// Draw all buttons, prepending label message
void draw_buttons() {

    char* msg = "Inspection Console Buttons";
    move(2, (COLS - strlen(msg)) / 2);
    attron(A_BOLD);
    printw(msg);
    attroff(A_BOLD);

    draw_btn(stp_button, 'S', 2);
    draw_btn(rst_button, 'R', 3);
}

// Utility method to check if button has been pressed
int check_button_pressed(WINDOW *btn, MEVENT *event) {

    if(event->y >= btn->_begy && event->y < btn->_begy + BTN_SIZE / 2) {
        if(event->x >= btn->_begx && event->x < btn->_begx + BTN_SIZE) {
            return TRUE;
        }
    }
    return FALSE;

}

// Method to check whether ee grasped the container
int check_ee_grasped_container(float ee_x, float ee_y) {

    // Convert  real coordinates to lower integer...
    int ee_x_int = floor(ee_x);
    int ee_y_int = floor(ee_y);

    return ee_x_int == container.x && ee_y_int == container.y;
}

void init_console_ui() {

    // Initialize curses mode
    initscr();		
	start_color();

    // Disable line buffering...
	cbreak();

    // Disable char echoing and blinking cursos
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(3, COLOR_BLACK,   COLOR_YELLOW);

    // Initialize UI elements
    make_hoist();
    make_buttons();

    // draw UI elements
    draw_hoist();
    draw_end_effector_msg(0, 0);
    draw_buttons();

    // Set initially container as not spawned..
    container.is_set = FALSE;

    // Activate input listening (keybord + mouse events ...)
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);

    refresh();
}

void update_console_ui(float *ee_x, float *ee_y) {

    // check if next end-effector position is within limits
    check_ee_within_limits(ee_x, ee_y);

    // Draw updated end-effector position
    draw_hoist_end_effector_at(*ee_x,*ee_y);

    // Update string message for end-effector position
    draw_end_effector_msg(*ee_x, *ee_y);

    // Check whether end-effector reached container
    if(container.is_set) {
        container.is_set = !check_ee_grasped_container(*ee_x, *ee_y);
    }
    // If reached, spawn a new one
    else {
        spawn_random_container();
    }
    
    refresh();

}

void reset_console_ui() {

    // Free resources
    delwin(stp_button);
    delwin(rst_button);

    // Clear screen
    erase();

    // Re-create UI elements
    make_hoist();
    make_buttons();

    // draw UI elements
    draw_hoist();
    draw_end_effector_msg(0, 0);
    draw_buttons();

    refresh();
}

