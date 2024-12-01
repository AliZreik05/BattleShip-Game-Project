#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> //this is needed for the random function so we can guarantee that the whoever gets first serve is random
#include <time.h>   /*this is also needed to set a random seed for the random function, 
                                     this is so we dont get the same sequence of random numbers each time*/

// struct player so we can store characteristics of each player
typedef struct player Player; // instead of writing struct player we just write Player to initialize a struct
struct player
{
    char BattleGround[10][10];
    char name[16];
    char positions[4][4];
    char positions_orientations[5];
    int numberOfShips;
    bool DestroyedShips[5];
    int destroyedAShipInPreviousTurn;
    int radar_sweeps;
    int smoke_screens;
    int detectedTargets[4][2];
    bool DetectedTargets;
    int num_DetectedTargets_ToCheck;
    char obscured_areas[4][3];
    int number_obscured_positions;
    bool torpedoUnlocked;
    bool torpedoAlreadyUnlocked;
    int heatmap[10][10];
    // int score; we can implement this one later when the game can be played more than once
};

// global variables so we can access them anywhere in the code
Player player;
Player bot;
char gameDifficulty;
char botDifficulty;
const char *arsenal[] = {"carrier", "battleship", "destroyer", "submarine"};
int Counter;
bool inAttackingMode;
int checkedDirection;
int alreadyHitSpots[5][2];
int lastSpotHitIndex;
char orientation;
bool correctDirection;
int NumSpotsHitAttackingMode;
bool correctDirection2;
int adjacentHitSpots[5][2];
int numAdjacentHitSpots;
bool are_adjacent_ships;
bool HittingAdjacent;

// methods
void Artillery(int x, int y);
void BotGetPositions();
void BotMakeMove();
bool checkInputValidity(char input[]);
bool checkPositionValidity(char name[], char input[], int lengthOfCaerrier, char BattleGround[10][10], char orientation);
void checkForDestroyedShips(Player *p);
bool checkHeatMapPlacementValidity(int x, int y, char battlefield[10][10], int lengthOfCarrier, char orientation);
void clearScreen();
void displayBattleField(Player *p, char difficulty);
void displayAvailableMoves(Player *p);
void EasyBotArtillery(int x, int y);
void EasyBotTorpedo(int z, bool isRow);
void EasyBotRadarSweep(int x, int y);
void EasyBotFire();
void EasyBotSmokeScreen(int x, int y);
void EasyBotMakeMove();
void Fire(int x, int y);
void fillArrays();
int getRandomTurn();
void getPositions();
void getHeatMap(Player *realPlayer);
void initializePlayers();
void makeMove();
void MediumBotArtillery(int heatmap[10][10]);
void MediumBotTorpedo(int heatmap[10][10]);
void MediumBotRadarSweep(int heatmap[10][10]);
void MediumBotFire(const int heatmap[10][10]);
void MediumBotSmokeScreen(int heatmap[10][10]);
void MediumBotMakeMove();
void performRadarSweep(int x, int y);
void smoke_screen(int x, int y);
char setGameDifficulty();
char setBotDifficulty();
void toUpper1(char input[]); // toUpper1 makes a char[] to uppercase (a string)
void Torpedo(int z, bool isRow);
void updateBattleField(char battlefield[10][10], char position[], char orientation, int lengthOfCarrier, int numberAssociated);
void UpdateHeapMap(int heatmap[10][10], int x, int y, int lengthOfCarrier, char orientation);

int main()
{
    gameDifficulty = setGameDifficulty();
    botDifficulty = setBotDifficulty(); // now we have game and bot difficulties set and a message will be printed to the user
    initializePlayers();                // player1, player2 now have a name, battleground...

    int currentPlayer = getRandomTurn();

    do
    {

        if (currentPlayer == 1)
        {
            printf("%s's turn:\n", player.name);
            displayBattleField(&bot, gameDifficulty);
            displayAvailableMoves(&player);
            makeMove();
            checkForDestroyedShips(&bot);
            currentPlayer = 2;
        }
        else
        {
            printf("Bot's turn:\n");
            displayBattleField(&player, gameDifficulty);
            displayAvailableMoves(&bot);
            BotMakeMove();
            checkForDestroyedShips(&player);
            currentPlayer = 1;
        }
    } while (player.numberOfShips != 0 && bot.numberOfShips != 0);

    printf("Game Over!\n");
    if (player.numberOfShips == 0)
    {
        printf("The Bot is the winner!\n");
    }
    else
    {
        printf("%s is the winner!\n", player.name);
    }

    return 0;
}

// Functions:
void Artillery(int x, int y)
{
    if (player.destroyedAShipInPreviousTurn == 0)
    {
        printf("You cannot use this ability since you did not sink a ship in your past turn\n"); // this is to fullfill the condition that a player needs to sink a ship in his previous turn to be able to use this ability
        return;
    }
    int hit = 0;
    int alreadyHit = 0;
    int miss = 0;
    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            if (bot.BattleGround[i][j] != 0 && bot.BattleGround[i][j] != 5 && bot.BattleGround[i][j] != 6)
            {
                bot.BattleGround[i][j] = 5;
                hit++;
            }
            else if (bot.BattleGround[i][j] == 0)
            {
                bot.BattleGround[i][j] = 6;
                miss++;
            }
            else if (bot.BattleGround[i][j] == 5 || bot.BattleGround[i][j] == 6)
            {
                alreadyHit++;
            }
        }
    }
    printf("You hit %d unharmed ships, %d already harmed ships or already missed spots, and missed on %d shots.\n\n", hit, alreadyHit, miss);
    player.destroyedAShipInPreviousTurn = 0;
    bot.number_obscured_positions = 0; // this is because the effect of smoke screen which
                                       // the bot potentially used expired after a turn
}

void BotGetPositions()
{
    int carriersLength[] = {5, 4, 3, 2};
    for (int i = 0; i < 4; i++)
    {
        bool validInput = false;
        while (validInput == false)
        {
            srand(time(NULL));         // this function allows us to set the a different seed for the random number each time our program runs so we don't have the same sequence of random numbers everytime we run our code
            int randomX = rand() % 10; //%10 gives a number from 0 to 9 so we can generate a random x
            int randomY = rand() % 10; //%10 gives a number from 0 to 9 so we can generate a random y
            int randomOrientation = rand() % 2;
            char ortientation;
            if (randomOrientation == 0)
            {
                ortientation = 'H';
            }
            else
            {
                ortientation = 'V';
            }
            char coordinate[4];
            coordinate[0] = 'A' + randomX;
            if (randomY == 9)
            {
                coordinate[1] = '1';
                coordinate[2] = '0';
                coordinate[3] = '\0';
            }
            else
            {
                coordinate[1] = '1' + randomY;
                coordinate[2] = '\0';
            }
            if (checkPositionValidity(bot.name, coordinate, carriersLength[i], bot.BattleGround, ortientation))
            {
                updateBattleField(bot.BattleGround, coordinate, ortientation, carriersLength[i], i + 1);
                validInput = true;
            }
        }
    }
}

void BotMakeMove()
{
    switch (botDifficulty)
    {
    case 'e':
        EasyBotMakeMove();
        break;
    case 'm':
        MediumBotMakeMove();
        break;
    default:
        break;
    }
}

bool checkInputValidity(char input[]) // so we can check if the position inputed is in the correct format
{
    if (strlen(input) < 2 || strlen(input) > 3) // checking for length, because it should be minimum 2 and max 3 (examples for wrong input:A,2,6,AB5)
    {
        return 0;
    }

    if (strlen(input) == 2) // if it is from A-J and 1-9 only(2 characters long)
    {
        if (input[0] < 'A' || input[0] > 'J')
        {
            return 0;
        }
        else if (input[1] - '0' < 1 || input[1] - '0' > 9)
        {
            return 0;
        }
    }
    else if (strlen(input) == 3) // if it is from A-J and 10
    {
        if (input[0] < 'A' || input[0] > 'J')
        {
            return 0;
        }
        else if (input[1] != '1' || input[2] != '0') // if the input has length 3 then the letter should be followed by 10 (example: A10)
        {
            return 0;
        }
    }
    return 1;
}

bool checkPositionValidity(char name[], char input[], int lengthOfCarrier, char BattleGround[10][10], char orientation) // after checking the input, we check if the position inputted is correct or not
{
    int x = input[0] - 'A';
    int y;
    if (strlen(input) == 2)
    {
        y = input[1] - '1'; // you may wonder why the 2 and 3 and u are correct
    } // if it was made of 2 then it is in the form A0 for example
    else if (strlen(input) == 3) // we can then directly get Y, but if it was 3 characters long,
    {                            // it means the user entered y to be 10 (which is 9 because we work with arrays that start at index 0)
        y = 9;
    }
    // now we have our x and y and we can check for the position validity

    if (orientation == 'V')
    {
        for (int i = y; i < y + lengthOfCarrier; i++)
        {
            if (i >= 10)
            {
                if (strcmp(name, player.name) == 0)                  // we are comparing to check if we are checking the position validity of the player or bot. This is important because we don't want a spam of messages saying that the bot ship position is invalid
                    printf("Ship goes out of bounds vertically.\n"); // if the ship goes out of bounds even though its first position is in bounds
                return 0;
            }
            if (BattleGround[i][x] != 0) // if it wasnt 0, then other ship exists there and the position is not available
            {
                if (strcmp(name, player.name) == 0)
                    printf("Another carrier exists in these coordinates.\n");
                return 0;
            }
        }
    }

    if (orientation == 'H')
    {
        for (int i = x; i < x + lengthOfCarrier; i++)
        {
            if (i >= 10)
            {
                if (strcmp(name, player.name) == 0)
                    printf("Ship goes out of bounds horizontally.\n"); // if the ship goes out of bounds even though its first position is in bounds
                return 0;
            }
            if (BattleGround[y][i] != 0) // if it wasnt 0, then other ship exists there and the position is not available
            {
                if (strcmp(name, player.name) == 0)
                    printf("Another carrier exists in these coordinates.\n");
                return 0;
            }
        }
    }
    return 1;
}

void checkForDestroyedShips(Player *p)
{
    Player *PlayerAttacking = (strcmp(p->name, player.name) == 0) ? &bot : &player;
    int previousNumberOfShips = p->numberOfShips;
    p->numberOfShips = 0;
    int newDestroyedShip;

    // Check for remaining ships
    for (int m = 1; m <= 4; m++)
    { //  ship identifiers are 1 to 4
        bool shipStillExists = false;

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                if (p->BattleGround[i][j] == m)
                {
                    shipStillExists = true; // If we find any part of the ship, mark it as existing
                }
            }
        }
        if (shipStillExists)
        {
            p->numberOfShips++; // Count this ship since it exists
        }
        else
        { // we should update the DestroyedShips so we know which ships are destroyed
            if (p->DestroyedShips[m] == 0)
            {
                newDestroyedShip = m; // if DestroyedShips[m] was false it means the ship wasn't
                                      // destroyed so it just got destroyed. We save the newly
                                      // destroyed ship in an int so we can print this ship later
            }
            p->DestroyedShips[m] = true;
        }
    }

    if (previousNumberOfShips != p->numberOfShips)
    {
        PlayerAttacking->destroyedAShipInPreviousTurn = 1;
        PlayerAttacking->smoke_screens++;
        printf("%s destroyed a %s!\n\n", PlayerAttacking->name, arsenal[newDestroyedShip - 1]); // print which ship just got destroyed
    }
    else
    {
        PlayerAttacking->destroyedAShipInPreviousTurn = 0;
    }

    // Unlock the torpedo if three ships are sunk and it hasn't been unlocked already
    // (the player passed in checkForDestroyedShips() is the one attacked)
    PlayerAttacking->torpedoUnlocked = false;
    if (p->numberOfShips < 2 && PlayerAttacking->torpedoAlreadyUnlocked == false)
    {
        PlayerAttacking->torpedoUnlocked = true;        // Unlock the torpedo
        PlayerAttacking->torpedoAlreadyUnlocked = true; // Prevent future unlocks
    }
}

void clearScreen()
{
#ifdef _WIN32
    system("cls"); // For Windows
#else
    system("clear"); // For Linux/MacOS
#endif
}

/*typica; method to display the playing field, the difficulity char is to determin wether or not to print the misses also not only the hits*/
void displayBattleField(Player *p, char difficulty)
{
    printf("    %c  %c  %c  %c  %c  %c  %c  %c  %c  %c\n", 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J');

    for (int i = 0; i < 10; i++)
    {
        printf("%-4i", i + 1);
        for (int j = 0; j < 10; j++)
        {
            if (p->BattleGround[i][j] == 0 || p->BattleGround[i][j] == 1 || p->BattleGround[i][j] == 2 || p->BattleGround[i][j] == 3 || p->BattleGround[i][j] == 4) // 0 means we never attacked there and 1 to 4 means there is a ship
            {
                printf("~  ");
            }
            else
            {
                difficulty = tolower(difficulty);
                switch (difficulty) // as stated in the given instructions, diffirent dificulities print different stuff hence why i had to take 2 cases
                {
                case 'e':
                    if (p->BattleGround[i][j] == 5)
                    {                  // 5 means hit
                        printf("*  "); // 6 means we shot there but didnt hit anything
                    }
                    else if (p->BattleGround[i][j] == 6)
                    {
                        printf("o  "); // we shot there but didnt hit anything
                    }
                    break;

                case 'h':
                    if (p->BattleGround[i][j] == 5)
                    {
                        printf("*  "); // we hit something
                    }
                    else if (p->BattleGround[i][j] == 6)
                    {
                        printf("~  "); // we dont show if we missed in hard difficulities
                    }
                    break;

                default:
                    break;
                }
            }
        }
        printf("\n");
    }
}

void displayAvailableMoves(Player *p)
{
    printf("Available moves for %s:\n", p->name);
    printf("-Fire\n");
    if (p->radar_sweeps > 0)
    {
        printf("-Radar Sweep (Remaining: %d)\n", p->radar_sweeps);
    }
    if (p->smoke_screens > 0)
    {
        printf("-Smoke Screen (Remaining: %d)\n", p->smoke_screens);
    }
    if (p->destroyedAShipInPreviousTurn == 1)
    {
        printf("-Use Artillery\n");
    }
    if (p->torpedoUnlocked)
    {
        printf("-Use Torpedo\n");
    }
    printf("\n");
}

void EasyBotTorpedo(int z, bool isRow)
{
    printf("Bot fires a torpedo at %s %d\n", isRow ? "row" : "column", z + 1);
    bool hit = false;

    if (isRow)
    {
        for (int col = 0; col < 10; col++)
        {
            if (player.BattleGround[z][col] >= 1 && player.BattleGround[z][col] <= 4)
            {
                player.BattleGround[z][col] = 5; // Mark as hit
                hit = true;
            }
            else if (player.BattleGround[z][col] == 0)
            {
                player.BattleGround[z][col] = 6; // Mark as miss
            }
        }
    }
    else // column
    {
        for (int row = 0; row < 10; row++)
        {
            if (player.BattleGround[row][z] >= 1 && player.BattleGround[row][z] <= 4)
            {
                player.BattleGround[row][z] = 5; // Mark as hit
                hit = true;
            }
            else if (player.BattleGround[row][z] == 0)
            {
                player.BattleGround[row][z] = 6; // Mark as miss
            }
        }
    }

    if (hit)
    {
        printf("Hit!\n");
    }
    else
    {
        printf("Miss!\n");
    }
    bot.torpedoUnlocked = false;          // Reset the torpedo ability
    player.number_obscured_positions = 0; // this is because the effect of smoke screen which the player potentially used expired after a turn
}

void EasyBotArtillery(int x, int y)
{
    int hit = 0;
    int alreadyHit = 0;
    int miss = 0;
    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            if (player.BattleGround[i][j] != 0 && player.BattleGround[i][j] != 5 && player.BattleGround[i][j] != 6)
            {
                player.BattleGround[i][j] = 5;
                hit++;
            }
            else if (player.BattleGround[i][j] == 0)
            {
                player.BattleGround[i][j] = 6;
                miss++;
            }
            else if (player.BattleGround[i][j] == 5 || player.BattleGround[i][j] == 6)
            {
                alreadyHit++;
            }
        }
    }
    printf("Bot hit %d unharmed ships, %d already harmed ships or already missed spots, and missed on %d shots.\n\n", hit, alreadyHit, miss);
    bot.destroyedAShipInPreviousTurn = 0;
    player.number_obscured_positions = 0; // this is because the effect of smoke screen which
                                          // the player potentially used expired after a turn
}

void EasyBotFire()
{
    int x, y;
    bool validMove = false;

    while (validMove == false)
    {
        x = rand() % 10; // Random row
        y = rand() % 10; // random coloumn

        if (gameDifficulty == 'e')
        {
            if (player.BattleGround[x][y] != 5 && player.BattleGround[x][y] != 6)
            {
                validMove = true;
            }
        }
        else // gameDifficulty == 'h'
        {
            if (player.BattleGround[x][y] != 5) // in hard game mode they can target the same location twice because missed shots dont show
            {
                validMove = true;
            }
        }
    }

    if (player.BattleGround[x][y] >= 1 && player.BattleGround[x][y] <= 4)
    {
        player.BattleGround[x][y] = 5; // Hit
        printf("Bot fires at %c%d!\n", 'A' + y, x + 1);
        printf("Hit!\n");
    }
    else if (player.BattleGround[x][y] == 0)
    {
        player.BattleGround[x][y] = 6; // Miss
        printf("Bot fires at %c%d!\n", 'A' + y, x + 1);
        printf("Miss!\n");
    }
    else
    {
        printf("Bot fires at %c%d!\n", 'A' + y, x + 1);
        printf("Already Targeted this location!\n");
    }

    player.number_obscured_positions = 0; // this is because the effect of smoke screen which the player potentially used expired after a turn
}

void EasyBotSmokeScreen(int x, int y)
{
    bot.number_obscured_positions = 0;

    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            sprintf(bot.obscured_areas[bot.number_obscured_positions], "%d%d", i, j);
            bot.number_obscured_positions++;
        }
    }

    bot.smoke_screens--;
    player.number_obscured_positions = 0; // this is because the effect of smoke screen which the player potentially used expired after a turn
}

void EasyBotRadarSweep(int x, int y)
{
    printf("Bot uses Radar Sweep at %c%d\n", 'A' + y, x + 1);

    // set previous DetectedTargets boolean to false and the number of detected targets to check back to 0
    bot.num_DetectedTargets_ToCheck = 0;
    bot.DetectedTargets = false;
    char ij[3]; // to store ij position to compare it to obscured positions

    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            if (player.BattleGround[i][j] <= 4 && player.BattleGround[i][j] >= 1) // 1,2,3,4 represent ships
            {
                bot.DetectedTargets = true;
                for (int k = 0; k < player.number_obscured_positions; k++)
                {
                    sprintf(ij, "%d%d", i, j);
                    if (strcmp(player.obscured_areas[k], ij) == 0)
                    {
                        bot.DetectedTargets = false;
                        break;
                    }
                }
            }
            if (bot.DetectedTargets == true)
                break;
        }
        if (bot.DetectedTargets == true)
            break;
    }

    if (bot.DetectedTargets == true)
    {
        for (int i = 0; i < 4; i++)
        {
            if (i == 0)
            {
                // Step 1: Keep x and y the same
            }
            else if (i == 1)
            {
                // Step 2: Increment x
                x++;
            }
            else if (i == 2)
            {
                // Step 3: Increment both x and y
                y++;
            }
            else if (i == 3)
            {
                // Step 4: Increment only y
                x--; // this way we return x to its initial value
            }

            if (x >= 0 && x < 10 && y >= 0 && y < 10)
            {
                bot.detectedTargets[i][0] = x;
                bot.detectedTargets[i][1] = y;
                bot.num_DetectedTargets_ToCheck++;
            }
            else
            {
                // the spot is outside the grid
                bot.detectedTargets[i][0] = -1;
                bot.detectedTargets[i][1] = -1;
            }
        }
        printf("Enemy ships found!\n");
    }

    else
    {
        printf("No enemy ships found!\n");
    }

    bot.radar_sweeps--;                   // Decrement radar sweeps after use
    player.number_obscured_positions = 0; // this is because the effect of smoke screen which the player potentially used expired after a turn
    printf("\n");
}

void EasyBotMakeMove()
{
    srand(time(NULL));

    if (bot.torpedoUnlocked) // if unlocked bot should use it and prioritize it over artillery
    {
        printf("Bot chooses to use Torpedo!\n");
        int z = rand() % 10;
        bool isRow = rand() % 2;
        EasyBotTorpedo(z, isRow);
    }
    else if (bot.destroyedAShipInPreviousTurn == 1) // if artillery is unlocked bot should use it
    {
        printf("Bot chooses to perform Artillery!\n");
        int x = rand() % 10;
        int y = rand() % 10;
        EasyBotArtillery(x, y);
    }
    else if (bot.DetectedTargets == true) // if bot found ships in radar sweep it should shoot there
    {
        // Prioritize targeting detected ships, so that if the bot uses radar sweep it won't continue hitting randomly (which would defeat the point of using radar sweep)
        int targetX = -1;
        int targetY = -1;
        int targetIndex = -1;
        while (targetX == -1 && targetY == -1)
        {
            targetIndex = rand() % 4; // because the number of ships found could be 4 max
            targetX = bot.detectedTargets[targetIndex][0];
            targetY = bot.detectedTargets[targetIndex][1];
        }
        bot.detectedTargets[targetIndex][0] = -1; // Remove the detected target position from detected
        bot.detectedTargets[targetIndex][1] = -1; // targets (action is done so no need to keep it stored)
        bot.num_DetectedTargets_ToCheck--;

        printf("Bot fires at detected target %c%d\n", 'A' + targetY, targetX + 1);

        if (player.BattleGround[targetX][targetY] >= 1 && player.BattleGround[targetX][targetY] <= 4)
        {
            player.BattleGround[targetX][targetY] = 5; // Hit
            printf("Hit!\n");
        }
        else
        {
            if (player.BattleGround[targetX][targetY] == 0)
                player.BattleGround[targetX][targetY] = 6;
            if (player.BattleGround[targetX][targetY] == 5)
                player.BattleGround[targetX][targetY] = 5;
            printf("Miss!\n");
        }

        if (bot.num_DetectedTargets_ToCheck == 0)
        {
            bot.DetectedTargets = false; // we finished checking all radar spots with potential ships
        }
    }
    else
    {
        // Normal random move if no torpedo or artillery unlocked or detected targets (if no sweeps were used or if they were used but no find)
        int randomChoice = rand() % 100;

        if (randomChoice < 80)
        {
            printf("Bot chooses to Fire randomly!\n");
            EasyBotFire();
        }
        else if (randomChoice >= 80 && randomChoice < 85)
        {
            if (bot.smoke_screens > 0)
            {
                printf("Bot chooses to use Smoke Screen!\n");
                int x = rand() % 10;
                int y = rand() % 10;
                EasyBotSmokeScreen(x, y);
            }
            else
            {
                printf("Bot attempts to use Smoke Screen, but has none left. Bot loses its turn.\n");
            }
        }
        else if (randomChoice >= 85 && randomChoice < 90)
        {
            if (bot.radar_sweeps > 0)
            {
                printf("Bot chooses to perform a Radar Sweep!\n");
                int x = rand() % 10;
                int y = rand() % 10;
                EasyBotRadarSweep(x, y);
            }
            else
            {
                printf("Bot attempts to use Radar Sweep, but has none left. Bot loses its turn.\n");
            }
        }
        else if (randomChoice >= 90 && randomChoice < 95)
        {
            printf("Bot attempts to use Torpedo, but it is not unlocked. Bot loses its turn.\n");
        }
        else
        {
            printf("Bot attempts to use Artillery, but didn't destroy a ship in previous turn. Bot loses its turn.\n");
        }
    }
}

void Fire(int x, int y)
{
    // Check if the game is on easy mode and if the location was already targeted
    if (gameDifficulty == 'e' && (bot.BattleGround[x][y] == 5 || bot.BattleGround[x][y] == 6))
    {
        printf("You already targeted this location. Choose another location.\n");
        return;
    }

    if (bot.BattleGround[x][y] == 1 || bot.BattleGround[x][y] == 2 || bot.BattleGround[x][y] == 3 || bot.BattleGround[x][y] == 4)
    {
        bot.BattleGround[x][y] = 5; // Hit
        printf("Hit!\n");
    }
    else if (bot.BattleGround[x][y] == 5 || bot.BattleGround[x][y] == 6)
    {
        printf("Already targeted this location.\n");
    }
    else // Hit water
    {
        bot.BattleGround[x][y] = 6; // Miss
        printf("Miss!\n");
    }

    printf("\n");
    bot.number_obscured_positions = 0; // Clear smoke screen effects
}

/*this fillArray method fills in all the array spaces with 0, the reason i didn't
 initialize the first element to 0 and let the rest of the array get filled automatically is
 because while testing, when i initialized the first element only, the last 7 rows got filled with garbage values,
 i believe this is caused by not allocating memory but we haven't covered that yet so i didn't want to confuse any of you guys.*/
void fillArrays()
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            player.BattleGround[i][j] = 0;
            bot.BattleGround[i][j] = 0;
        }
    }
}

int getRandomTurn()
{
    srand(time(NULL));       // this function allows us to set the a different seed for the random number each time our program runs so we don't have the same sequence of random numbers everytime we run our code
    int random = rand() % 2; //%2 gives either 0 or 1 which would guarantee a 50/50 chance for the player and bot
    return random;
}

void getPositions() // method to get positions from user and update the battlefield with the coordinates
{
    int carriersLength[] = {5, 4, 3, 2}; // self explanatory, so we can know the length of each carrier
    int c;
    for (int i = 0; i < 4; i++)
    {
        char orientation; // V/H
        int validInput = 0;
        while (!validInput)
        { // so we can detect any illegal arguments passed by the user
            printf("Enter the position of your %s.\n", arsenal[i]);
            scanf("%4s", player.positions[i]);
            toUpper1(player.positions[i]); // make this easier for us to update the battlefield later
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            clearScreen();
            if (checkInputValidity(player.positions[i]) == 0) // this is to check if the input entered is the correct format or not
            {
                printf("The input you entered is invalid. Please enter a character between A and J and a number between 1 and 10.\n");
                continue;
            }
            printf("what orientation? H/V\n"); // getting the orientation
            scanf("%c", &orientation);
            orientation = toupper(orientation);           // it's one letter thats why i didnt use toupper1 method
            if (orientation != 'H' && orientation != 'V') // validity checks
            {
                printf("wrong input, please try again and enter a valid orientation\n");
                continue;
            }
            if (checkPositionValidity(player.name, player.positions[i], carriersLength[i], player.BattleGround, orientation) == 0)
            { // this is to check if the entered position is taken,goes out of bound...etc (example: J1 and H orientation)
                printf("Please enter a valid position.\n");
                continue;
            }
            validInput = 1;
        }

        updateBattleField(player.BattleGround, player.positions[i], orientation, carriersLength[i], i + 1); // once all checks are done we update
        printf("    %c  %c  %c  %c  %c  %c  %c  %c  %c  %c\n", 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J');
        printf("\n");
        for (int i = 0; i < 10; i++)
        {
            printf("%-4i", i + 1);
            for (int j = 0; j < 10; j++)
            {
                printf("%d  ", player.BattleGround[i][j]); // print the battlefield after each iteration
            } // we didn't use displayBattleField() because we want the player to see his ships
            printf("\n");
        }
    }
}

void initializePlayers()
{
    fillArrays(); // filling the battlegrounds of player and bot with 0s

    int c; // giving names to each player
    printf("Please enter your username: \n");
    fgets(player.name, sizeof(player.name), stdin);
    player.name[strcspn(player.name, "\n")] = '\0'; // Remove newline character if present

    strcpy(bot.name, "Bot");

    clearScreen();
    // At the start of the game, each of the player and bot have 3 radar sweeps, 0 smoke
    // screens, 4 ships etc...
    player.radar_sweeps = 3;
    player.smoke_screens = 0;
    player.numberOfShips = 4;
    player.destroyedAShipInPreviousTurn = 0;
    player.torpedoUnlocked = 0;
    player.torpedoAlreadyUnlocked = 0;
    player.number_obscured_positions = 0;

    bot.radar_sweeps = 3;
    bot.smoke_screens = 0;
    bot.numberOfShips = 4;
    bot.destroyedAShipInPreviousTurn = 0;
    bot.torpedoUnlocked = 0;
    bot.torpedoAlreadyUnlocked = 0;
    bot.number_obscured_positions = 0;

    // still have to get positions of ships of the player and the bot
    printf("Please enter the positions of your ships.\n");
    getPositions();
    clearScreen();

    BotGetPositions();
}

void makeMove()
{
    char ability[10];
    char coordinates[5];
    int x, y;
    bool validMove = false;

    do
    {
        printf("Enter a move followed by the coordinate (e.g., FIRE A5, TORPEDO 1, SMOKE B5): ");
        scanf("%9s %4s", ability, coordinates);

        toUpper1(ability);
        toUpper1(coordinates);

        // Check if it is a valid row
        if (coordinates[0] < 'A' || coordinates[0] > 'J')
        {
            printf("Invalid coordinate letter. Please enter a letter between A and J.\n");
            continue;
        }

        // chekc if it is a valid column
        if (strlen(coordinates) == 2 && (coordinates[1] < '1' || coordinates[1] > '9'))
        {
            printf("Invalid coordinate number. Please enter a number between 1 and 10.\n");
            continue;
        }
        else if (strlen(coordinates) == 3 && !(coordinates[1] == '1' && coordinates[2] == '0'))
        {
            printf("Invalid coordinate number. Please enter a number between 1 and 10.\n");
            continue;
        }

        y = coordinates[0] - 'A';
        x = (strlen(coordinates) == 2) ? coordinates[1] - '1' : 9;

        if (x < 0 || x >= 10 || y < 0 || y >= 10)
        {
            printf("Out of bounds! Coordinates must be within A-J and 1-10.\n");
            continue;
        }

        validMove = true;

    } while (!validMove);

    if (strcmp(ability, "FIRE") == 0)
    {
        Fire(x, y);
    }
    else if (strcmp(ability, "ARTILLERY") == 0)
    {
        Artillery(x, y);
    }
    else if (strcmp(ability, "RADAR") == 0)
    {
        performRadarSweep(x, y);
    }
    else if (strcmp(ability, "SMOKE") == 0)
    {
        smoke_screen(x, y);
        clearScreen();
    }
    else if (strcmp(ability, "TORPEDO") == 0)
    {
        int z;
        bool isRow;
        if (isdigit(coordinates[0]))
        {
            z = atoi(&coordinates[0]) - 1;
            isRow = true;
        }
        else
        {
            z = coordinates[0] - 'A';
            isRow = false;
        }
        Torpedo(z, isRow);
    }
    else
    {
        printf("Unknown ability.\n");
    }
}
void performRadarSweep(int x, int y)
{
    if (player.radar_sweeps == 0)
    {
        printf("No radar sweeps remaining. You lose your turn.\n");
        return;
    }

    bool shipsFound = 0;
    char ij[3]; // to store ij position to compare it to obscured positions
    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            if (bot.BattleGround[i][j] <= 4 && bot.BattleGround[i][j] >= 1) // 1,2,3,4 represent ships
            {
                shipsFound = 1;
                for (int k = 0; k < bot.number_obscured_positions; k++)
                {
                    sprintf(ij, "%d%d", i, j);
                    if (strcmp(bot.obscured_areas[k], ij) == 0)
                    {
                        shipsFound = 0;
                        break;
                    }
                }
            }
            if (shipsFound)
                break;
        }
        if (shipsFound)
            break;
    }

    if (shipsFound)
    {
        printf("Enemy ships found!\n");
    }
    else
    {
        printf("No enemy ships found!\n");
    }

    // Decrement radar sweeps
    (player.radar_sweeps)--;
    bot.number_obscured_positions = 0; // this is because the effect of smoke screen which the bot potentially used expired after a turn

    printf("\n");
}

void smoke_screen(int x, int y)
{
    if (player.smoke_screens == 0)
    {
        printf("You don't have any smoke screens. You lose your turn.\n");
        return;
    }

    player.number_obscured_positions = 0;
    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            sprintf(player.obscured_areas[player.number_obscured_positions], "%d%d", i, j);
            player.number_obscured_positions++;
        }
    }

    player.smoke_screens--;
    bot.number_obscured_positions = 0; // this is because the effect of smoke screen which
                                       // the bot potentially used expired after a turn
}

char setBotDifficulty()
{
    // We are going to use the global variable char botDifficulty

    printf("Select Bot Difficulty Level:\n");
    printf("e. Easy\n");
    printf("m. Medium\n");
    printf("h. Hard\n");
    printf("Enter your choice (e/m/h): ");
    int c;
    char b;

    while (1)
    {
        scanf("%c", &botDifficulty);
        scanf("%c", &b);                        // this is to ensure we cleared any extra input in the buffer
        botDifficulty = tolower(botDifficulty); // Convert to lower-case to handle upper-case input
        if (botDifficulty == 'e' || botDifficulty == 'm' || botDifficulty == 'h')
        {
            break; // Valid input, exit loop
        }
        else
        {
            printf("Invalid choice. Please enter (e) or (m) or (h): ");
        }
    }

    switch (botDifficulty)
    {
    case 'e':
        printf("You have selected Easy Bot Difficulty.\n");
        break;
    case 'm':
        printf("You have selected Medium Bot Difficulty.\n");
        break;
    case 'h':
        printf("You have selected Hard Bot Difficulty.\n");
        break;
    }
    printf("\n");
    return botDifficulty;
}

char setGameDifficulty()
{
    // We are going to use the global variable char gameDifficulty

    printf("Select Game Difficulty Level:\n");
    printf("e. Easy\n");
    printf("h. Hard\n");
    printf("Enter your choice (e/h): ");
    int c;
    char b;

    while (1)
    {
        scanf("%c", &gameDifficulty);
        scanf("%c", &b);                          // this is to ensure we cleared any extra input in the buffer
        gameDifficulty = tolower(gameDifficulty); // Convert to lower-case to handle upper-case input
        if (gameDifficulty == 'e' || gameDifficulty == 'h')
        {
            break; // Valid input, exit loop
        }
        else
        {
            printf("Invalid choice. Please enter (e) or (h): ");
        }
    }

    switch (gameDifficulty)
    {
    case 'e':
        printf("You have selected Easy Game Difficulty.\n");
        break;
    case 'h':
        printf("You have selected Hard Game Difficulty.\n");
        break;
    }
    printf("\n");
    return gameDifficulty;
}

void toUpper1(char input[]) // this is a method to turn string into uppercase since there is no default way to do this as far as i am aware
{
    for (int i = 0; i < input[i] != '\0'; i++)
    {
        input[i] = toupper((unsigned char)input[i]);
    }
}

void Torpedo(int z, bool isRow)
{
    if (!(player.torpedoUnlocked))
    {
        printf("Torpedo is not unlocked.\n");
        return;
    }

    bool hit = false;

    if (isRow)
    {
        for (int i = 0; i < 10; i++)
        {
            if (bot.BattleGround[z][i] >= 1 && bot.BattleGround[z][i] <= 4)
            {
                bot.BattleGround[z][i] = 5;
                hit = true;
            }
            else if (bot.BattleGround[z][i] == 0)
            {
                bot.BattleGround[z][i] = 6;
            }
        }
    }
    else
    {
        for (int i = 0; i < 10; i++)
        {
            if (bot.BattleGround[i][z] >= 1 && bot.BattleGround[i][z] <= 4)
            {
                bot.BattleGround[i][z] = 5;
                hit = true;
            }
            else if (bot.BattleGround[i][z] == 0)
            {
                bot.BattleGround[i][z] = 6;
            }
        }
    }
    if (hit)
    {
        printf("hit\n");
    }
    else
    {
        printf("miss\n");
    }

    player.torpedoUnlocked = 0;
    bot.number_obscured_positions = 0; // this is because the effect of smoke screen which
                                       // the bot potentially used expired after a turn
}

void updateBattleField(char BattleGround[10][10], char position[], char orientation, int lengthOfCarrier, int numberAssociated) // Updating the battlefield after positions are entered
{
    int x = position[0] - 'A';
    int y;
    if (strlen(position) == 2) // to get Y, this is the same tactic used in the get positions method
    {
        y = position[1] - '1';
    }
    else if (strlen(position) == 3)
    {
        y = 9;
    }
    if (orientation == 'V') // very simple updating vertically/horizontally using length of the carriers
    {
        for (int i = y; i < y + lengthOfCarrier; i++)
        {
            BattleGround[i][x] = numberAssociated;
        }
    }
    else if (orientation == 'H')
    {
        for (int i = x; i < x + lengthOfCarrier; i++)
        {
            BattleGround[y][i] = numberAssociated;
        }
    }
}

void getHeatMap(Player *realPlayer)
{
    for (int i = 0; i < 10; i++) // this is to reset the heatmap
    {
        for (int j = 0; j < 10; j++)
        {
            realPlayer->heatmap[i][j] = 0;
        }
    }

    for (int m = 1; m < 5; m++)
    {
        if (realPlayer->DestroyedShips[m] == false)
        {
            for (int i = 0; i < 10; i++)
            {
                for (int j = 0; j < 10; j++)
                {
                    if (checkHeatMapPlacementValidity(i, j, realPlayer->BattleGround, 6 - m, 'H') == true)
                    {
                        UpdateHeapMap(realPlayer->heatmap, i, j, 6 - m, 'H');
                    }
                    if (checkHeatMapPlacementValidity(i, j, realPlayer->BattleGround, 6 - m, 'V') == true)
                    {
                        UpdateHeapMap(realPlayer->heatmap, i, j, 6 - m, 'V');
                    }
                }
            }
        }
    }
}

bool checkHeatMapPlacementValidity(int x, int y, char battlefield[10][10], int lengthOfCarrier, char orientation)
{
    if (orientation == 'V')
    {
        for (int i = x; i < x + lengthOfCarrier; i++)
        {
            if (i >= 10 || battlefield[i][y] == 5 || battlefield[i][y] == 6)
            {
                return false;
            }
        }
    }
    else if (orientation == 'H')
    {
        for (int i = y; i < y + lengthOfCarrier; i++)
        {
            if (i >= 10 || battlefield[x][i] == 5 || battlefield[x][i] == 6)
            {
                return false;
            }
        }
    }
    return true;
}

void UpdateHeapMap(int heatmap[10][10], int x, int y, int lengthOfCarrier, char orientation)
{
    if (orientation == 'V') // very simple updating vertically/horizontally using length of the carriers
    {
        for (int i = x; i < x + lengthOfCarrier; i++)
        {
            heatmap[i][y]++;
        }
    }
    else if (orientation == 'H')
    {
        for (int i = y; i < y + lengthOfCarrier; i++)
        {
            heatmap[x][i]++;
        }
    }
}

bool checkBotHitValidity(int x, int y)
{
    if (x >= 10 || y >= 10 || x < 0 || y < 0)
    {
        return false;
    }
    return true;
}

char getOrientation(int newX, int oldX)
{
    if ((oldX - newX) != 0)
    {
        return 'V';
    }
    else
    {
        return 'H';
    }
}

void MediumBotMakeMove()
{
    getHeatMap(&player);
    Counter++;

    if (bot.torpedoUnlocked) // if unlocked bot should use it and prioritize it over artillery
    {
        printf("Bot chooses to use Torpedo!\n");
        MediumBotTorpedo(player.heatmap);
    }
    else if (bot.destroyedAShipInPreviousTurn == 1) // if artillery is unlocked bot should use it
    {
        printf("Bot chooses to perform Artillery!\n");
        MediumBotArtillery(player.heatmap);
    }
    else if (bot.DetectedTargets == true) // if bot found ships in radar sweep it should shoot there
    {
        // Prioritize targeting detected ships
        int targetX = -1;
        int targetY = -1;
        int targetIndex = 0;
        while (targetX == -1 && targetY == -1)
        {
            targetX = bot.detectedTargets[targetIndex][0];
            targetY = bot.detectedTargets[targetIndex][1];
            targetIndex++;
        }
        bot.detectedTargets[targetIndex][0] = -1; // Remove the detected target position from detected
        bot.detectedTargets[targetIndex][1] = -1; // targets (action is done so no need to keep it stored)
        bot.num_DetectedTargets_ToCheck--;

        printf("Bot fires at detected target %c%d\n", 'A' + targetY, targetX + 1);

        if (player.BattleGround[targetX][targetY] >= 1 && player.BattleGround[targetX][targetY] <= 4)
        {
            player.BattleGround[targetX][targetY] = 5; // Hit
            printf("Hit!\n");
        }
        else
        {
            if (player.BattleGround[targetX][targetY] == 0)
                player.BattleGround[targetX][targetY] = 6;
            if (player.BattleGround[targetX][targetY] == 5)
                player.BattleGround[targetX][targetY] = 5;
            printf("Miss!\n");
        }

        if (bot.num_DetectedTargets_ToCheck == 0)
        {
            bot.DetectedTargets = false; // we finished checking all radar spots with potential ships
        }
    }

    else
    {

        if (inAttackingMode == true)
        {
            MediumBotFire(player.heatmap);
        }
        else // inAttackingMode == false
        {
            if (Counter >= 4 && Counter % 2 == 0 && bot.radar_sweeps != 0)
            {
                MediumBotRadarSweep(player.heatmap);
            }
            else if (Counter >= 4 && Counter % 4 == 0 && bot.smoke_screens != 0)
            {
                MediumBotSmokeScreen(bot.heatmap);
            }
            else
            {
                MediumBotFire(player.heatmap);
            }
        }
    }
}
void MediumBotRadarSweep(int heatmap[10][10])
{
    int max1 = 0;
    int max2 = 0;
    int max3 = 0;
    int x;
    int y;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            int sum = 0;
            for (int m = i; m <= i + 1 && m < 10; m++)
            {
                for (int n = j; n <= j + 1 && n < 10; n++)
                {
                    sum = sum + heatmap[m][n];
                }
            }
            if (sum >= max3)
            {
                if (sum >= max2)
                {
                    if (sum >= max1)
                    {
                        max3 = max2;
                        max2 = max1;
                        max1 = sum;
                    }
                    else
                    {
                        max3 = max2;
                        max2 = sum;
                    }
                }
                else
                {
                    max3 = sum;
                }
            }
        }
    }
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            int sum = 0;
            for (int m = i; m <= i + 1 && m < 10; m++)
            {
                for (int n = j; n <= j + 1 && n < 10; n++)
                {
                    sum = sum + heatmap[m][n];
                    if (sum == max3)
                    {
                        x = i;
                        y = j;
                    }
                }
            }
        }
    }

    printf("Bot uses Radar Sweep at %c%d\n", 'A' + y, x + 1);

    // set previous DetectedTargets boolean to false and the number of detected targets to check back to 0
    bot.num_DetectedTargets_ToCheck = 0;
    bot.DetectedTargets = false;
    char ij[3]; // to store ij position to compare it to obscured positions

    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            if (player.BattleGround[i][j] <= 4 && player.BattleGround[i][j] >= 1) // 1,2,3,4 represent ships
            {
                bot.DetectedTargets = true;
                for (int k = 0; k < player.number_obscured_positions; k++)
                {
                    sprintf(ij, "%d%d", i, j);
                    if (strcmp(player.obscured_areas[k], ij) == 0)
                    {
                        bot.DetectedTargets = false;
                        break;
                    }
                }
            }
            if (bot.DetectedTargets == true)
                break;
        }
        if (bot.DetectedTargets == true)
            break;
    }

    if (bot.DetectedTargets == true)
    {
        for (int i = 0; i < 4; i++)
        {
            if (i == 0)
            {
                // Step 1: Keep x and y the same
            }
            else if (i == 1)
            {
                // Step 2: Increment x
                x++;
            }
            else if (i == 2)
            {
                // Step 3: Increment both x and y
                y++;
            }
            else if (i == 3)
            {
                // Step 4: Increment only y
                x--; // this way we return x to its initial value
            }

            if (x >= 0 && x < 10 && y >= 0 && y < 10)
            {
                bot.detectedTargets[i][0] = x;
                bot.detectedTargets[i][1] = y;
                bot.num_DetectedTargets_ToCheck++;
            }
            else
            {
                // the spot is outside the grid
                bot.detectedTargets[i][0] = -1;
                bot.detectedTargets[i][1] = -1;
            }
        }
        printf("Enemy ships found!\n");
    }

    else
    {
        printf("No enemy ships found!\n");
    }

    bot.radar_sweeps--;                   // Decrement radar sweeps after use
    player.number_obscured_positions = 0; // this is because the effect of smoke screen which the player potentially used expired after a turn
    printf("\n");
}
void MediumBotFire(const int heatmap[10][10])
{
    if (inAttackingMode == false)
    {
        int max = -1;
        int second_max = -1;
        int third_max = -1;
        int x;
        int y;

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                int current = heatmap[i][j];

                if (current > max)
                {
                    // Shift down max hierarchy
                    third_max = second_max;
                    second_max = max;
                    max = current;
                }
                else if (current > second_max && current < max)
                {
                    third_max = second_max;
                    second_max = current;
                }
                else if (current > third_max && current < second_max)
                {
                    third_max = current;
                }
            }
        }

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                if (heatmap[i][j] == third_max)
                {
                    x = i;
                    y = j;
                    break;
                }
            }
        }

        if (player.BattleGround[x][y] == 1 || player.BattleGround[x][y] == 2 || player.BattleGround[x][y] == 3 || player.BattleGround[x][y] == 4)
        {
            player.BattleGround[x][y] = 5;
            inAttackingMode = true;
            lastSpotHitIndex++;
            alreadyHitSpots[lastSpotHitIndex][0] = x;
            alreadyHitSpots[lastSpotHitIndex][1] = y;
            adjacentHitSpots[numAdjacentHitSpots][0] = x; // saving potential adjacent ship spots
            adjacentHitSpots[numAdjacentHitSpots][1] = y;
            numAdjacentHitSpots++;
            printf("Hit!\n");
        }
        else
        {
            player.BattleGround[x][y] = 6;
            inAttackingMode = false;
            printf("Miss!\n");
        }
        bot.number_obscured_positions = 0; // this is because the effect of smoke screen which
                                           // the player potentially used expired after a turn
    }

    else //(in attacking mode)
    {
        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; // up, down, left, right
        int lastHitX = alreadyHitSpots[lastSpotHitIndex][0];
        int lastHitY = alreadyHitSpots[lastSpotHitIndex][1];
        int newTargetX = -1;
        int newTargetY = -1;

        newTargetX = lastHitX + directions[checkedDirection][0];
        newTargetY = lastHitY + directions[checkedDirection][1];

        if (correctDirection2 == true && correctDirection == true)
        {
            if (checkBotHitValidity(newTargetX, newTargetY) == false || player.BattleGround[newTargetX][newTargetY] == 5 || player.BattleGround[newTargetX][newTargetY] == 6)
            {
                // means we targeted in one direction and couldn't target in the other direction and
                // we still didn't destroy a ship which means we targeted parts of different ships
                are_adjacent_ships = true;
            }
        }

        if (are_adjacent_ships == true && HittingAdjacent == false)
        {
            // we need to hit the adjacent ship
            lastSpotHitIndex = 0;
            alreadyHitSpots[lastSpotHitIndex][0] = adjacentHitSpots[numAdjacentHitSpots - 1][0];
            alreadyHitSpots[lastSpotHitIndex][1] = adjacentHitSpots[numAdjacentHitSpots - 1][1];

            lastHitX = alreadyHitSpots[lastSpotHitIndex][0];
            lastHitY = alreadyHitSpots[lastSpotHitIndex][1];

            newTargetX = lastHitX + directions[checkedDirection][0];
            newTargetY = lastHitY + directions[checkedDirection][1];

            HittingAdjacent = true;
        }

        orientation = getOrientation(newTargetX, lastHitX);
        if (checkBotHitValidity(newTargetX, newTargetY) == false || player.BattleGround[newTargetX][newTargetY] == 5 || player.BattleGround[newTargetX][newTargetY] == 6)
        {
            // the only time checkBotHitValidity() is false is when we are in attakind mode and try to hit
            // a spot outside the grid which means we need to go opposite direction to stay on the grid
            // and continue hitting the ship
            lastSpotHitIndex = 0; // we return to the first spot we had hit on the ship
            lastHitX = alreadyHitSpots[lastSpotHitIndex][0];
            lastHitY = alreadyHitSpots[lastSpotHitIndex][1];

            if (orientation == 'V')
            {
                // For vertical, 0 = "up" and 1 = "down"
                checkedDirection = (checkedDirection == 0) ? 1 : 0; // Flip between "up" and "down"
                newTargetX = lastHitX + directions[checkedDirection][0];
                newTargetY = lastHitY + directions[checkedDirection][1];

                if (player.BattleGround[newTargetX][newTargetY] == 5 || player.BattleGround[newTargetX][newTargetY] == 6 || checkBotHitValidity(newTargetX, newTargetY) == false)
                {
                    // means we already shot there and should change direction or the spot is outside the grid
                    checkedDirection = 2; // left
                    newTargetX = lastHitX + directions[checkedDirection][0];
                    newTargetY = lastHitY + directions[checkedDirection][1];
                }
                if (player.BattleGround[newTargetX][newTargetY] == 5 || player.BattleGround[newTargetX][newTargetY] == 6 || checkBotHitValidity(newTargetX, newTargetY) == false)
                {
                    // means we already shot there and should change direction or the spot is outside the grid
                    checkedDirection = 3; // right
                    newTargetX = lastHitX + directions[checkedDirection][0];
                    newTargetY = lastHitY + directions[checkedDirection][1];
                }
            }
            else if (orientation == 'H')
            {
                // For horizontal, 2 = "left" and 3 = "right"
                checkedDirection = (checkedDirection == 2) ? 3 : 2; // Flip between "left" and "right"
                newTargetX = lastHitX + directions[checkedDirection][0];
                newTargetY = lastHitY + directions[checkedDirection][1];

                if (player.BattleGround[newTargetX][newTargetY] == 5 || player.BattleGround[newTargetX][newTargetY] == 6 || checkBotHitValidity(newTargetX, newTargetY) == false)
                {
                    // means we already shot there and should change direction or the spot is outside the grid
                    checkedDirection = 0; // up
                    newTargetX = lastHitX + directions[checkedDirection][0];
                    newTargetY = lastHitY + directions[checkedDirection][1];
                }
                if (player.BattleGround[newTargetX][newTargetY] == 5 || player.BattleGround[newTargetX][newTargetY] == 6 || checkBotHitValidity(newTargetX, newTargetY) == false)
                {
                    // means we already shot there and should change direction or the spot is outside the grid
                    checkedDirection = 1; // down
                    newTargetX = lastHitX + directions[checkedDirection][0];
                    newTargetY = lastHitY + directions[checkedDirection][1];
                }
            }
        }

        printf("shooting at %d%d", newTargetX, newTargetY);

        if (player.BattleGround[newTargetX][newTargetY] >= 1 && player.BattleGround[newTargetX][newTargetY] <= 4) // hit
        {
            player.BattleGround[newTargetX][newTargetY] = 5;
            lastSpotHitIndex++;
            alreadyHitSpots[lastSpotHitIndex][0] = newTargetX;
            alreadyHitSpots[lastSpotHitIndex][1] = newTargetY;
            adjacentHitSpots[numAdjacentHitSpots][0] = newTargetX; // saving potential adjacent ship spots
            adjacentHitSpots[numAdjacentHitSpots][1] = newTargetY;
            if (!are_adjacent_ships) // means we didn't get our adjacent ships yet
            {
                numAdjacentHitSpots++;
            }
            correctDirection = true;
            NumSpotsHitAttackingMode++;
            printf("Hit!\n\n");
        }
        else // miss
        {
            if (correctDirection2 == true && correctDirection == true)
            {
                are_adjacent_ships = true;
                // means we targeted in both directions but didnt destroy a ship which means we
                // targeted parts of different ships
                correctDirection2 = false;
                correctDirection = false;
            }
            else if (correctDirection == true) // means the direction was right but we could have started hitting the ship in the middle which is why we are missing now so we have to go the opposite direction now
            {
                correctDirection2 = true; // we are targeting in the opposite direction of the first direction
                lastSpotHitIndex = 0;     // we return to the first spot we had hit on the ship
                if (orientation == 'V')
                {
                    // For vertical, 0 = "up" and 1 = "down"
                    checkedDirection = (checkedDirection == 0) ? 1 : 0; // Flip between "up" and "down"
                }
                else if (orientation == 'H')
                {
                    // For horizontal, 2 = "left" and 3 = "right"
                    checkedDirection = (checkedDirection == 2) ? 3 : 2; // Flip between "left" and "right"
                }
            }
            else
            {
                checkedDirection++; // we try new direction
                correctDirection = false;
            }
            player.BattleGround[newTargetX][newTargetY] = 6;
            printf("Miss!\n\n");
        }
    }
    player.number_obscured_positions = 0; // this is because the effect of smoke screen which the player potentially used expired after a turn
}

void MediumBotSmokeScreen(int heatmap[10][10])
{
    int max1 = 0;
    int max2 = 0;
    int max3 = 0;
    int x;
    int y;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            int sum = 0;
            for (int m = i; m <= i + 1 && m < 10; m++)
            {
                for (int n = j; n <= j + 1 && n < 10; n++)
                {
                    sum = sum + heatmap[m][n];
                }
            }
            if (sum >= max3)
            {
                if (sum >= max2)
                {
                    if (sum >= max1)
                    {
                        max3 = max2;
                        max2 = max1;
                        max1 = sum;
                    }
                    else
                    {
                        max3 = max2;
                        max2 = sum;
                    }
                }
                else
                {
                    max3 = sum;
                }
            }
        }
    }
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            int sum = 0;
            for (int m = i; m <= i + 1 && m < 10; m++)
            {
                for (int n = j; n <= j + 1 && n < 10; n++)
                {
                    sum = sum + heatmap[m][n];
                    if (sum == max3)
                    {
                        x = i;
                        y = j;
                    }
                }
            }
        }
    }
    bot.number_obscured_positions = 0;
    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            sprintf(bot.obscured_areas[bot.number_obscured_positions], "%d%d", i, j);
            bot.number_obscured_positions++;
        }
    }

    bot.smoke_screens--;
    player.number_obscured_positions = 0; // this is because the effect of smoke screen which the player potentially used expired after a turn
}
void MediumBotArtillery(int heatmap[10][10])
{
    int max1 = 0;
    int max2 = 0;
    int max3 = 0;
    int x;
    int y;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            int sum = 0;
            for (int m = i; m <= i + 1 && m < 10; m++)
            {
                for (int n = j; n <= j + 1 && n < 10; n++)
                {
                    sum = sum + heatmap[m][n];
                }
            }
            if (sum >= max3)
            {
                if (sum >= max2)
                {
                    if (sum >= max1)
                    {
                        max3 = max2;
                        max2 = max1;
                        max1 = sum;
                    }
                    else
                    {
                        max3 = max2;
                        max2 = sum;
                    }
                }
                else
                {
                    max3 = sum;
                }
            }
        }
    }
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            int sum = 0;
            for (int m = i; m <= i + 1 && m < 10; m++)
            {
                for (int n = j; n <= j + 1 && n < 10; n++)
                {
                    sum = sum + heatmap[m][n];
                    if (sum == max3)
                    {
                        x = i;
                        y = j;
                    }
                }
            }
        }
    }
    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            if (player.BattleGround[i][j] != 0 && player.BattleGround[i][j] != 5 && player.BattleGround[i][j] != 6)
            {
                player.BattleGround[i][j] = 5;
                lastSpotHitIndex++;
                alreadyHitSpots[lastSpotHitIndex][0] = i;
                alreadyHitSpots[lastSpotHitIndex][1] = j;
                inAttackingMode = true;
            }
            else if (player.BattleGround[i][j] == 0)
            {
                player.BattleGround[i][j] = 6;
            }
        }
    }
    bot.destroyedAShipInPreviousTurn = 0;
    player.number_obscured_positions = 0; // this is because the effect of smoke screen which the player potentially used expired after a turn
}
void MediumBotTorpedo(int heatmap[10][10])
{
    int maxRowSum1 = 0, maxRowSum2 = 0, maxRowSum3 = 0, maxRowIdx = -1;
    int maxColSum1 = 0, maxColSum2 = 0, maxColSum3 = 0, maxColIdx = -1;
    bool hit = false;

    // Calculate row sums
    for (int i = 0; i < 10; i++)
    {
        int rowSum = 0;
        for (int j = 0; j < 10; j++)
        {
            rowSum += heatmap[i][j];
        }
        if (rowSum >= maxRowSum3)
        {
            if (rowSum >= maxRowSum2)
            {
                if (rowSum >= maxRowSum1)
                {
                    maxRowSum3 = maxRowSum2;
                    maxRowSum2 = maxRowSum1;
                    maxRowSum1 = rowSum;
                }
                else
                {
                    maxRowSum3 = maxRowSum2;
                    maxRowSum2 = rowSum;
                }
            }
            else
            {
                maxRowSum3 = rowSum;
            }
        }
    }

    // Calculate column sums
    for (int j = 0; j < 10; j++)
    {
        int colSum = 0;
        for (int i = 0; i < 10; i++)
        {
            colSum += heatmap[i][j];
        }
        if (colSum >= maxColSum3)
        {
            if (colSum >= maxColSum2)
            {
                if (colSum >= maxColSum1)
                {
                    maxColSum3 = maxColSum2;
                    maxColSum2 = maxColSum1;
                    maxColSum1 = colSum;
                }
                else
                {
                    maxColSum3 = maxColSum2;
                    maxColSum2 = colSum;
                }
            }
            else
            {
                maxColSum3 = colSum;
            }
        }
    }
    for (int i = 0; i < 10; i++)
    {
        int rowSum = 0;
        for (int j = 0; j < 10; j++)
        {
            rowSum += heatmap[i][j];
        }
        if (rowSum == maxRowSum3)
        {
            maxRowIdx = i;
        }
    }
    for (int j = 0; j < 10; j++)
    {
        int colSum = 0;
        for (int i = 0; i < 10; i++)
        {
            colSum += heatmap[i][j];
        }
        if (colSum == maxColSum3)
        {
            maxRowIdx = j;
        }
    }
    if (maxRowSum3 >= maxColSum3)
    {
        for (int i = 0; i < 10; i++)
        {
            if (player.BattleGround[maxRowIdx][i] >= 1 && player.BattleGround[maxRowIdx][i] <= 4)
            {
                player.BattleGround[maxRowIdx][i] = 5; // 5 means hit
                hit = true;
            }
            else if (player.BattleGround[maxRowIdx][i] == 0)
            {
                player.BattleGround[maxRowIdx][i] = 6; // 6 means miss
            }
        }
    }

    else
    {
        for (int i = 0; i < 10; i++)
        {
            if (player.BattleGround[i][maxColIdx] >= 1 && player.BattleGround[i][maxColIdx] <= 4)
            {
                player.BattleGround[i][maxColIdx] = 5; // 5 means hit
                hit = true;
            }
            else if (player.BattleGround[i][maxColIdx] == 0)
            {
                player.BattleGround[i][maxColIdx] = 6; // 6 means miss
            }
        }
    }

    if (hit)
    {
        printf("Hit!\n");
    }
    else
    {
        printf("Miss!\n");
    }

    bot.torpedoUnlocked = false;
    player.number_obscured_positions = 0; // this is because the effect of smoke screen which the player potentially used expired after a turn
}
