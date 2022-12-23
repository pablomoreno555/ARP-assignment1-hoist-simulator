#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>

// Variables needed for the log file
FILE * log_file;
time_t rawtime;
struct tm * timeinfo;
char * timestamp;

// Creates an unnamed pipe, storing its file descriptors in the specified array
int create_pipe(int fd[2])
{
  if (pipe(fd) == -1) return 1; // Create pipe
  if (fcntl(fd[0], F_SETFL, O_NONBLOCK) < 0) return 1; // Make it non-blocking

  time(&rawtime); timeinfo = localtime(&rawtime); // Report in the log file
  timestamp = asctime(timeinfo); timestamp[strlen(timestamp)-1] = 0; 
  fprintf(log_file, "%s -- ", timestamp); 
  fprintf(log_file, "%s\n", "Pipe created"); fflush(log_file);

  return 0;
}

// Gets the number of seconds since the specified file was modified
time_t get_seconds_last_modified (char* file_path)
{
  struct stat attr;
  stat(file_path, &attr);
  time_t last_modified_time = attr.st_mtime;

  time_t current_time;
  time(&current_time);

  return current_time - last_modified_time;
}

// Forks and executes the specified program
int spawn(const char * program, char * arg_list[])
{
  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }
  else if(child_pid != 0) {
    return child_pid;
  }
  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}

int main()
{
  // Open the log file
  log_file = fopen("log/master.log", "w");
	if (log_file == NULL) {
    perror("Error while opening the log file");
    return 1;
  }

  // Declare the file descriptors of all pipes
  int fd_vx[2], fd_vz[2], fd_xhat[2], fd_zhat[2], fd_x[2], fd_z[2];

  // Create all pipes
  if (create_pipe(fd_vx) == 1) {
    perror("Error while creating pipe vx");
    return 1;
  }
  if (create_pipe(fd_vz) == 1) {
    perror("Error while creating pipe vz");
    return 1;
  } 
  if (create_pipe(fd_xhat) == 1) {
    perror("Error while creating pipe xhat");
    return 1;
  } 
  if (create_pipe(fd_zhat) == 1) {
    perror("Error while creating pipe zhat");
    return 1;
  }
  if (create_pipe(fd_x) == 1) {
    perror("Error while creating pipe x");
    return 1;
  } 
  if (create_pipe(fd_z) == 1) {
    perror("Error while creating pipe z");
    return 1;
  } 

  // Convert file descriptors of pipe "i" into the strings "read_fd_i" and "write_fd_i", for all pipes:

  char read_fd_vx[10], write_fd_vx[10]; // pipe vx
  if (sprintf(read_fd_vx, "%d", fd_vx[0]) < 0) {
    perror("Error using sprintf");
    return 1;
  } 
  if (sprintf(write_fd_vx, "%d", fd_vx[1]) < 0) {
    perror("Error using sprintf");
    return 1;
  }
  char read_fd_vz[10], write_fd_vz[10]; // pipe vz
  if (sprintf(read_fd_vz, "%d", fd_vz[0]) < 0) { 
    perror("Error using sprintf");
    return 1;
  } 
  if (sprintf(write_fd_vz, "%d", fd_vz[1]) < 0) {
    perror("Error using sprintf");
    return 1;
  }
  char read_fd_xhat[10], write_fd_xhat[10]; // pipe xhat
  if(sprintf(read_fd_xhat, "%d", fd_xhat[0]) < 0) {
    perror("Error using sprintf");
    return 1;
  }
  if (sprintf(write_fd_xhat, "%d", fd_xhat[1]) < 0) {
    perror("Error using sprintf");
    return 1;
  }
  char read_fd_zhat[10], write_fd_zhat[10]; // pipe zhat
  if (sprintf(read_fd_zhat, "%d", fd_zhat[0]) < 0) {
    perror("Error using sprintf");
    return 1;
  }
  if (sprintf(write_fd_zhat, "%d", fd_zhat[1]) < 0) {
    perror("Error using sprintf");
    return 1;
  }
  char read_fd_x[10], write_fd_x[10]; // pipe x
  if (sprintf(read_fd_x, "%d", fd_x[0]) < 0) {
    perror("Error using sprintf");
    return 1;
  }
  if (sprintf(write_fd_x, "%d", fd_x[1]) < 0 ) {
    perror("Error using sprintf");
    return 1;
  } 
  char read_fd_z[10], write_fd_z[10]; // pipe z
  if (sprintf(read_fd_z, "%d", fd_z[0]) < 0) {
    perror("Error using sprintf");
    return 1;
  }
  if (sprintf(write_fd_z, "%d", fd_z[1]) < 0) {
    perror("Error using sprintf");
    return 1;
  }

  // Declare the lists of arguments for all children processes, including the corresponding pipes' file descriptors
  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", write_fd_vx, write_fd_vz, NULL };
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", read_fd_x, read_fd_z,  NULL };
  char * arg_list_motor1[] = { "./bin/motor1", read_fd_vx, write_fd_xhat, NULL};
  char * arg_list_motor2[] = { "./bin/motor2", read_fd_vz, write_fd_zhat, NULL};
  char * arg_list_world[] = { "./bin/world", read_fd_xhat, read_fd_zhat, write_fd_x, write_fd_z, NULL};

  // Spawn all children processes
  pid_t pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_t pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);
  pid_t pid_motor1 = spawn("./bin/motor1", arg_list_motor1);
  pid_t pid_motor2 = spawn("./bin/motor2", arg_list_motor2);
  pid_t pid_world = spawn("./bin/world", arg_list_world);


  // ** WATCHDOG ** //

  sleep(8); // Wait some time for all processes to create their log files before starting to check the watchdog condition
  
  int program_active = 1;
  while(program_active == 1) 
  {
    // Get the number of seconds since the log file of each child process was modified
    time_t t_command = get_seconds_last_modified("log/command.log");
    time_t t_inspection = get_seconds_last_modified("log/inspection.log");
    time_t t_motor1 = get_seconds_last_modified("log/motor1.log");
    time_t t_motor2 = get_seconds_last_modified("log/motor2.log");
    time_t t_world = get_seconds_last_modified("log/world.log");

    // If none of the log files was modified in the last 60 seconds, it means that none of the child processes performed any action
    if (t_command>60 && t_inspection>60 && t_motor1>60 && t_motor2>60 && t_world>60)
    {
      // Kill all children processes
      kill(pid_cmd, SIGKILL);
      kill(pid_insp, SIGKILL);
      kill(pid_motor1, SIGKILL);
      kill(pid_motor2, SIGKILL);
      kill(pid_world, SIGKILL);

      fclose(log_file); // Close the log file of the master

      kill(getpid(), SIGKILL); // Kill the master itself

      program_active = 0;
    }
  }

  int status;
  waitpid(pid_cmd, &status, 0);
  waitpid(pid_insp, &status, 0);
  waitpid(pid_motor1, &status, 0);
  waitpid(pid_motor2, &status, 0);
  waitpid(pid_world, &status, 0);
  printf ("Main program exiting with status %d\n", status);

  fclose(log_file);
  return 0;
}

