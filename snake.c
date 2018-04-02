/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>

/* direction definitions for easy to read code */
#define DOWN 0
#define UP 1
#define LEFT 2
#define RIGHT 3

/* game area size */
#define WIDTH 20
#define HEIGHT 20

typedef struct snakepiece
{
	/* snakepiece is a doubly linked list */
	int x, y; /* current x and y position */
	int oldx, oldy; /* previous x and y position */
	struct snakepiece *prev; /* will point to the previous piece or null */
	struct snakepiece *next; /* will point to the piece in front or null */
} piece;

typedef struct snakehead
{
	int x, y; /* current head position */
	int oldx, oldy; /* old x and y position */
	int direction; /* current direction the head will move */
	int score; /* incremented when food is eaten */
	piece *firstpiece; /* will point to the head of the snakepiece linked list or null */
	int state; /* game state int, 1 = running 0 = quit */
	int npc; /* determines if the snake controls itself 1 = bot 0 = human */
	WINDOW *area; /* terrible place to store the play area window */
} head;

typedef struct foodpiece
{
	int x, y; /* food piece location */
} food;

void *update(head *snake);
void draw(head *snake, food *eat);
void setup_food(head *snake, food *eat);
void create_body(head *snake);
void npc_logic(head *snake, food *eat);

int main(void)
{
	/* seed rand */
	srand(time(NULL));
	/* ncurses setup */
	initscr();
	noecho();
	curs_set(0);
	timeout(50);
	/* terminal width and height */
	int w, h;
	getmaxyx(stdscr, h, w);
	/* setup snake */
	head *snake;
	snake->x = 0;
	snake->y = 5;
	snake->oldx = snake->x;
	snake->oldy = snake->y;
	snake->direction = RIGHT;
	snake->state = 1;
	snake->npc = 0; /* start asa huma nplayer */
	snake->firstpiece = NULL;
	/* setup game area */
	snake->area = newwin(HEIGHT, WIDTH, 1, 8);
	/* setup update thread */
	pthread_t updatethread;
	pthread_create(&updatethread, NULL, update, snake); /* we pass our snake pointer to update(...) */
	/* input loop */
	int ch; /* stores our current input */
	while(snake->state) /* while state == 1 aka while game running */
	{
		ch = getch();
		switch(ch)
		{
		/* direction key detection */
		case 's':
		case 'j':
			/* prevent changing direction into yourself */
			if (snake->direction != UP)
				snake->direction = DOWN;
			break;
		case 'w':
		case 'k':
			/* prevent changing direction into yourself */
			if (snake->direction != DOWN)
				snake->direction = UP;
			break;
		case 'a':
		case 'h':	
			/* prevent changing direction into yourself */
			if (snake->direction != RIGHT)
				snake->direction = LEFT;
			break;
		case 'd':
		case 'l':
			/* prevent changing direction into yourself */
			if (snake->direction != LEFT)
				snake->direction = RIGHT;
			break;
		case 'q': /* q quits the game */
			snake->state = 0;
			break;
		case 'b': /* b toggles bot playing */
			snake->npc = !snake->npc;
			break;
		default:
			break;
		}
	}
	/* end ncurses for a sane terminal */
	endwin();
	printf("final score: %d\n", snake->score);
	return 0;
}

void *update(head *snake)
{
	food eat;
	setup_food(snake, &eat);
	while(1)
	{
		/* store snake x and y, the head body piece needs to know this */
		snake->oldx = snake->x;
		snake->oldy = snake->y;
		/* move the snake towards the direction it's pointing */
		switch(snake->direction)
		{
		case DOWN:
			snake->y++;
			break;
		case UP:
			snake->y--;
			break;
		case LEFT:
			snake->x--;
			break;
		case RIGHT:
			snake->x++;
			break;
		default:
			break;
		}
		/* detect if head is touching the border */
		if (snake->x == WIDTH-1 ||
		    snake-> y == HEIGHT-1 ||
		    snake->x == 0 ||
		    snake->y == 0)
			snake->state = 0;
		/* check if food eaten */
		if (snake->x == eat.x &&
		    snake->y == eat.y)
		{
			/* create a body piece */
			create_body(snake);
			snake->score++;
			setup_food(snake, &eat);
		}
		/* check if touching any body part */
		piece *finger = snake->firstpiece;
		while (finger != NULL)
		{
			if (snake->x == finger->x && snake->y == finger->y)
			{
				snake->state = 0; /* exit the game */
			}
			finger = finger->next;
		}

		/* update body pieces position if any exist */
		finger = snake->firstpiece;
		while (finger != NULL)
			{
				/* store our old x and y, the finger->next needs it */
				finger->oldx = finger->x;
				finger->oldy = finger->y;
				/* check if we are the head of the list, if we are prev will be NULL */
				if (finger->prev == NULL)
				{
					finger->x = snake->oldx;
					finger->y = snake->oldy;
				}
				else
				{
					finger->x = finger->prev->oldx;
					finger->y = finger->prev->oldy;
				}
				finger = finger->next;
			}
		free(finger);
		/* check if the game should play itself */
		if (snake->npc)
			npc_logic(snake, &eat);
		draw(snake, &eat);
		/* sleep to keep game at a reasonable speed */
		usleep(200000);
		//usleep(60000);
	}
}

void draw(head *snake, food *eat)
{
	/* clear the game area */
	wclear(snake->area);
	/* box our game area */
	box(snake->area, 0, 0);
	/* draw the snake head */
	mvwaddch(snake->area, snake->y, snake->x, ACS_BLOCK);
	/* draw the food */
	mvwaddch(snake->area, eat->y, eat->x, '*');
	/* render body pieces if any exist */
	piece *finger = snake->firstpiece;
	while (finger != NULL)
	{
		mvwaddch(snake->area, finger->y, finger->x, ACS_BLOCK);
		finger = finger->next;
	}
	free(finger);
	mvprintw(0, 0, "score: %d\n", snake->score);
	mvprintw(HEIGHT+2, 0, "  'wasd/hjkl' to control the snake.\n" \
			      "  'b' to toggle bot control.\n" \
			      "  'q' to quit.");
	refresh();
	wrefresh(snake->area);
}

void setup_food(head *snake, food *eat)
{	
	/* set random x and y positions */
	/* restart label in the event food touches snake */
	restart:
	eat->x = rand() % WIDTH;
	eat->y = rand() % HEIGHT;
	/* check if the food is touching the nsake or a body piece */
	if (eat->x == snake->x && eat->y == snake->y)
		goto restart;
	piece *finger = snake->firstpiece;
	while (finger != NULL)
	{	
		if (eat->x == finger->x && eat->y == finger->y)
			goto restart;
		finger = finger->next;
	}
	free(finger);
	/* make sure the food is in bounds */
	if (eat->x < 3 ||
	    eat->x > WIDTH-3)
		goto restart;
	if (eat->y < 3 ||
	    eat->y > HEIGHT-3)
		goto restart;
}

void create_body(head *snake)
{
	piece *new = (piece *) malloc(sizeof(piece)); /* allocate memory for our new piece */
	/* check if a head body piece exists */
	if (snake->firstpiece == NULL)
	{
		/* a head body piece doesn't exist, make it */
		/* set our initial x and y */
		new->x = 0;
		new->y = 0;
		/* these aren't used without updating, so the value doesn't really matter */
		new->oldx = 0;
		new->oldy = 0;
		/* our head wont will never have a prev */
		new->prev = NULL;
		snake->firstpiece = new;
		/* we have nothing else to do, so return */
		return;
	}
	/* we aren't the head, so add a new piece */
	/* allocate memory to store our previous piece */
	new->prev = malloc(sizeof(piece));
	//piece *finger = (piece *) malloc(sizeof(piece));
	piece *finger = snake->firstpiece;
	/* loop through every body piece */
	while (finger->next != NULL)
	{
		finger = finger->next;
	}
	/* finger is now the last piece, set it as new->prev */
	new->prev = finger;
	/* set our initial x and y */
	new->x = 0;
	new->y = 0;
	/* these aren't used without updating, so the values doesn't really matter */
	new->oldx = 0;
	new->oldy = 0;
	/* set our new piece as the last piece in the list */
	finger->next = new;
}

void npc_logic(head *snake, food *eat)
{
	if (snake->y > eat->y)
		snake->direction = UP;
	else if (snake->y < eat->y)
		snake->direction = DOWN;
	else if (snake->x > eat->x)
		snake->direction = LEFT;
	else if (snake->x < eat->x)
		snake->direction = RIGHT;
	}
