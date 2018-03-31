# snake in c and ncurses

Simple snake game written in C using ncurses. The code is well commented and should step you through the flow nicely. A thread is used for user input. Controls displayed on screen.

# things to change
Change the board width and height by editing:
```
#define WIDTH 20
#define HEIGHT 20
```

Change the speed of the game by altering the int at the bottom of update() passed to usleep(). A recommended value is there commented out.

# compiling and running
compile and run using:
```
gcc snake.c -o snake -lncurses -lpthread && ./snake
```

# example

[asciinema game example](https://asciinema.org/a/N1qgDuEvJpThShuyRr0uR5QtZ)
