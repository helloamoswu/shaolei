//
//  main.c
//  shaolei
//
//  Created by amos on 15-1-10.
//  Copyright (c) 2015年 amos. All rights reserved.
//


#include <curses.h>
#include <stdlib.h>
#include <time.h>
//#include <string.h>

#define StartX 2
#define StartY 1

void initEnv();
void initMap(int row, int col);
void initGame();
void createGame();
void play(int row, int col);
void findBoom(int row, int col);
void gameOver();


int **map = NULL;
int boomHasFinded = 0;
int numOfRow, numOfCol, numOfBoom;
int maxW, maxH;
int offsetX, offsetY;

void initEnv()
{
    initscr();
    start_color();
    cbreak();
    nonl();
    noecho();
    intrflush(stdscr,false);
    keypad(stdscr,true);
    refresh();
}

void initMap(int row, int col)
{
    numOfRow = row;
    numOfCol = col;
    
    numOfBoom = numOfRow * numOfCol / 10 + rand()%10;
    maxH = numOfRow;
    maxW = numOfCol * 2;
    
    offsetX = (78 - maxW) / 2;
    offsetY = (22 - maxH) / 2;
}

void createMenu()
{
    // print border box
    box(stdscr,'|','-');
    attron(A_REVERSE);
    mvaddstr(0,36,"Shao Lei");
    attroff(A_REVERSE);
    
    mvaddstr(3, 30, "Select a map:");
    mvaddstr(4, 35, "1  10 x 10");
    mvaddstr(5, 35, "2  15 X 15");
    mvaddstr(6, 35, "3  20 x 20");
    mvaddstr(7, 35, "4  quit");

    int menu = 4;
    move(menu, 35);
    
    int ch;
    do {
        ch = getch();
        switch (ch) {
            case KEY_UP:
                menu == 4 ? (menu = 7) : (menu--);
                move(menu, 35);
                break;
            case KEY_DOWN:
                menu == 7 ? (menu = 4) : (menu++);
                move(menu, 35);
                break;
            case 32:
                switch (menu) {
                    case 4:
                        initMap(10, 10);
                        return;
                    case 5:
                        initMap(15, 15);
                        return;
                    case 6:
                        initMap(20, 20);
                        return;
                    case 7:
                        endwin();
                        exit(1);
                }
                break;
            default:
                break;
        }
    } while (1);
}

void initGame()
{
    boomHasFinded = 0;
    // free map if map is not empty
    if (map) {
        for (int r = 0; r < numOfRow + 2; r++) {
            free(map[r]);
        }
        free(map);
        map = NULL;
    }
    // create a menu to select a map
    createMenu();
    clear();
    refresh();
    // create memory for the selected map
    map = (int **)malloc((numOfRow + 2) * sizeof(int*));
    for (int r = 0; r < numOfRow + 2; r++) {
        map[r] = (int *)malloc((numOfCol + 2) * sizeof(int));
    }
    // initial the map with 0
    for (int r = 0; r < numOfRow + 2; r++) {
        for (int c = 0; c < numOfCol + 2; c++) {
            map[r][c] = 0;
        }
    }
    // generate random booms
    srand((unsigned)time(0));
    
    int n = 0;
    int randr,randc;
   
    while (n++ < numOfBoom) {
        do {
            randr = rand()%numOfRow + 1;
            randc = rand()%numOfCol + 1;
        } while (map[randr][randc] == -1);
        map[randr][randc] = -1;
    }
}

void createGame()
{
    initGame();
    // calculate the number of each position in the map
    for (int i = 1; i <= numOfRow; i++) {
        
        for (int j = 1; j <= numOfCol; j++) {
            
            if (map[i][j] != -1) {
                int n = 0;
                for (int x = -1; x < 2; x++) {
                    for (int y = -1; y < 2; y++) {
                        if (map[i + x][j + y] == -1) {
                            n++;
                        }
                    }
                }
                map[i][j] = n;
            }
            // hide the number
            mvaddstr(i + offsetY, j*2-1+offsetX, " *");
        }
    }
}

void gameOver()
{
    attron(A_BLINK);
    mvaddstr(1, 20, "Game Over! Please enter [r] to restart!");
    attroff(A_BLINK);
}

void openHide(int row, int col)
{
    // out of bound detection
    if (row < 1 || row > numOfRow || col < 1 || col > numOfCol) {
        return;
    }
    // prevent dead loop
    map[row][col] = -2;
    // show those position that is around 0
    for (int x = -1; x < 2; x++) {
        for (int y = -1; y < 2; y++) {
            if (map[row + x][col + y] == 0) {
                openHide(row + x, col + y);
            } else {
                int t;
                map[row+x][col+y] == -2 ? (t = 0) : (t = map[row+x][col+y]);
                char str[3];
                sprintf(str, "%2d", t);
                mvaddstr(row + x + offsetY, (col + y) * 2 - 1 + offsetX, str);
            }
        }
    }
}


void play(int row, int col)
{
    if (map[row][col] == -1) {
        mvaddstr(row + offsetY, col * 2 - 1 + offsetX, " @");
        gameOver();
        return;
    } else if (map[row][col] == -2) {
        return;
    }
    char str[3];
    sprintf(str, "%2d", map[row][col]);
    mvaddstr(row + offsetY, col * 2 - 1 + offsetX, str);
    if (map[row][col] == 0) {
        openHide(row, col);
    }
}

void findBoom(int row, int col)
{
    // the position is not a boom
    if (map[row][col] != -1) {
        gameOver();
        return;
    }
    // lol, found a boom
    attron(A_REVERSE);
    mvaddstr(row + offsetY, col * 2 - 1 + offsetX, " B");
    attroff(A_REVERSE);
    // if all the boom is found, the player wins
    if (++boomHasFinded == numOfBoom) {
        attron(A_BLINK);
        mvaddstr(1, 20, "You Winning! Please enter [r] to restart!");
        attroff(A_BLINK);
    }
}

int main(int argc, const char* argv[])
{
    // current position
    int x = StartX;
    int y = StartY;
    // input key
    int ch;
    
    initEnv();
    
    createGame();
    
    move(StartY + offsetY, StartX + offsetX);
    
    do{
        ch = getch();
        switch(ch)
        {
            case KEY_UP:
                y--;
                break;
            case KEY_DOWN:
                y++;
                break;
            case KEY_RIGHT:
                x += 2;
                break;
            case KEY_LEFT:
                x -= 2;
                break;
            case 32:
                play(y, x/2);
                break;
            case 'q': case 'Q':     // quit
                endwin();
                exit(1);
            case 'b': case 'B':     //assume find a boom
                findBoom(y, x/2);
                break;
            case 'r': case 'R':     // restart
                clear();
                refresh();
                createGame();
                break;
            default:
                break;
        }
        if (y > maxH) {
            y = StartY;
        } else if (y < StartY) {
            y = maxH;
        }
        if (x > maxW) {
            x = StartX;
        } else if (x < StartX) {
            x = maxW;
        }
        
        move(y + offsetY,x + offsetX);
        
    } while(1);
    
    return 0;
}

