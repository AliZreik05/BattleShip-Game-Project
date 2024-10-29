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
    // int score; we can implement this one later when the game can be played more than once
};

// global variables so we can access them anywhere in the code
Player player1;
Player player2;
char gameDifficulty;
const char *arsenal[] = {"carrier", "battleship", "destroyer", "submarine"};

// methods
void Artillery(Player *p, int x, int y);
bool checkInputValidity(char input[]);
bool checkPositionValidity(char input[], int lengthOfCaerrier, char BattleGround[10][10], char orientation);
void checkForDestroyedShips(Player *p);
void clearScreen();
void displayBattleField(Player *p, char difficulty);
void displayAvailableMoves(Player *p);
void Fire(Player *p, int x, int y);
void fillArrays();
int getRandomTurn();
void getPositions(Player *p);
void initializePlayers();
void makeMove(Player *p);
void performRadarSweep(Player *p, int x, int y);
void smoke_screen(Player *p, int x, int y);
char setGameDifficulty();
void toUpper1(char input[]); // toUpper1 makes a char[] to uppercase (a string)
void Torpedo(Player *p, int z, bool isRow);
void updateBattleField(char battlefield[10][10], char position[], char orientation, int lengthOfCarrier, int numberAssociated);

int main()
{
    gameDifficulty = setGameDifficulty(); // now we have a difficulty set and a message will be printed to the user
    initializePlayers();                  // player1, player2 now have a name, battleground...

    int c;
    int currentPlayer = getRandomTurn();

    do
    {

        // clearScreen();
        if (currentPlayer == 1)
        {
            printf("Player 1's (%s's) turn:\n", player1.name);
            displayBattleField(&player2, gameDifficulty);
            displayAvailableMoves(&player1);
            makeMove(&player2);
            checkForDestroyedShips(&player2);
            currentPlayer = 2;
        }
        else
        {
            printf("Player 2's (%s's) turn:\n", player2.name);
            displayBattleField(&player1, gameDifficulty);
            displayAvailableMoves(&player2);
            makeMove(&player1);
            checkForDestroyedShips(&player1);
            currentPlayer = 1;
        }
    } while (player1.numberOfShips != 0 && player2.numberOfShips != 0);

    printf("Game Over!\n");
    if (player1.numberOfShips == 0)
    {
        printf("Player 2 (%s) is the winner!\n", player2.name);
    }
    else
    {
        printf("Player 1 (%s) is the winner!\n", player1.name);
    }

    return 0;
}

// Functions:
void Artillery(Player *p, int x, int y)
{
    // this artillery method allows the user to unlock it multiple times throughout the game
    // however if we want to allow to unlock it only once throughout the game then we
    // can create a boolean artilleryAlreadyUnloked if its 1 then we wont allow him to do
    // any artilleries anymore

    Player *PlayerAttacking = (strcmp(p->name, player1.name) == 0) ? &player2 : &player1;
    // (the player passed in Artillery is the one being attacked)

    if (PlayerAttacking->destroyedAShipInPreviousTurn == 0)
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
            if (p->BattleGround[i][j] != 0 && p->BattleGround[i][j] != 5 && p->BattleGround[i][j] != 6)
            {
                p->BattleGround[i][j] = 5;
                hit++;
            }
            else if (p->BattleGround[i][j] == 0)
            {
                p->BattleGround[i][j] = 6;
                miss++;
            }
            else if (p->BattleGround[i][j] == 5 || p->BattleGround[i][j] == 6)
            {
                alreadyHit++;
            }
        }
    }
    printf("You hit %d unharmed ships, %d already harmed ships, and missed on %d shots.\n\n", hit, alreadyHit, miss);
    PlayerAttacking->destroyedAShipInPreviousTurn = 0;
    p->number_obscured_positions = 0; // this is because the effect of smoke screen expired after a turn
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

bool checkPositionValidity(char input[], int lengthOfCarrier, char BattleGround[10][10], char orientation) // after checking the input, we check if the position inputted is correct or not
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
                printf("Ship goes out of bounds vertically.\n"); // if the ship goes out of bounds even though its first position is in bounds
                return 0;
            }
            if (BattleGround[i][x] != 0) // if it wasnt 0, then other ship exists there and the position is not available
            {
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
                printf("Ship goes out of bounds horizontally.\n"); // if the ship goes out of bounds even though its first position is in bounds
                return 0;
            }
            if (BattleGround[y][i] != 0) // if it wasnt 0, then other ship exists there and the position is not available
            {
                return 0;
            }
        }
    }
    return 1;
}

void checkForDestroyedShips(Player *p)
{
    Player *PlayerAttacking = (strcmp(p->name, player1.name) == 0) ? &player2 : &player1;
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
        printf("You destroyed a %s!\n\n",arsenal[newDestroyedShip-1]);//print which ship just got destroyed
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

void Fire(Player *p, int x, int y)
{
    // we passed the player being attacked to the method so that we can edit his BattleGround

    if (p->BattleGround[x][y] == 1 || p->BattleGround[x][y] == 2 || p->BattleGround[x][y] == 3 || p->BattleGround[x][y] == 4)
    {
        p->BattleGround[x][y] = 5;
        printf("Hit!\n");
    }
    else if (p->BattleGround[x][y] == 5 || p->BattleGround[x][y] == 6)
    {
        printf("Already targeted this location.\n");
    }
    else // hit water
    {
        printf("Miss!\n");
        p->BattleGround[x][y] = 6;
    }
    printf("\n");
    p->number_obscured_positions = 0; // this is because the effect of smoke screen expired after a turn
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
            player1.BattleGround[i][j] = 0;
            player2.BattleGround[i][j] = 0;
        }
    }
}

int getRandomTurn()
{
    srand(time(NULL));       // this function allows us to set the a different seed for the random number each time our program runs so we don't have the same sequence of random numbers everytime we run our code
    int random = rand() % 2; //%2 gives either 0 or 1 which would guarantee a 50/50 chance for each player
    return random;
}

void getPositions(Player *p) // method to get positions from user and update the battlefield with the coordinates
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
            scanf("%4s", p->positions[i]);
            toUpper1(p->positions[i]); // make this easier for us to update the battlefield later
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            clearScreen();
            if (checkInputValidity(p->positions[i]) == 0) // this is to check if the input entered is the correct format or not
            {
                printf("The input you entered is invalid. Please enter a character between A and J and a number between 1 and 10.\n");
                continue;
            }
            printf("what orientation? H/V\n"); // getting the orientation
            scanf("%c", &orientation);
            orientation = toupper(orientation);           // it's one letter thats why i didnt use toupperposition
            if (orientation != 'H' && orientation != 'V') // validity checks
            {
                printf("wrong input, please try again and enter a valid input\n");
                continue;
            }
            if (checkPositionValidity(p->positions[i], carriersLength[i], p->BattleGround, orientation) == 0)
            { // this is to check if the entered position is taken,goes out of bound...etc (example: J1 and H orientation)
                printf("Another carrier exists in these coordinates, or the position is invalid. Please enter valid ones.\n");
                continue;
            }
            validInput = 1;
        }

        updateBattleField(p->BattleGround, p->positions[i], orientation, carriersLength[i], i + 1); // once all checks are done we update
        printf("    %c  %c  %c  %c  %c  %c  %c  %c  %c  %c\n", 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J');
        printf("\n");
        for (int i = 0; i < 10; i++)
        {
            printf("%-4i", i + 1);
            for (int j = 0; j < 10; j++)
            {
                printf("%d  ", p->BattleGround[i][j]); // print the battlefield after each iteration
            } // we didn't use displayBattleField() because we want the player to see his ships
            printf("\n");
        }
    }
}

void initializePlayers()
{
    fillArrays(); // filling both players battlegrounds with 0s

    int c; // giving names to each player
    printf("Player 1 please enter your username: \n");
    fgets(player1.name, sizeof(player1.name), stdin);
    player1.name[strcspn(player1.name, "\n")] = '\0'; // Remove newline character if present

    printf("Player 2 please enter your username: \n");
    fgets(player2.name, sizeof(player2.name), stdin);
    player2.name[strcspn(player2.name, "\n")] = '\0'; // Remove newline character if present

    while (strcmp(player1.name, player2.name) == 0)
    {
        printf("Name is already used by player1\n");
        printf("Enter another name: \n");
        scanf("%s", player2.name);
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
    clearScreen();
    // At the start of the game each player has 3 radar sweeps, 0 smoke screens,
    // 4 ships and he hasn't hit a ship yet in previous turn (obviously)
    player1.radar_sweeps = 3;
    player1.smoke_screens = 0;
    player1.numberOfShips = 4;
    player1.destroyedAShipInPreviousTurn = 0;
    player1.torpedoUnlocked = 0;
    player1.torpedoAlreadyUnlocked = 0;
    player1.number_obscured_positions = 0;

    player2.radar_sweeps = 3;
    player2.smoke_screens = 0;
    player2.numberOfShips = 4;
    player2.destroyedAShipInPreviousTurn = 0;
    player2.torpedoUnlocked = 0;
    player2.torpedoAlreadyUnlocked = 0;
    player2.number_obscured_positions = 0;

    // still have to get positions of ships of players
    printf("Player 1 (%s) please enter the positions of your ships\n", player1.name);
    getPositions(&player1);
    clearScreen();
    printf("Player 2 (%s) please enter the positions of your ships\n", player2.name);
    getPositions(&player2);
    clearScreen();
}

void makeMove(Player *p)
{
    char ability[10];
    char coordinates[5];
    printf("Enter a move followed by the coordinate (e.g., FIRE A5, TORPEDO 1, SMOKE B5): ");
    scanf("%9s %4s", ability, coordinates);

    toUpper1(ability);
    toUpper1(coordinates);

    if (strcmp(ability, "FIRE") == 0 || strcmp(ability, "ARTILLERY") == 0 || strcmp(ability, "RADAR") == 0 || strcmp(ability, "SMOKE") == 0)
    {
        if (coordinates[0] < 'A' || coordinates[0] > 'J')
        {
            printf("Invalid coordinate letter.\n");
            return;
        }

        int y = coordinates[0] - 'A';
        int x = atoi(&coordinates[1]) - 1;

        if (y < 0 || y >= 10)
        {
            printf("Invalid coordinate number.\n");
            return;
        }

        if (strcmp(ability, "FIRE") == 0)
        {
            Fire(p, x, y);
        }
        else if (strcmp(ability, "ARTILLERY") == 0)
        {
            Artillery(p, x, y);
        }
        else if (strcmp(ability, "RADAR") == 0)
        {
            performRadarSweep(p, x, y);
        }
        else if (strcmp(ability, "SMOKE") == 0)
        {
            Player *PlayerPlaying = (strcmp(p->name, player1.name) == 0) ? &player2 : &player1;
            smoke_screen(PlayerPlaying, x, y); // because we need to modify the player playing's grid to hide his ships
            clearScreen();
        }
    }

    else if (strcmp(ability, "TORPEDO") == 0)
    {

        int z;      // need z and isRow for torpedo method as it only needs
        bool isRow; // one coordinate (either row or column coordinate)
        if (isdigit(coordinates[0]))
        {
            z = atoi(&coordinates[0]) - 1;
            isRow = 1;
        }
        else
        {
            z = coordinates[0] - 'A';
            isRow = 0;
        }
        Torpedo(p, z, isRow);
    }

    else
    {
        printf("Unknown ability.\n");
    }
}

void performRadarSweep(Player *p, int x, int y)
{
    Player *PlayerAttacking = (strcmp(p->name, player1.name) == 0) ? &player2 : &player1;
    // this is important as we need to check the number of radar sweeps remaining
    // for the attacking player (the player passed in performRadarSweep is the one being attacked)

    if (PlayerAttacking->radar_sweeps == 0)
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
            if (p->BattleGround[i][j] <= 4 && p->BattleGround[i][j] >= 1) // 1,2,3,4 represent ships
            {
                shipsFound = 1;
                for (int k = 0; k < p->number_obscured_positions; k++)
                {
                    sprintf(ij, "%d%d", i, j);
                    if (strcmp(p->obscured_areas[k], ij) == 0)
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
    (PlayerAttacking->radar_sweeps)--;
    p->number_obscured_positions = 0; // this is because the effect of smoke screen expired after a turn

    printf("\n");
}

void smoke_screen(Player *p, int x, int y)
{
    if (p->smoke_screens == 0)
    {
        printf("You don't have any smoke screens. You lose your turn.\n");
        return;
    }

    p->number_obscured_positions = 0;
    for (int i = x; i <= x + 1 && i < 10; i++)
    {
        for (int j = y; j <= y + 1 && j < 10; j++)
        {
            sprintf(p->obscured_areas[p->number_obscured_positions], "%d%d", i, j);
            p->number_obscured_positions++;
        }
    }

    p->smoke_screens--;
}

char setGameDifficulty()
{
    // We are going to use the global variable char gameDifficulty

    printf("Select Difficulty Level:\n");
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
        printf("You have selected Easy difficulty.\n");
        break;
    case 'h':
        printf("You have selected Hard difficulty.\n");
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

void Torpedo(Player *p, int z, bool isRow)
{
    Player *PlayerAttacking = (strcmp(p->name, player1.name) == 0) ? &player2 : &player1;
    // (the player passed in Torpedo is the one being attacked)

    if (!(PlayerAttacking->torpedoUnlocked))
    {
        printf("Torpedo is not unlocked.\n");
        return;
    }

    bool hit = false;

    if (isRow)
    {
        for (int i = 0; i < 10; i++)
        {
            if (p->BattleGround[z][i] >= 1 && p->BattleGround[z][i] <= 4)
            {
                p->BattleGround[z][i] = 5;
                hit = true;
            }
            else if (p->BattleGround[z][i] == 0)
            {
                p->BattleGround[z][i] = 6;
            }
        }
    }
    else
    {
        for (int i = 0; i < 10; i++)
        {
            if (p->BattleGround[i][z] >= 1 && p->BattleGround[i][z] <= 4)
            {
                p->BattleGround[i][z] = 5;
                hit = true;
            }
            else if (p->BattleGround[i][z] == 0)
            {
                p->BattleGround[i][z] = 6;
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

    PlayerAttacking->torpedoUnlocked = 0;
    p->number_obscured_positions = 0; // this is because the effect of smoke screen expired after a turn
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
