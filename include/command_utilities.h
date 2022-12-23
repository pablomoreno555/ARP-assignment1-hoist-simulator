#include <ncurses.h>
#include <string.h>
#include <unistd.h> 
#include <math.h>

int BTN_SIZE_X = 7;
int BTN_SIZE_Y = 3;

// Ptrs to button windows
WINDOW *vx_incr_btn, *vx_decr_btn, *vx_stp_button, *vz_incr_btn, *vz_decr_btn, *vz_stp_button;
// Mouse event var
MEVENT event;

// Create button windows
void make_buttons() {

    // Offsets
    int offset_x = 6;
    int offset_y = 2;

    // Horizontal displacements for aligning buttons
    int decr_button_startx = (COLS - 3 * BTN_SIZE_X - offset_x) / 2;
    int stp_button_startx = (COLS - 2 * BTN_SIZE_X + offset_x) / 2;
    int incr_button_startx = (COLS - BTN_SIZE_X + 3 * offset_x + 1) / 2;

    // Vertical displacements
    int vx_buttons_starty = (LINES - 2 * BTN_SIZE_Y - offset_y) / 2;
    int vz_buttons_starty = (LINES - BTN_SIZE_Y + 3 * offset_y) / 2;

    // Horizontal velocity buttons
    vx_decr_btn = newwin(BTN_SIZE_Y, BTN_SIZE_X, vx_buttons_starty, decr_button_startx);
    vx_stp_button = newwin(BTN_SIZE_Y, BTN_SIZE_X, vx_buttons_starty, stp_button_startx);
    vx_incr_btn = newwin(BTN_SIZE_Y, BTN_SIZE_X, vx_buttons_starty, incr_button_startx);

    // Vertical velocity buttons
    vz_decr_btn = newwin(BTN_SIZE_Y, BTN_SIZE_X, vz_buttons_starty, decr_button_startx);
    vz_stp_button = newwin(BTN_SIZE_Y, BTN_SIZE_X, vz_buttons_starty, stp_button_startx);
    vz_incr_btn = newwin(BTN_SIZE_Y, BTN_SIZE_X, vz_buttons_starty, incr_button_startx);
}

// Draw button with colored background
void draw_btn(WINDOW *btn, char* msg, int color) {

    wbkgd(btn, COLOR_PAIR(color));
    wmove(btn, BTN_SIZE_Y / 2, (BTN_SIZE_X - strlen(msg)) / 2);

    attron(A_BOLD);
    wprintw(btn, msg);
    attroff(A_BOLD);

    wrefresh(btn);
}

// Draw all buttons, prepending label message
void draw_buttons() {

    char* msg = "Command Console Buttons";
    move((LINES - 2 - BTN_SIZE_Y - 8) / 2, (COLS - strlen(msg)) / 2);
    attron(A_BOLD);
    printw(msg);
    attroff(A_BOLD);

    draw_btn(vx_decr_btn, "Vx-", 1);
    draw_btn(vx_stp_button, "STP", 2);
    draw_btn(vx_incr_btn, "Vx+", 3);

    draw_btn(vz_decr_btn, "Vz-", 1);
    draw_btn(vz_stp_button, "STP", 2);
    draw_btn(vz_incr_btn, "Vz+", 3);
}

// Utility method to check if button has been pressed
int check_button_pressed(WINDOW *btn, MEVENT *event) {

    if(event->y >= btn->_begy && event->y < btn->_begy + BTN_SIZE_Y) {
        if(event->x >= btn->_begx && event->x < btn->_begx + BTN_SIZE_X) {
            return TRUE;
        }
    }
    return FALSE;

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

    init_pair(1, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_WHITE, COLOR_BLUE);

    // Initialize UI elements
    refresh();
    make_buttons();

    // draw UI elements
    draw_buttons();

    // Activate input listening (keybord + mouse events ...)
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
}

void reset_console_ui() {

    // Free resources
    delwin(vx_decr_btn);
    delwin(vx_incr_btn);
    delwin(vz_decr_btn);
    delwin(vz_incr_btn);

    // Clear screen
    erase();

    // Re-create UI elements
    refresh();
    make_buttons();

    // draw UI elements
    draw_buttons();
}

