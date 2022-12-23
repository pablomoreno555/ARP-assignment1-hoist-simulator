# ARP Hoist Assignment
First *Advanced and Robot Programming (ARP)* assignment, by Pablo Moreno Moreno (S5646698).

## Compiling and running the code
In order to compile all the processes that make up the program, you just need to execute the following command:
```console
./compile.sh
```

Then, you can run the program by executing:
```console
./run.sh
```

Two new windows will pop up: one corresponding to the **Command Console** and another corresponding to the **Inspection Console**.


## Content of the Repository
Apart from the *compile.sh* and *run.sh* files, the repository is organized as follows:
- The `src` folder contains the source code for the **Master**, **Command Console**, **Inspection Console**, **Motor1**, **Motor2**, and **World** processes.
- The `bin` folder is where the executable files corresponding to the previous processes are generated after compilation.
- The `include` folder contains all the data structures and methods used within the *ncurses* framework to build the two GUIs.
- The `log` folder is where the log files corresponding to the above processes will be genearated after running the program.


## Project Description
The project consists in designing and developing an interactive simulator of a hoist with 2 d.o.f. in which two different consoles allow the user to activate the hoist.

Two different motors allow the hoist to displace along two differet axes: X (horizontal) and Z (vertical). Motions along the axes have bounds: **(0, 100)** for the X axis and **(0, 40)** for the Z axis.

From the user side, there are two consoles (shell windows) that simulate a real system: the *command console* and the *inspection console*, implemented through the *ncurses* library as simple GUIs.

The program is made up of the following 6 processes:

- **Master** process. It spawns all the other processes and defines all the unnamed pipes used for the IPC. It also implements the *watchdog*: it checks the activity of all the other processes, and in case none of them did anything for 60 seconds, it kills all the processes, including itself.
- **Command Console** process. It reads the velocity commands given by the user through the buttons *vx-*, *vx stop*, *vx+*, *vz-*, *vz stop* and *vz+*, sending them to the appropiate motor. It is also capable of handling the *STOP* and *RESET* signals, ignoring all user inputs.
- **Motor1** process. It simulates the motion along the X axis, receiving the velocity commands from the **Command Console** process, computing the estimated X position of the end effector, and sending it to the **World** process. It is also capable of handling the *STOP* signal (setting the velocity immediately to zero) and the *RESET* signal (rewinding the hoist to the zero position).
- **Motor2** process. It simulates the motion along the Z axis, in the same way as the **Motor1** process, and handles the *STOP* and *RESET* signals in the same way.
- **World** process. It receives the estimated X and Z positions from the **Motor1** and **Motor2** processes, adds a certain error to them, and sends the actual position values of X and Z (including error) to the **Inspection Console** process.
- **Inspection Console** process. It receives the actual X and Z positions from the **Motor1** and **Motor2** processes, and reports the current hoist position on its associated window. It also manages the *STOP* and *RESET* buttons, sending the appropriate signals to the **Command Console**, **Motor1** and **Motor2** processes when they are pressed.


## User Guide
After compiling and running the program as mentioned above, two new windows will pop up: one corresponding to the **Command Console** and another corresponding to the **Inspection Console**.

The user can send velocity commands to the motors by clicking the buttons *vx-*, *vx stop*, *vx+*, *vz-*, *vz stop* and *vz+*, present in the  **Command Console** window. The velocity levels are discretized in the range [-2.0, 2.0] in increments of 0.5.

The current position of the hoist is represented in the **Inspection Console** window. In this window, the user can also activate the *STOP* and *RESET* signals by pressing the corresponding buttons.

After pressing the *STOP* button, the velocities of both motors will be immediately set to zero, regardless of the user commands. Once the hoist has stopped, the user can continue to command it, by sending new velocity commands.

After pressing the *RESET* button, the velocities of both motors will be set to -0.5 until the hoist has reached its initial position. While the hoist is rewinding to its initial position, the velocity commands given by the user will be ignored, but once it has reached the origin, the user can continue to command it, by sending new velocity commands.

Finally, if all processes are inactive during 60 seconds (no motion of the hoist, no pressing of the buttons, etc), the program will end, terminating all processes and closing all windows.


## Required libraries/packages
- *ncurses* library. You can install it with the following command:
```console
sudo apt-get install libncurses-dev
```

- *konsole* application. You can install it with the following command:
```console
sudo apt-get install konsole
```


## Troubleshooting
Should you experience some weird behavior after launching the application (buttons not spawning inside the GUI or graphical assets misaligned) simply try to resize the terminal window, it should solve the bug.
