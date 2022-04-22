#include <ncurses.h>
#include <sys/time.h>
#include<time.h>
#include <signal.h>
#include<stdlib.h>

#define LEFT 260
#define RIGHT 261
#define UP 259
#define DOWN 258
#define ESC 27
#define ENTER 10

#define NOTHING 0

#define WAIT_INPUT 0
#define CONTINUE 1
#define MIDDLE 2

#define WIDTH 120
#define HEIGHT 60

typedef struct Node{
    int x;
    int y;
    struct Node *next;
} Node;

Node *snake;
Node *foods;
int input;
int dir;
volatile int status;

WINDOW* win;

void sig_alarm_handler();
Node *delete(Node *p,int y,int x);
void addFood();
Node *search(Node *source,Node *lt);
int game_play();
Node *delete(Node *p,int y,int x);

int main()
{	
	struct itimerval timer_set;
	timer_set.it_interval.tv_usec=timer_set.it_value.tv_usec=500000;
	timer_set.it_interval.tv_sec=timer_set.it_value.tv_sec=0;

    srand((unsigned) time(0));

	initscr();
	raw();
	noecho();
	keypad(stdscr, TRUE);

    WINDOW* border=newwin(HEIGHT+2, WIDTH+2,4,0);
    WINDOW* title=newwin(4, WIDTH+2,0,0);
    wborder(border, '|', '|', '-', '-', '+', '+', '+', '+');
    wborder(title, '|', '|', '-', ' ', '+', '+', '|', '|');
    wrefresh(border);
    mvwprintw(title,1,WIDTH/3,"Use UP DOWN LEFT RIGHT to control the snake");
    mvwprintw(title,2,WIDTH/3,"Press ESC to quit");
    wrefresh(title);

    win = newwin(HEIGHT, WIDTH, 5 ,1);
    keypad(win, TRUE);

	mvwprintw(win,HEIGHT/2,WIDTH/3,"Press Enter to play");
    int c=wgetch(win);
    if(c==ENTER)
    {
        signal(SIGALRM,sig_alarm_handler);
	    setitimer(ITIMER_REAL,&timer_set,0);
        
        mvwprintw(win,HEIGHT/2,WIDTH/3,"                           ");
    
        while(1)
        {
            wclear(win);
            nodelay(win,1);
            dir=DOWN;
            input=NOTHING;
            snake=malloc(sizeof(Node));
            snake->next=malloc(sizeof(Node));
            snake->next->next=NULL;
            snake->y=HEIGHT/2+1;
            snake->next->y=HEIGHT/2;
            snake->x=snake->next->x=WIDTH/2;
            mvwprintw(win,snake->y,snake->x,"X");
            mvwprintw(win,snake->next->y,snake->next->x,"X");
            wrefresh(win);
            if(game_play()==0)
                break;
            nodelay(win,0);
            wclear(win);
            mvwprintw(win,HEIGHT/2,WIDTH/3,"YOU LOSE !!!Press anything to restart");
            wrefresh(win);
            if(wgetch(win)==ESC)
                break;
        }    
    }
    delwin(win);
    endwin();
	return 0;
} 

void sig_alarm_handler(){
	status=CONTINUE;
    wrefresh(win);
}

void addFood(){
    Node *p=(Node *)malloc(sizeof(Node));
    p->next=foods;
    foods=p;
    while(1)
    {
        p->x=rand()%WIDTH;
        p->y=rand()%HEIGHT;
        if(search(p,snake)==NULL)
        {
            mvwprintw(win,p->y,p->x,"O");
            return;
        }
    }
}

Node *search(Node *source,Node *lt){
    for(;lt;lt=lt->next)
        if(source->x==lt->x&&source->y==lt->y)
            return lt;
    return NULL;
}


int game_play(){
    while(1)
    {   
        switch (status)
        {
            case WAIT_INPUT:
                int i=wgetch(win);
                if(i!=ERR)
                    input=i;
                break;
            case CONTINUE:   
                    for(Node *x=snake;;x=x->next)
                    {
                        if(x->next->next==NULL)
                        {
                            mvwprintw(win,x->next->y,x->next->x," ");
                            free(x->next);
                            x->next=NULL;
                            wrefresh(win);
                            break;
                        }
                    }

            case MIDDLE:

                {       if(input==ESC)
                            return 0;
                        if(input==UP||input==DOWN||input==LEFT||input==RIGHT)
                            dir=input;
                        input=NOTHING;
                        Node *p=(Node *)malloc(sizeof(Node));

                        switch (dir)
                        {
                        case UP:
                            p->x=snake->x;
                            p->y=snake->y-1;
                            break;
                        case DOWN:
                            p->x=snake->x;
                            p->y=snake->y+1;
                            break;
                        case LEFT:
                            p->x=snake->x-1;
                            p->y=snake->y;
                            break;
                        case RIGHT:
                            p->x=snake->x+1;
                            p->y=snake->y;
                            break;
                        }

                        if((p->x>=WIDTH)||(p->y>=HEIGHT)||(p->x<0)||(p->y<0))
                        {
                            mvwprintw(win,HEIGHT/2,WIDTH/3,"YOU LOSE !!!");
                            return 1;
                        }
                        p->next=snake;
                        snake=p;
                        mvwprintw(win,snake->y,snake->x,"X");

                        if(search(p,foods)!=NULL)
                        {
                            status=MIDDLE;
                            foods=delete(foods,p->y,p->x);
                        }
                        else
                            status=WAIT_INPUT;                   
                        wrefresh(win);
                }
            default:
                addFood();
        }
    }
}

Node *delete(Node *p,int y,int x){
    Node *ptr=p->next;
    if(p->x==x&&p->y==y)
    {
        free(p);
        return ptr;
    }
    for(ptr=p;ptr->next;ptr=ptr->next)
    {
        if(ptr->next->x==x&&ptr->next->y==y)
            {
                Node *x=ptr->next->next;
                free(ptr->next);
                ptr->next=x;
            }
    }
    return p;
}