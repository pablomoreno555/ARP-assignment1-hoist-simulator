#include "./../include/inspection_utilities.h"
#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#define LEN 10

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

// Generates a random integer between l and r
int get_random_int(int l, int r)
{
    int rand_num = (rand() % (r - l + 1)) + l;
    return rand_num;
}

// Returns the maximum of the two input numbers
int max(int num1, int num2)
{
    return (num1 > num2 ) ? num1 : num2;
}

int main(int argc, char const *argv[])
{
    // Open the log file
    log_file = fopen("log/inspection.log", "w");
	if (log_file == NULL) {
        perror("Error while opening the log file");
        return 1;
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // End-effector coordinates
    float ee_x = 0.0;
    float ee_z = 0.0;

    // Initialize User Interface 
    init_console_ui();

    // Get the file descriptor of the read side of pipe x
    int read_fd_x;
    sscanf(argv[1], "%d", &read_fd_x);

    // Get the file descriptor of the read side of pipe z
    int read_fd_z;
    sscanf(argv[2], "%d", &read_fd_z);

    srand(time(NULL)); // Seed for random number generator

    // Vbles needed for the use of select()
    fd_set rfds;
    struct timeval tv;
    int retval, n;

    // Infinite loop
    while(TRUE)
	{	
        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();

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

            // Check if a button has been pressed...
            if(getmouse(&event) == OK) {

                // STOP or RESET buttons pressed
                if(check_button_pressed(stp_button, &event) || check_button_pressed(rst_button, &event))
                {
                    // Get the pid of 'command_console'
                    char line[LEN];
                    FILE *cmd_ = popen("pidof -s command", "r");
                    fgets(line, LEN, cmd_);
                    long pid_cmd = 0;
                    pid_cmd = strtoul(line, NULL, 10);

                    // Get the pid of 'motor1'
                    char line2[LEN];
                    FILE *cmd_2 = popen("pidof -s motor1", "r");
                    fgets(line2, LEN, cmd_2);
                    long pid_m1 = 0;
                    pid_m1 = strtoul(line2, NULL, 10);

                    // Get the pid of 'motor2'
                    char line3[LEN];
                    FILE *cmd_3 = popen("pidof -s motor2", "r");
                    fgets(line3, LEN, cmd_3);
                    long pid_m2 = 0;
                    pid_m2 = strtoul(line3, NULL, 10);
                
                    // STOP button pressed...
                    if(check_button_pressed(stp_button, &event))
                    {
                        mvprintw(LINES - 1, 1, "STP button pressed");
                        refresh();
                        sleep(1);
                        for(int j = 0; j < COLS; j++) {
                            mvaddch(LINES - 1, j, ' ');
                        }

                        // Send a STOP signal to the 'command_console' process, to discard the current command given by the user, if any
                        kill(pid_cmd, SIGUSR1);

                        // Send a STOP signal to the 'motor1' process, to stop it immediately
                        kill(pid_m1, SIGUSR1);

                        // Send a STOP signal to the 'motor2' process, to stop it immediately
                        kill(pid_m2, SIGUSR1);

                        // Report in the log file
                        write_timestamp();
                        fprintf(log_file, "%s\n", "STOP button pressed"); fflush(log_file);
                    }

                    // RESET button pressed...
                    else if(check_button_pressed(rst_button, &event))
                    {
                        mvprintw(LINES - 1, 1, "RST button pressed");
                        refresh();
                        sleep(1);
                        for(int j = 0; j < COLS; j++) {
                            mvaddch(LINES - 1, j, ' ');
                        }

                        // Send a RESET signal to the 'command_console' process, to ignore all velocity commands given by the user
                        // while the end effector is rewinding to the zero position
                        kill(pid_cmd, SIGUSR2);

                        // Send a RESET signal to the 'motor1' process, to make it rewind to the zero position
                        kill(pid_m1, SIGUSR2);

                        // Send a RESET signal to the 'motor2' process, to make it rewind to the zero position
                        kill(pid_m2, SIGUSR2); 

                        // Report in the log file
                        write_timestamp();
                        fprintf(log_file, "%s\n", "RESET button pressed"); fflush(log_file); 
                    }
                }
            }
        }

        FD_ZERO(&rfds); // Initialize the array where we'll store the set of fd's that we want to 'select' for reading
        FD_SET(read_fd_x, &rfds); // Add the fd of the read side of pipe x
        FD_SET(read_fd_z, &rfds); // Add the fd of the read side of pipe z
        tv.tv_sec = 1.0/100.0; // Time that we wait to check again if the status has changed
        tv.tv_usec = 0;

        // Continuously check if there are data available in any of the pipes 'x' and 'z'
        retval = select(max(read_fd_x, read_fd_z)+1, &rfds, NULL, NULL, &tv);
        
        // Check the return value of the select() systemcall
        switch (retval)
        {
            case -1:
                perror("inspection console - error in select()");
                return -1;

            case 2: // There are two pipes with data available to be read
                int rand_number = get_random_int(0,1); // Choose one randomnly
                if(rand_number == 0)
                    read(read_fd_x, &ee_x, sizeof(ee_x)); // Read the new actual x position from pipe 'x'
                else
                    read(read_fd_z, &ee_z, sizeof(ee_z)); // Read the new actual z position from pipe 'z'

                write_timestamp(); // Report in the log file
                fprintf(log_file, "%s", "New position of the end effector: x = ");
                fprintf(log_file, "%f", ee_x);
                fprintf(log_file, "%s", ", z = ");
                fprintf(log_file, "%f\n", ee_z); fflush(log_file);

                break;

            case 1: // There is one pipe with data available to be read
                if(FD_ISSET(read_fd_x, &rfds) != 0) // It's the pipe 'x'
                    read(read_fd_x, &ee_x, sizeof(ee_x)); // Read the new actual x position from pipe 'x'

                else if(FD_ISSET(read_fd_z, &rfds) != 0) // It's the pipe 'z'
                    read(read_fd_z, &ee_z, sizeof(ee_z)); // Read the new actual z position from pipe 'z'

                write_timestamp(); // Report in the log file
                fprintf(log_file, "%s", "New position of the end effector: x = ");
                fprintf(log_file, "%f", ee_x);
                fprintf(log_file, "%s", ", z = ");
                fprintf(log_file, "%f\n", ee_z); fflush(log_file);

                break;
        }

        // Update UI with the current x and z positions
        update_console_ui(&ee_x, &ee_z);
	}

    // Terminate
    endwin();
    fclose(log_file);
    return 0;
}
