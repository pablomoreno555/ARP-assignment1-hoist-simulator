#include "./../include/command_utilities.h"
#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>
#include <time.h>

// Velocity command sent to the motors: 0 -> v-, 1 -> stop, 2 -> v+
char command_to_send; 

// Variables needed for the log file
FILE * log_file;
time_t rawtime;
struct tm * timeinfo;
char * timestamp;

// Writes the current time in the log file
void write_timestamp ()
{
    time(&rawtime); timeinfo = localtime(&rawtime);
    timestamp = asctime(timeinfo); timestamp[strlen(timestamp)-1] = 0; 
    fprintf(log_file, "%s -- ", timestamp);
}

// Signal handler for the STOP signal
void sig_handler_stop(int signo)
{
    if (signo == SIGUSR1) {
        command_to_send = 'X'; // The command will be ignored

        write_timestamp(); // Report in the log file
        fprintf(log_file, "%s\n", "Signal STOP received"); fflush(log_file);
    }
}

// Signal handler for the RESET signal
void sig_handler_reset(int signo)
{
    if (signo == SIGUSR2) {
        command_to_send = 'X'; // The command will be ignored

        write_timestamp(); // Report in the log file
        fprintf(log_file, "%s\n", "Signal RESET received"); fflush(log_file);
    }
}

int main(int argc, char const *argv[])
{
    // Open the log file
    log_file = fopen("log/command.log", "w");
	if (log_file == NULL) {
        perror("Error while opening the log file");
        return 1;
    }

    // Associate the signal SIGUSR1 with the signal handler 'sig_handler_stop'
    if (signal(SIGUSR1, sig_handler_stop) == SIG_ERR)
        printf("Can't catch signal SIGUSR1\n");

    // Associate the signal SIGUSR2 with the signal handler 'sig_handler_reset'
    if (signal(SIGUSR2, sig_handler_reset) == SIG_ERR)
        printf("Can't catch signal SIGUSR2\n");

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();

    // Get the file descriptor of the write side of pipe vx
    int write_fd_vx;
    sscanf(argv[1], "%d", &write_fd_vx);

    // Get the file descriptor of the write side of pipe vz
    int write_fd_vz;
    sscanf(argv[2], "%d", &write_fd_vz);

    // Infinite loop
    while(TRUE)
	{	
        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch(); // Non-blocking function: if there's no input, it simply does nothing

        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {

                // Vx- button pressed
                if(check_button_pressed(vx_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");
                    refresh();
                    sleep(1); // The previous message will stay there 1 second
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    // Send this command to motor1 by writing it in pipe vx
                    command_to_send = '0';
                    int n = write(write_fd_vx, &command_to_send, 1);
                    if (n == -1) {
                        perror("command console: write error");
                        return 1;
                    }
                    // Report in the log file
                    write_timestamp();
                    fprintf(log_file, "%s\n", "\"Vx-\" button pressed"); fflush(log_file);
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    // Send this command to motor1 by writing it in pipe vx
                    command_to_send = '1';
                    int n = write(write_fd_vx, &command_to_send, 1);
                    if (n == -1) {
                        perror("command console: write error");
                        return 1;
                    }
                    // Report in the log file
                    write_timestamp();
                    fprintf(log_file, "%s\n", "\"Vx stop\" button pressed"); fflush(log_file);
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    // Send this command to motor1 by writing it in pipe vx
                    command_to_send = '2';
                    int n = write(write_fd_vx, &command_to_send, 1);
                    if (n == -1) {
                        perror("command console: write error");
                        return 1;
                    }
                    // Report in the log file
                    write_timestamp();
                    fprintf(log_file, "%s\n", "\"Vx+\" button pressed"); fflush(log_file);
                }

                // Vz- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    // Send this command to motor2 by writing it in pipe vz
                    command_to_send = '0';
                    int n = write(write_fd_vz, &command_to_send, 1);
                    if (n == -1) {
                        perror("command console: write error");
                        return 1;
                    }
                    // Report in the log file
                    write_timestamp();
                    fprintf(log_file, "%s\n", "\"Vz-\" button pressed"); fflush(log_file);    
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    // Send this command to motor2 by writing it in pipe vz
                    command_to_send = '1';
                    int n = write(write_fd_vz, &command_to_send, 1);
                    if (n == -1) {
                        perror("command console: write error");
                        return 1;
                    }
                    // Report in the log file
                    write_timestamp();
                    fprintf(log_file, "%s\n", "\"Vz stop\" button pressed"); fflush(log_file);
                }

                // Vz+ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    // Send this command to motor2 by writing it in pipe vz
                    command_to_send = '2';
                    int n = write(write_fd_vz, &command_to_send, 1);
                    if (n == -1) {
                        perror("command console: write error");
                        return 1;
                    }
                    // Report in the log file
                    write_timestamp();
                    fprintf(log_file, "%s\n", "\"Vz+\" button pressed"); fflush(log_file);
                }
            }
        }
        refresh();
	}

    // Terminate
    endwin(); // It disables the graphical mode of the console
    fclose(log_file);
    return 0;
}
