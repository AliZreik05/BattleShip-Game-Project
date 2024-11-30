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
    int numberOfShips;
    bool DestroyedShips[5];
    int destroyedAShipInPreviousTurn;
    int radar_sweeps;
    int smoke_screens;
    char obscured_areas[4][3];
    int number_obscured_positions;
    bool torpedoUnlocked;
    bool torpedoAlreadyUnlocked;
    int detectedTargets[10][2];  
    int numDetectedTargets;

    // int score; we can implement this one later when the game can be played more than once
};

// global variables so we can access them anywhere in the code
Player player;
Player bot;
char gameDifficulty;
char botDifficulty;
const char *arsenal[] = {"carrier", "battleship", "destroyer", "submarine"};

// methods
void Artillery(int x, int y);
void BotGetPositions();
void BotmakeMove();
void BotTorpedo();
void BotRadarSweep();
void BotSmokeScreen(int x, int y);
void BotFire();
bool checkInputValidity(char input[]);
bool checkPositionValidity(char name[], char input[], int lengthOfCaerrier, char BattleGround[10][10], char orientation);
void checkForDestroyedShips(Player *p);
void clearScreen();
void displayBattleField(Player *p, char difficulty);
void displayAvailableMoves(Player *p);
void Fire(int x, int y);
void fillArrays();
int getRandomTurn();
void getPositions();
void initializePlayers();
void makeMove();
void performRadarSweep(int x, int y);
void smoke_screen(int x, int y);
char setGameDifficulty();
char setBotDifficulty();
void toUpper1(char input[]); // toUpper1 makes a char[] to uppercase (a string)
void Torpedo(int z, bool isRow);
void updateBattleField(char battlefield[10][10], char position[], char orientation, int lengthOfCarrier, int numberAssociated);

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
            BotmakeMove();
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
    printf("You hit %d unharmed ships, %d already harmed ships, and missed on %d shots.\n\n", hit, alreadyHit, miss);
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

void BotmakeMove() {
    srand(time(NULL));

    if (bot.numDetectedTargets > 0) {
        // Prioritize targeting detected ships, so that if the bot uses radar sweep it won't continue hitting randomly (which would defeat the point of using radar sweep)
        int targetIndex = rand() % bot.numDetectedTargets;
        int targetX = bot.detectedTargets[targetIndex][0];
        int targetY = bot.detectedTargets[targetIndex][1];

        printf("Bot fires at detected target %c%d\n", 'A' + targetY, targetX + 1);

        if (player.BattleGround[targetX][targetY] >= 1 && player.BattleGround[targetX][targetY] <= 4) {
            player.BattleGround[targetX][targetY] = 5;  // Hit
            printf("Hit!\n");
        } else {
            player.BattleGround[targetX][targetY] = 6;  // Miss
            printf("Miss!\n");
        }

        // Remove the targeted position from detected targets (action is done so no need to keep it stored)
        for (int i = targetIndex; i < bot.numDetectedTargets - 1; i++) {
            bot.detectedTargets[i][0] = bot.detectedTargets[i + 1][0];
            bot.detectedTargets[i][1] = bot.detectedTargets[i + 1][1];
        }
        bot.numDetectedTargets--;

    } else {
        // Normal random move if no detected targets (if no sweeps were used or if they were used but no find)
        int randomChoice = rand() % 100;

        if (randomChoice < 50) {
            printf("Bot chooses to Fire randomly!\n");
            BotFire();
        } else if (randomChoice >= 50 && randomChoice < 70) {
            if (bot.torpedoUnlocked) {
                printf("Bot chooses to use Torpedo!\n");
                int z = rand() % 10;
                bool isRow = rand() % 2;
                BotTorpedo(z, isRow);
            } else {
                printf("Bot attempts to use Torpedo, but it is not unlocked. Bot loses its turn.\n");
            }
        } else if (randomChoice >= 70 && randomChoice < 85) {
            if (bot.smoke_screens > 0) {
                printf("Bot chooses to use Smoke Screen!\n");
                int x = rand() % 9;
                int y = rand() % 9;
                BotSmokeScreen(x, y);
            } else {
                printf("Bot attempts to use Smoke Screen, but has none left. Bot loses its turn.\n");
            }
        } else {
            if (bot.radar_sweeps > 0) {
                printf("Bot chooses to perform a Radar Sweep!\n");
                int x = rand() % 9;
                int y = rand() % 9;
                BotRadarSweep(x, y);
            } else {
                printf("Bot attempts to use Radar Sweep, but has none left. Bot loses its turn.\n");
            }
        }
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

void BotFire()
{
    int x, y;
    bool validMove = false;

    while (!validMove)
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
        else
        {
            validMove = true; // in hard or meduim mode they can target the same locaiotn twice
        }
    }

    if (player.BattleGround[x][y] >= 1 && player.BattleGround[x][y] <= 4)
    {
        player.BattleGround[x][y] = 5; // Hit
        printf("Bot fires at %c%d and hits!\n", 'A' + y, x + 1);
    }
    else
    {
        player.BattleGround[x][y] = 6; // Miss
        printf("Bot fires at %c%d and misses!\n", 'A' + y, x + 1);
    }

    player.number_obscured_positions = 0; // Clear smoke screen effects
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
void BotRadarSweep(int x, int y) {
    if (bot.radar_sweeps == 0) {
        printf("Bot has no radar sweeps left.\n");
        return;
    }

    printf("Bot uses Radar Sweep at %c%d\n", 'A' + x, y + 1);

    // Clear previous detected targets
    bot.numDetectedTargets = 0;

    for (int i = x - 1; i <= x + 1; i++) {
        for (int j = y - 1; j <= y + 1; j++) {
            if (i >= 0 && i < 10 && j >= 0 && j < 10) { // Boundary checking
                if (player.BattleGround[i][j] >= 1 && player.BattleGround[i][j] <= 4) { // Ship detected
                    printf("Radar detected a ship part at %c%d\n", 'A' + i, j + 1);

                    // Store detected target coordinates
                    bot.detectedTargets[bot.numDetectedTargets][0] = i;
                    bot.detectedTargets[bot.numDetectedTargets][1] = j;
                    bot.numDetectedTargets++;
                }
            }
        }
    }

    bot.radar_sweeps--;  // Decrement radar sweeps after use
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
void BotSmokeScreen(int x, int y) {
    if (bot.smoke_screens == 0) {
        printf("Bot has no smoke screens left. Skipping action.\n");
        return;
    }

    bot.number_obscured_positions = 0;

    for (int i = x; i <= x + 1 && i < 10; i++) {
        for (int j = y; j <= y + 1 && j < 10; j++) {
            sprintf(bot.obscured_areas[bot.number_obscured_positions], "%d%d", i, j);
            bot.number_obscured_positions++;
        }
    }

    bot.smoke_screens--;
    printf("Bot used a smoke screen at area starting (%c%d).\n", 'A' + x, y + 1);
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
void BotTorpedo() {
    if (!bot.torpedoUnlocked) {
        printf("Bot's torpedo is not unlocked yet.\n");
        return;
    }

    srand(time(NULL));
    bool isRow = rand() % 2; // Randomly choose row or column
    int target = rand() % 10;

    printf("Bot fires a torpedo at %s %d\n", isRow ? "row" : "column", target + 1);

    if (isRow) {
        for (int col = 0; col < 10; col++) {
            if (player.BattleGround[target][col] >= 1 && player.BattleGround[target][col] <= 4) {
                player.BattleGround[target][col] = 5; // Mark as hit
            } else if (player.BattleGround[target][col] == 0) {
                player.BattleGround[target][col] = 6; // Mark as miss
            }
        }
    } else {
        for (int row = 0; row < 10; row++) {
            if (player.BattleGround[row][target] >= 1 && player.BattleGround[row][target] <= 4) {
                player.BattleGround[row][target] = 5; // Mark as hit
            } else if (player.BattleGround[row][target] == 0) {
                player.BattleGround[row][target] = 6; // Mark as miss
            }
        }
    }

    bot.torpedoUnlocked = false; // Reset the torpedo ability
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
