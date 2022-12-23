gcc src/master.c -o bin/master
gcc src/inspection_console.c -lncurses -lm -o bin/inspection
gcc src/command_console.c -lncurses -o bin/command
gcc src/motor1.c -o bin/motor1
gcc src/motor2.c -o bin/motor2
gcc src/world.c -o bin/world
