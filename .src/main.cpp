#include <fstream>
#include <iostream>
#include <graphics.h>
#include <windows.h>
#include <winbgim.h>
#include <time.h>
#include <chrono>
#define dim 60
#define nrLines 13
#define nrColumns 25
#define nrLevels 5
#define nrImagesToLoadInBuffer 15
#define TimePerLevel 100
#define DELAY 50
using namespace std;
using namespace std::chrono;

ifstream fin("map.txt");

char currLevel[nrLines + 3][nrColumns + 3];
char aux[nrLines + 3][nrColumns + 3];
char currLevelNoModify[nrLines + 3][nrColumns + 3];
char currDirection, nextDirection;
char bufferchar[50];
int marioX, marioY, Duration;
int statusLeft, statusRight, statusOnLadder;
int currScore, maxScore, level;
bool finish;
auto start = steady_clock::now();
auto Now = steady_clock::now();
int nrLives;
int nrEnemies;
int enemyTimeLoop;
void* buffer[100];
struct enemy
{
    int lin, col;
    int lin_init, col_init;
    int direc;
    int time;
    bool active;
}D[10];
void loadImages()
{
    char s[15][20] = { "mushroom.jpg", "background.jpg", "grass.jpg", "heart.jpg", "mario1.jpg", "mario2.jpg", "mario3.jpg",
    "mario4.jpg", "mario5.jpg", "mario6.jpg", "mario7.jpg", "mario8.jpg", "stone.jpg", "ladder.jpg", "star.jpg" };
    int i;
    for (i = 0; i < nrImagesToLoadInBuffer; i++)
    {
        readimagefile(s[i], 0, 0, dim, dim);
        buffer[i] = malloc(imagesize(0, 0, dim, dim));
        getimage(0, 0, dim, dim, buffer[i]);
        cleardevice();
    }
}
void textBox(int x1, int y1, int x2, int y2, char* text)
{
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(x1, y1, x2, y2);

    setcolor(BLACK);
    rectangle(x1, y1, x2, y2);

    setbkcolor(LIGHTBLUE);
    settextstyle(GOTHIC_FONT, HORIZ_DIR, 5);
    int textWidth = textwidth(text);
    int textHeight = textheight(text);
    outtextxy(x1 + (x2 - x1 - textWidth) / 2, y1 + (y2 - y1 - textHeight) / 2, text);
}

bool isButtonClicked(int x1, int y1, int x2, int y2, int mouseX, int mouseY)
{
    return mouseX >= x1 && mouseX <= x2 && mouseY >= y1 && mouseY <= y2;
}
void clearScreen()
{
    setbkcolor(BLACK);
    cleardevice();
}
void clearMario(int i, int j)
{
    if (aux[i][j] == '.' || aux[i][j] == 'M' || aux[i][j] == 'X')
        putimage(dim * j, dim * i, buffer[1], COPY_PUT);
    if (aux[i][j] == '#')
        putimage(dim * j, dim * i, buffer[13], COPY_PUT);
    if (aux[i][j] == '*')
        putimage(dim * j, dim * i, buffer[1], COPY_PUT);
}
void clearEnemy(int i, int j)
{
    if (aux[i][j] == '.' || aux[i][j] == 'X')
        putimage(dim * j, dim * i, buffer[1], COPY_PUT);
    if (aux[i][j] == '#')
        putimage(dim * j, dim * i, buffer[13], COPY_PUT);
    if (aux[i][j] == '*')
        putimage(dim * j, dim * i, buffer[14], COPY_PUT);
}
void displayEnemy(int i, int j)
{
    putimage(dim * j, dim * i, buffer[0], COPY_PUT);
}
void displayMario(int i, int j)
{
    if (currDirection == 'w' || currDirection == 's' || aux[marioX][marioY] == '#')
    {
        switch (statusOnLadder)
        {
        case 1:  putimage(dim * j, dim * i, buffer[10], COPY_PUT); break;
        case 2:  putimage(dim * j, dim * i, buffer[11], COPY_PUT); break;
        }
        return;
    }
    if (currDirection == 'a')
    {
        switch (statusLeft)
        {
        case 1: putimage(dim * j, dim * i, buffer[7], COPY_PUT); break;
        case 2: putimage(dim * j, dim * i, buffer[8], COPY_PUT); break;
        case 3: putimage(dim * j, dim * i, buffer[9], COPY_PUT); break;
        }
        return;
    }
    if (currDirection == 'd')
    {
        switch (statusRight)
        {
        case 1: putimage(dim * j, dim * i, buffer[4], COPY_PUT);break;
        case 2: putimage(dim * j, dim * i, buffer[5], COPY_PUT);break;
        case 3: putimage(dim * j, dim * i, buffer[6], COPY_PUT);break;
        }
        return;
    }
}
void displayImage(char symbol, int i, int j)
{
    if (symbol == '@')
    {
        if (i == 0)
        {
            if (j <= 2)
                putimage(dim * j, dim * i, buffer[3], COPY_PUT);
            else
                putimage(dim * j, dim * i, buffer[12], COPY_PUT);
        }
        else
            putimage(dim * j, dim * i, buffer[2], COPY_PUT);
    }
    if (symbol == '#')
        putimage(dim * j, dim * i, buffer[13], COPY_PUT);
    if (symbol == '*')
        putimage(dim * j, dim * i, buffer[14], COPY_PUT);
    if (symbol == '.')
        putimage(dim * j, dim * i, buffer[1], COPY_PUT);
    if (symbol == 'X')
        putimage(dim * j, dim * i, buffer[0], COPY_PUT);

}
void nextStatus()
{
    if (nextDirection == 'a')
    {
        if (currDirection != 'a')
            statusLeft = 1;
        else
        {
            if (++statusLeft > 3)
               statusLeft = 1;
        }
        if (aux[marioX][marioY - 1] == '#')
            statusOnLadder = 1;
    }
    else if (nextDirection == 'd')
    {
        if (currDirection != 'd')
            statusRight = 1;
        else
        {
            if (++statusRight > 3)
                statusRight = 1;
        }
        if (aux[marioX][marioY + 1] == '#')
            statusOnLadder = 1;
    }
    else if (nextDirection == 'w' || nextDirection == 's')
    {
        if (currDirection == 'a' || currDirection == 'd' && aux[marioX][marioY] != '#')
            statusOnLadder = 1;
        else
        {
            if (++statusOnLadder > 2)
                statusOnLadder = 1;
        }
    }

}
void gameOver()
{
    PlaySound("game_over.wav", NULL, SND_ASYNC);
    displayEnemy(marioX, marioY);
    readimagefile("game_over.jpg", getmaxx() / 3, 100, getmaxx() - getmaxx() / 3, getmaxy() / 3);
    char key;
    while (true)
    {
        key = getch();
        if ((int)key == 13)
        {
            break;
        }
    }
    Duration = TimePerLevel;
    start = steady_clock::now();
    PlaySound("start_game.wav", NULL, SND_ASYNC);
}
void resetLevel()
{
    clearScreen();
    currScore = maxScore = 0;
    finish = 0;
    nrEnemies = 0;
    nrLives = 3;
    statusRight = 1;
    statusLeft = 1;
    statusOnLadder = 1;
    currDirection = 'd';
    int i, j;
    char s[5];
    for (i = 0; i < nrLines; i++)
    {
        for (j = 0; j < nrColumns; j++)
        {
            currLevel[i][j] = aux[i][j] = currLevelNoModify[i][j];
            if (currLevel[i][j] == 'M')
            {
                marioX = i;
                marioY = j;
                aux[i][j] = '.';
                displayMario(marioX, marioY);
            }
            displayImage(currLevel[i][j], i, j);
            if (currLevel[i][j] == 'X')
            {
                nrEnemies++;
                aux[i][j] = '.';
            }
            if (currLevel[i][j] == '*')
                maxScore += 10;
        }
    }
    setfillstyle(SOLID_FILL, BLACK);
    bar(3 * dim, 0, 7 * dim - 20, 20);
    outtextxy(1330, 23, "Score: ");
    outtextxy(1450, 23, "0");
    sprintf(s, "Level %d", level);
    outtextxy(3 * dim, 30, s);
}
void resetEnemies()
{
    int i;
    for (i = 0; i < nrEnemies; i++)
    {
        displayImage(aux[D[i].lin][D[i].col], D[i].lin, D[i].col);
        currLevel[D[i].lin][D[i].col] = aux[D[i].lin][D[i].col];
        D[i].lin = D[i].lin_init;
        D[i].col = D[i].col_init;
        D[i].direc = 1;
        D[i].active = 1;
        currLevel[D[i].lin][D[i].col] = 'X';
        displayEnemy(D[i].lin, D[i].col);
    }
    currLevel[marioX][marioY] = 'M';
}
void loseLife()
{
    nrLives--;
    currDirection = 'd';
    nextStatus();
    currLevel[marioX][marioY] = aux[marioX][marioY];
    displayEnemy(marioX, marioY);
    if (nrLives == 2)
        putimage(dim * 2, 0, buffer[12], COPY_PUT);
    if (nrLives == 1)
        putimage(dim, 0, buffer[12], COPY_PUT);
    if (nrLives == 0)
    {
        putimage(0, 0, buffer[12], COPY_PUT);
        gameOver();
        resetEnemies();
        resetLevel();
        return;
    }
    PlaySound("contact_with_enemy.wav", NULL, SND_ASYNC);
    marioX = 11; marioY = 0;
    delay(DELAY * 20);
    displayMario(marioX, marioY);
}
void updateScore()
{
    char s[5];
    if (aux[marioX][marioY] == '*')
    {
        PlaySound("collect_star.wav", NULL, SND_ASYNC);
        currScore += 10;
        aux[marioX][marioY] = '.';
        sprintf(s, "%d", currScore);
        outtextxy(1450, 23, s);
        if (currScore == maxScore)
            finish = 1;
    }
}
void eliminateEnemy()
{
    int i;
    char s[5];
    PlaySound("fall_on_enemy.wav", NULL, SND_ASYNC);
    for (i = 0; i < nrEnemies; i++)
    {
        if (D[i].lin == marioX && D[i].col == marioY)
        {
            displayMario(marioX, marioY);
            currLevel[marioX][marioY] = 'M';
            D[i].active = 0;
        }
    }
    maxScore += 50;
    currScore += 50;
    sprintf(s, "%d", currScore);
    outtextxy(1450, 23, s);
}
void marioFalls()
{
    while (currLevel[marioX + 1][marioY] != '@')
    {
        currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); marioX++;
        if (currLevel[marioX][marioY] == 'X')
            eliminateEnemy();
        else
        {
            currLevel[marioX][marioY] = 'M';
            displayMario(marioX, marioY); delay(DELAY / 2);
        }
        updateScore();
        if (finish == 1 || aux[marioX][marioY] == '#') return;
    }
}
void up()
{
    currDirection = 'w';
    currLevel[marioX][marioY] = '#'; clearMario(marioX, marioY); currLevel[--marioX][marioY] = 'M';
    displayMario(marioX, marioY); delay(DELAY);
}
void _left()
{
    currDirection = 'a';
    currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioY;
    if (currLevel[marioX][marioY] == 'X')
    {
        loseLife();
        resetEnemies();
    }
    else
    {
        currLevel[marioX][marioY] = 'M';
        displayMario(marioX, marioY); delay(DELAY);
    }

    updateScore();
    if (finish == 1) return;

    if (aux[marioX][marioY] == '.' && (aux[marioX][marioY + 1] == '#' || aux[marioX + 1][marioY + 1] == '@'))
    {
        marioFalls();
        updateScore();
        if (finish == 1) return;
    }
}
void down()
{
    currDirection = 's';
    currLevel[marioX][marioY] = '#'; clearMario(marioX, marioY); ++marioX;

    if (currLevel[marioX][marioY] == 'X')
        eliminateEnemy();
    else
    {
        currLevel[marioX][marioY] = 'M';
        displayMario(marioX, marioY); delay(DELAY);
    }
}
void _right()
{
    currDirection = 'd';
    currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); ++marioY;
    if (currLevel[marioX][marioY] == 'X')
    {
        loseLife();
        resetEnemies();
    }
    else
    {
        currLevel[marioX][marioY] = 'M';
        displayMario(marioX, marioY); delay(DELAY);
    }
    updateScore();
    if (finish == 1) return;

    if (aux[marioX][marioY] == '.' && (aux[marioX][marioY - 1] == '#' || aux[marioX + 1][marioY - 1] == '@'))
    {
        marioFalls();
        updateScore();
        if (finish == 1) return;
    }
}
void levelClear()
{
    PlaySound("level_clear.wav", NULL, SND_ASYNC);
    readimagefile("level_clear.jpg", getmaxx() / 3, 100, getmaxx() - getmaxx() / 3, getmaxy() / 3);
    char key;
    while (true)
    {
        key = getch();
        if ((int)key == 13)
        {
            break;
        }
    }
    PlaySound("start_game.wav", NULL, SND_ASYNC);
}
bool insideMap(int i, int j)
{
    return i >= 0 && i < nrLines && j >= 0 && j < nrColumns;
}
void updateEnemies()
{
    int i;
    enemyTimeLoop = (enemyTimeLoop + 1) % 30;
    for (i = 0; i < nrEnemies; i++)
    {
        if (enemyTimeLoop == D[i].time && D[i].active == 1)
        {
            if (insideMap(D[i].lin, D[i].col + D[i].direc) && aux[D[i].lin][D[i].col + D[i].direc] != '@' &&
                aux[D[i].lin + 1][D[i].col + D[i].direc] == '@')
            {
                currLevel[D[i].lin][D[i].col] = '.'; clearEnemy(D[i].lin, D[i].col);
                if (currLevel[D[i].lin][D[i].col + D[i].direc] == 'M')
                {
                    currLevel[D[i].lin][D[i].col + D[i].direc] = 'X';
                    D[i].col += D[i].direc;
                    loseLife();
                    resetEnemies();
                    break;
                }
                currLevel[D[i].lin][D[i].col + D[i].direc] = 'X';
                D[i].col += D[i].direc;
                displayEnemy(D[i].lin, D[i].col);
            }
            else
            {
                D[i].direc *= (-1);
                if (insideMap(D[i].lin, D[i].col + D[i].direc) && aux[D[i].lin][D[i].col + D[i].direc] != '@' &&
                    aux[D[i].lin + 1][D[i].col + D[i].direc] == '@')
                {
                    currLevel[D[i].lin][D[i].col] = '.'; clearEnemy(D[i].lin, D[i].col);
                    if (currLevel[D[i].lin][D[i].col + D[i].direc] == 'M')
                    {
                        currLevel[D[i].lin][D[i].col + D[i].direc] = 'X';
                        D[i].col += D[i].direc;
                        loseLife();
                        resetEnemies();
                        break;
                    }
                    currLevel[D[i].lin][D[i].col + D[i].direc] = 'X';
                    D[i].col += D[i].direc;
                    displayEnemy(D[i].lin, D[i].col);
                }
            }
        }
    }
}
void jump()
{
    if ((marioY == 0 && currDirection == 'a') || marioY == nrColumns - 1)
    {
        currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX;
        if (currLevel[marioX][marioY] == 'X')
        {
            loseLife();
            resetEnemies();
            return;
        }
        else
        {
            currLevel[marioX][marioY] = 'M';
            displayMario(marioX, marioY); delay(DELAY);

            updateScore();
            if (finish == 1) return;
        }
        if (aux[marioX - 1][marioY] != '@')
        {
            currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX;
            if (currLevel[marioX][marioY] == 'X')
            {
                loseLife();
                resetEnemies();
                return;
            }
            else
            {
                currLevel[marioX][marioY] = 'M';
                displayMario(marioX, marioY); delay(DELAY);
                updateScore();
                if (finish == 1) return;
            }
        }
        if (aux[marioX + 1][marioY] != '@')
            marioFalls();
        return;
    }
    if (currDirection == 'a')
    {
        if (aux[marioX - 1][marioY] != '@' && aux[marioX - 2][marioY] != '@')
        {
            currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX;
            if (currLevel[marioX][marioY] == 'X')
            {
                loseLife();
                resetEnemies();
                return;
            }
            else
            {
                currLevel[marioX][marioY] = 'M';
                displayMario(marioX, marioY); delay(DELAY);

                updateScore();
                if (finish == 1) return;
            }
            if (aux[marioX - 1][marioY - 1] != '@')
            {
                currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX; --marioY;
                if (currLevel[marioX][marioY] == 'X')
                {
                    loseLife();
                    resetEnemies();
                    return;
                }
                else
                {
                    currLevel[marioX][marioY] = 'M';
                    displayMario(marioX, marioY); delay(DELAY);

                    updateScore();
                    if (finish == 1) return;
                }
                if (aux[marioX][marioY] == '#') return;

                if (insideMap(marioX, marioY - 1) && aux[marioX][marioY - 1] != '@')
                {
                    currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioY;
                    if (currLevel[marioX][marioY] == 'X')
                    {
                        loseLife();
                        resetEnemies();
                        return;
                    }
                    else
                    {
                        currLevel[marioX][marioY] = 'M';
                        displayMario(marioX, marioY); delay(DELAY);
                        updateScore();
                        if (finish == 1) return;
                    }
                }
            }
            else
            {
                currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX;
                if (currLevel[marioX][marioY] == 'X')
                {
                    loseLife();
                    resetEnemies();
                    return;
                }
                else
                {
                    currLevel[marioX][marioY] = 'M';
                    displayMario(marioX, marioY); delay(DELAY);

                    updateScore();
                    if (finish == 1) return;
                }
            }
            if (aux[marioX + 1][marioY] != '@' && aux[marioX + 1][marioY] != '#')
                marioFalls();
        }
        else if (aux[marioX - 1][marioY] != '@' && aux[marioX - 2][marioY] == '@')
        {
            currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX;
            if (currLevel[marioX][marioY] == 'X')
            {
                loseLife();
                resetEnemies();
                return;
            }
            else
            {
                currLevel[marioX][marioY] = 'M';
                displayMario(marioX, marioY); delay(DELAY);

                updateScore();
                if (finish == 1) return;
            }
            if (aux[marioX + 1][marioY - 1] != '@')
            {
                currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); ++marioX; --marioY;
                if (currLevel[marioX][marioY] == 'X')
                    eliminateEnemy();
                else
                {
                    currLevel[marioX][marioY] = 'M';
                    displayMario(marioX, marioY); delay(DELAY);
                }
                updateScore();
                if (finish == 1) return;
            }
            else
            {
                currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioY;
                if (currLevel[marioX][marioY] == 'X')
                {
                    loseLife();
                    resetEnemies();
                    return;
                }
                else
                {
                    currLevel[marioX][marioY] = 'M';
                    displayMario(marioX, marioY); delay(DELAY);
                    updateScore();
                    if (finish == 1) return;
                }
            }
            if (aux[marioX + 1][marioY] != '@' && aux[marioX + 1][marioY] != '#')
                marioFalls();
        }
    }
    else if (currDirection == 'd')
    {
        if (aux[marioX - 1][marioY] != '@' && aux[marioX - 2][marioY] != '@')
        {
            currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX;
            if (currLevel[marioX][marioY] == 'X')
            {
                loseLife();
                resetEnemies();
                return;
            }
            else
            {
                currLevel[marioX][marioY] = 'M';
                displayMario(marioX, marioY); delay(DELAY);

                updateScore();
                if (finish == 1) return;
            }
            if (aux[marioX - 1][marioY + 1] != '@')
            {
                currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX; ++marioY;
                if (currLevel[marioX][marioY] == 'X')
                {
                    loseLife();
                    resetEnemies();
                    return;
                }
                else
                {
                    currLevel[marioX][marioY] = 'M';
                    displayMario(marioX, marioY); delay(DELAY);

                    updateScore();
                    if (finish == 1) return;
                }
                if (aux[marioX][marioY] == '#') return;

                if (insideMap(marioX, marioY + 1) && aux[marioX][marioY + 1] != '@')
                {
                    currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); ++marioY;
                    if (currLevel[marioX][marioY] == 'X')
                    {
                        loseLife();
                        resetEnemies();
                        return;
                    }
                    else
                    {
                        currLevel[marioX][marioY] = 'M';
                        displayMario(marioX, marioY); delay(DELAY);

                        updateScore();
                        if (finish == 1) return;
                    }
                }
            }
            else
            {
                currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX;
                if (currLevel[marioX][marioY] == 'X')
                {
                    loseLife();
                    resetEnemies();
                    return;
                }
                else
                {
                    currLevel[marioX][marioY] = 'M';
                    displayMario(marioX, marioY); delay(DELAY);

                    updateScore();
                    if (finish == 1) return;
                }
            }
            if (aux[marioX + 1][marioY] != '@' && aux[marioX + 1][marioY] != '#')
                marioFalls();
        }
        else if (aux[marioX - 1][marioY] != '@' && aux[marioX - 2][marioY] == '@')
        {
            currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); --marioX;
            if (currLevel[marioX][marioY] == 'X')
            {
                loseLife();
                resetEnemies();
                return;
            }
            else
            {
                currLevel[marioX][marioY] = 'M';
                displayMario(marioX, marioY); delay(DELAY);

                updateScore();
                if (finish == 1) return;
            }
            if (aux[marioX + 1][marioY + 1] != '@')
            {
                currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); ++marioX; ++marioY;
                if (currLevel[marioX][marioY] == 'X')
                    eliminateEnemy();
                else
                {
                    currLevel[marioX][marioY] = 'M';
                    displayMario(marioX, marioY); delay(DELAY);
                }
                updateScore();
                if (finish == 1) return;
            }
            else
            {
                currLevel[marioX][marioY] = '.'; clearMario(marioX, marioY); ++marioY;
                if (currLevel[marioX][marioY] == 'X')
                {
                    loseLife();
                    resetEnemies();
                    return;
                }
                else
                {
                    currLevel[marioX][marioY] = 'M';
                    displayMario(marioX, marioY); delay(DELAY);

                    updateScore();
                    if (finish == 1) return;
                }
            }
            if (aux[marioX + 1][marioY] != '@' && aux[marioX + 1][marioY] != '#')
                marioFalls();
        }
    }
}
void nextLevel()
{
    setfillstyle(SOLID_FILL, WHITE);
    setcolor(WHITE);
    int i, j, k;
    currScore = maxScore = 0;
    finish = 0;
    nrEnemies = 0;
    nrLives = 3;
    statusRight = 1;
    statusLeft = 1;
    statusOnLadder = 1;
    currDirection = 'd';
    char c, s[5];
    for (i = 0; i < nrLines; i++)
    {
        for (j = 0; j < nrColumns; j++)
        {
            fin >> currLevel[i][j];
            aux[i][j] = currLevel[i][j];
            currLevelNoModify[i][j] = currLevel[i][j];
            if (currLevel[i][j] == 'M')
            {
                marioX = i;
                marioY = j;
                aux[i][j] = '.';
                displayMario(marioX, marioY);
            }
            displayImage(currLevel[i][j], i, j);
            if (currLevel[i][j] == 'X')
            {
                nrEnemies++;
                aux[i][j] = '.';
            }
            if (currLevel[i][j] == '*')
                maxScore += 10;
        }
    }
    fin >> c;
    k = 0;
    for (i = 0; i < nrLines; i++)
    {
        for (j = 0; j < nrColumns; j++)
        {
            if (currLevel[i][j] == 'X')
            {
                D[k].lin = D[k].lin_init = i;
                D[k].col = D[k].col_init = j;
                D[k].direc = 1;
                D[k].active = 1;
                D[k].time = rand() % 29;
                k++;
            }
        }
    }
    for (i = 0; i < nrLines; i++)
        currLevel[i][nrColumns] = currLevel[i][nrColumns + 1] = aux[i][nrColumns] = aux[i][nrColumns + 1] = '.';
    settextstyle(GOTHIC_FONT, HORIZ_DIR, 2);
    setfillstyle(SOLID_FILL, BLACK);
    bar(3 * dim, 0, 7 * dim - 20, 20);
    outtextxy(1330, 23, "Score: ");
    outtextxy(1450, 23, "0");
    sprintf(s, "Level %d", level);
    outtextxy(3 * dim, 30, s);
}

void mainMenu()
{
    int x1, y1, x2, y2;
    PlaySound("SuperMarioBros.wav", NULL, SND_ASYNC);
    x1 = getmaxx() / 4; y1 = getmaxy() / 2;
    x2 = getmaxx() / 3 + 600; y2 = getmaxy() / 2 + 100;
    readimagefile("menu_background.jpeg", 0, 0, getmaxx(), getmaxy());

    textBox(x1, y1, x2, y2, "Start");
    x1 = getmaxx() / 4, y1 = 500;
    x2 = getmaxx() / 3 + 600, y2 = 600;
    textBox(x1, y1, x2, y2, "Instructions");
    readimagefile("x.jpg", 1473, 4, 1522, 45);

    while (true)
    {
        if (ismouseclick(WM_LBUTTONDOWN))
        {
            int mouseX, mouseY;
            getmouseclick(WM_LBUTTONDOWN, mouseX, mouseY);
            if (isButtonClicked(getmaxx() / 4, getmaxy() / 2, getmaxx() / 3 + 600, getmaxy() / 2 + 100, mouseX, mouseY))
            {
                PlaySound("start_game.wav", NULL, SND_ASYNC);
                clearScreen();
                break;
            }
            if (isButtonClicked(getmaxx() / 4, 500, getmaxx() / 3 + 600, 600, mouseX, mouseY))
            {
                setfillstyle(SOLID_FILL, COLOR(185, 122, 87));
                bar(getmaxx() / 4, getmaxy() / 2, getmaxx() / 3 + 600, 600);
                readimagefile("x.jpg", 1062, 397, 1105, 431);
                settextstyle(GOTHIC_FONT, HORIZ_DIR, 2);
                setbkcolor(COLOR(185, 122, 87));
                outtextxy(getmaxx() / 4 + 25, getmaxy() / 2 + 50, "A, D - move left/right");
                outtextxy(getmaxx() / 4 + 25, getmaxy() / 2 + 70, "W, S - climb up/down the ladder");
                outtextxy(getmaxx() / 4 + 25, getmaxy() / 2 + 90, "Space - jump");
                outtextxy(getmaxx() / 4 + 25, getmaxy() / 2 + 110, "The goal: collect stars and avoid enemies!");
                while (true)
                {
                    if (ismouseclick(WM_LBUTTONDOWN))
                    {
                        getmouseclick(WM_LBUTTONDOWN, mouseX, mouseY);
                        if (isButtonClicked(1062, 397, 1105, 431, mouseX, mouseY))
                        {
                            clearScreen();
                            x1 = getmaxx() / 4; y1 = getmaxy() / 2;
                            x2 = getmaxx() / 3 + 600; y2 = getmaxy() / 2 + 100;
                            readimagefile("menu_background.jpeg", 0, 0, getmaxx(), getmaxy());

                            textBox(x1, y1, x2, y2, "Start");
                            x1 = getmaxx() / 4, y1 = 500;
                            x2 = getmaxx() / 3 + 600, y2 = 600;
                            textBox(x1, y1, x2, y2, "Instructions");
                            readimagefile("x.jpg", 1473, 4, 1522, 45);
                            break;
                        }
                    }
                }
            }
            if (isButtonClicked(1473, 4, 1522, 45, mouseX, mouseY))
            {
                exit(1);
                closegraph();
            }
        }
    }

}
int main()
{
    srand(time(NULL));
    initwindow(getmaxwidth(), getmaxheight() - 50, "Mario");
    loadImages();
    mainMenu();
    int i;
    char key, s1[5];
    for (i = 1; i <= nrLevels; i++)
    {
        level = i;
        nextLevel();
        start = steady_clock::now();
        Duration = TimePerLevel;
        while (true)
        {
            setfillstyle(SOLID_FILL, BLACK);

            Now = steady_clock::now();
            int timeAmount = duration_cast<seconds>(Now - start).count();
            int timeRemaining = Duration - timeAmount;
            if (timeRemaining == 9 || timeRemaining == 99)
                bar(3 * dim, 0, 8 * dim - 20, 20);
            sprintf(bufferchar, "Remaining time: %d", timeRemaining);
            outtextxy(3 * dim, 0, bufferchar);

            if (timeAmount >= Duration)
            {
                outtextxy(3 * dim, 0, "time's up!");
                gameOver();
                resetEnemies();
                resetLevel();
            }
            updateEnemies();
            if (kbhit())
            {
                key = getch();
                if (key == 119 && aux[marioX][marioY] == '#' && aux[marioX - 1][marioY] == '#' && marioX > 0)
                {
                    nextDirection = 'w';
                    nextStatus();
                    up();
                }
                if (key == 97 && marioY > 0 && currLevel[marioX][marioY - 1] != '@')
                {
                    nextDirection = 'a';
                    nextStatus();
                    _left();
                    if (finish == 1) break;
                }
                if (key == 115 && aux[marioX][marioY] == '#' && aux[marioX + 1][marioY] == '#' && marioX < nrLines - 1)
                {
                    nextDirection = 's';
                    nextStatus();
                    down();
                }
                if (key == 100 && marioY < nrColumns - 1 && currLevel[marioX][marioY + 1] != '@')
                {
                    nextDirection = 'd';
                    nextStatus();
                    _right();
                    if (finish == 1) break;
                }
                if (key == 32 && marioX != 0 && aux[marioX + 1][marioY] == '@' && aux[marioX][marioY] != '#' && aux[marioX - 1][marioY] != '@')
                {
                    PlaySound("mario_jumps.wav", NULL, SND_ASYNC);
                    jump();
                    if (finish == 1) break;
                }
                if (key == 27) exit(1);
            }
            delay(DELAY / 10);
        }
        levelClear();
        clearScreen();
    }
    closegraph();
    return 0;
}
