#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>

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

// Generates a random number in the range (x - 0.05*x, x + 0.05*x)
float add_error(float x)
{
    float upper_limit = 0.05 * x;
    float r = ((float)rand()/(float)(RAND_MAX)) * upper_limit;
    int n = rand() % 2; // Randomly generate 0 or 1
    if (n == 0) return x + r;
    else return x - r;
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
    log_file = fopen("log/world.log", "w");
	if (log_file == NULL) {
        perror("Error while opening the log file");
        return 1;
    }

    // Get the file descriptor of the read side of pipe xhat
    int read_fd_xhat;
    sscanf(argv[1], "%d", &read_fd_xhat);

    // Get the file descriptor of the read side of pipe zhat
    int read_fd_zhat;
    sscanf(argv[2], "%d", &read_fd_zhat);

    // Get the file descriptor of the write side of pipe x
    int write_fd_x;
    sscanf(argv[3], "%d", &write_fd_x);

    // Get the file descriptor of the write side of pipe z
    int write_fd_z;
    sscanf(argv[4], "%d", &write_fd_z);

    srand(time(NULL)); // Seed for random number generator

    // Variables needed for the use of select()
    fd_set rfds;
    struct timeval tv;
    int retval, n;

    // Current estimated values of the x and z positions of the e.e. (before adding the error)
    float x_hat = 0.0, z_hat = 0.0;
    
    // Current actual values of the x and z positions of the e.e. (after adding the error)
    float x = 0.0, z = 0.0;

    while(1)
    {
        FD_ZERO(&rfds); // Initialize the array where we'll store the set of fd's that we want to 'select' for reading
        FD_SET(read_fd_xhat, &rfds); // Add the fd of the read side of pipe xhat
        FD_SET(read_fd_zhat, &rfds); // Add the fd of the read side of pipe zhat
        tv.tv_sec = 1.0/100.0; // Time that we wait to check again if the status has changed
        tv.tv_usec = 0;

        // Continuously check if there are data available in any of the pipes 'xhat' and 'zhat'
        retval = select(max(read_fd_xhat, read_fd_zhat)+1, &rfds, NULL, NULL, &tv);
        
        // Check the return value of the select() systemcall
        switch (retval)
        {
            case -1:
                perror("world - error in select()");
                return 1;

            case 2: // There are two pipes with data available to be read
                int rand_number = get_random_int(0,1); // Choose one randomnly
                if(rand_number == 0)
                { 
                    read(read_fd_xhat, &x_hat, sizeof(x_hat)); // Read the new estimated x position from pipe 'xhat'
                    x = add_error(x_hat); // Add an error in the range (x_hat - 0.05*x_hat, x_hat + 0.05*x_hat)
                    n = write(write_fd_x, &x, sizeof(x)); // Write the new actual position (with the error) in pipe 'x'
                    if (n == -1) {
                        perror("world: write error");
                        return 1;
                    }
                }
                else
                {
                    read(read_fd_zhat, &z_hat, sizeof(z_hat)); // Read the new estimated z position from pipe 'zhat'
                    z = add_error(z_hat); // Add an error in the range (z_hat - 0.05*z_hat, z_hat + 0.05*z_hat)
                    n = write(write_fd_z, &z, sizeof(z)); // Write the new actual position (with the error) in pipe 'z'
                    if (n == -1) {
                        perror("world: write error");
                        return 1;
                    }
                }
                write_timestamp(); // Report in the log file
                fprintf(log_file, "%s", "New real position: x = ");
                fprintf(log_file, "%f", x);
                fprintf(log_file, "%s", ", z = ");
                fprintf(log_file, "%f\n", z); fflush(log_file);

                break;

            case 1: // There is one pipe with data available to be read
                if(FD_ISSET(read_fd_xhat, &rfds) != 0) // It's the pipe 'xhat'
                {
                    read(read_fd_xhat, &x_hat, sizeof(x_hat)); // Read the new estimated x position from pipe 'xhat'
                    x = add_error(x_hat); // Add an error in the range (x_hat - 0.05*x_hat, x_hat + 0.05*x_hat)
                    n = write(write_fd_x, &x, sizeof(x)); // Write the new actual position (with the error) in pipe 'x'
                    if (n == -1) {
                        perror("world: write error");
                        return 1;
                    }
                }
                else if(FD_ISSET(read_fd_zhat, &rfds) != 0) // 'It's the pipe zhat'
                {
                    read(read_fd_zhat, &z_hat, sizeof(z_hat)); // Read the new estimated z position from pipe 'zhat'
                    z = add_error(z_hat); // Add an error in the range (z_hat - 0.05*z_hat, z_hat + 0.05*z_hat)
                    n = write(write_fd_z, &z, sizeof(z)); // Write the new actual position (with the error) in pipe 'z'
                    if (n == -1) {
                        perror("world: write error");
                        return 1;
                    }
                }
                write_timestamp(); // Report in the log file
                fprintf(log_file, "%s", "New real position: x = ");
                fprintf(log_file, "%f", x);
                fprintf(log_file, "%s", ", z = ");
                fprintf(log_file, "%f\n", z); fflush(log_file);

                break;
        }
    }

    fclose(log_file);
    return 0;
}
