#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>                 //this is needed for the random function so we can guarantee that the whoever gets first serve is random
#include <time.h>                   /*this is also needed to set a random seed for the random function, 
                                     this is so we dont get the same sequence of random numbers each time*/
void displayBattleField(int battlefield[10][10], char difficulty);
void fillAray(int battleField[10][10]);
void updateBattleField(int battlefield[10][10], char input[],char orientation,int lengthOfCarrier);
void toUpperCase(char positions[4][3]);
void getPositions(char positions[4][3],char * arsenal[4],int battleGround[10][10]);
void toUpperPosition(char input[]);
_Bool checkInputValidity(char input []);
_Bool checkPositionValidity(char input [],int lengthOfCaerrier,int BattleGround[10][10],char orientation);
int main()
{
    int BattleGroundPlayer1[10][10];
    int BattleGroundPlayer2[10][10];
    fillAray(BattleGroundPlayer1);
    fillAray(BattleGroundPlayer2);
    char difficulty[2];  
    int c;                                  //this matter later to clear the buffer
    printf("Please enter a tracking difficulty level: easy (e) or hard (h): ");
    scanf("%s",difficulty);
    while ((c = getchar()) != '\n' && c != EOF); //this is to clear the buffer otherwise it will repeat the message twice everytime (i took this from stack overflow because i could figure out what was going wrong but this seems to have fixed it)
    while (strlen(difficulty) > 1 || difficulty[0] != 'e' && difficulty [0] != 'h' && difficulty[0]!='E' && difficulty[0]!='H') {
        printf("The input you entered is invalid, please enter (e) for easy or (h) for hard: ");
        scanf("%s",difficulty);
        while ((c = getchar()) != '\n' && c != EOF); //this is to clear the buffer otherwise it will repeat the message twice everytime (i took this from stack overflow because i could figure out what was going wrong but this seems to have fixed it)
    }

    printf("Player 1 please enter your username: ");
    char Player1[16];                              //made the player name 15 characters long at most but we can change it however we like
    scanf("%s",Player1);
    while ((c = getchar()) != '\n' && c != EOF);
    printf("Player 2 please enter your username: ");
    char Player2[16];
    scanf("%s",Player2);
    while ((c = getchar()) != '\n' && c != EOF);

    srand(time(NULL));                              //this function allows us to set the a different seed for the random number each time our program runs so we don't have the same sequence of random numbers everytime we run our code
    int random = rand() % 2 ;                    //%2 gives either 0 or 1 which would guarantee a 50/50 chance for each player 
    printf("Player %i will go first.\n",random+1);
    char* arsenal[] = {"carrier","battleship","destroyer","submarine"};
    char positions1 [4][3];
    char positions2 [4][3];
    getPositions(positions1,arsenal,BattleGroundPlayer1);
    getPositions(positions2,arsenal,BattleGroundPlayer2);
    toUpperCase(positions1);
    toUpperCase(positions2);
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            printf("%d ", BattleGroundPlayer1[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            printf("%d ", BattleGroundPlayer2[i][j]);
        }
        printf("\n");
    }
    
}
/*this fillArray method fills in all the array spaces with 0, the reason i didn't
 initialize the first element to 0 and let the rest of the array get filled automatically is 
 because while testing, when i initialized the first element only, the last 7 rows got filled with garbage values,
 i believe this is caused by not allocating memory but we haven't covered that yet so i didn't want to confuse any of you guys.*/
void fillAray(int battlefield[10][10])
{
    for(int i = 0 ; i < 10;i++)
{
    for(int j = 0 ; j < 10;j++)
    {
        battlefield[i][j] = 0;
    }
}
}

/*typica; method to display the playing field, the difficulity char is to determin wether or not to print the misses also not only the hits*/
void displayBattleField(int battlefield[10][10], char difficulty)
{
    printf("    %c  %c  %c  %c  %c  %c  %c  %c  %c  %c\n", 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J');
    int index = 1;
    for(int i = 0 ; i < 10;i++)
    {
        printf("%-4i",index);
        index++;
        for(int j = 0 ; j < 10;j++)
        {
            if(battlefield[i][j] == 0)
            {
                printf("~  ");
            }
            else
            {
                if(difficulty == 'e' || difficulty == 'E')                   //as stated in the given instructions, diffirent dificulities print different stuff hence why i had to take 2 cases
                {
                    if (battlefield[i][j] == 1)         // 1 means we got a hit
                    {                                   // 0 means we didnt shoot in that position yet
                        printf("*  ");                  // 2 (didnt actually hard code it just put else) means we shot there but didnt hit anything 
                    }
                    else 
                    {
                        printf("o  ");                      // we shot there but didnt hit anything 
                    }
                }
                else
                {
                    if (battlefield[i][j] == 1)
                    {
                        printf("*  ");                     //we hit something
                    }
                    else 
                    {  
                        printf("~  ");                      //we dont show if we missed in hard difficulities
                    }
                }
                
            }
        }
        printf("\n");
    }
}
void toUpperCase(char position[4][3])                   //this is to convert the letters entered to upper case to make it easier to calculate where to update the array
{ 
    for(int i = 0 ; i < 4;i++)
    {
        for(int j = 0 ; j < 2;j++)
        {
            position[i][j] = toupper(position[i][j]);
        }
    }
}
void toUpperPosition(char input[])
{
    for(int i = 0 ; i < input[i] != '\0';i++)
    {
        input[i] = toupper((unsigned char)input[i]);
    }
}
void getPositions(char positions[4][3],char * arsenal[],int battleGround[10][10])
{
    int carriersLength [] = {5,4,3,2};
    int c;
    for(int i = 0 ; i  < 4;i++)
    {
        char orientation;
        int validInput = 0;
        while (!validInput) {
            printf("Please enter the position of your %s.\n", arsenal[i]);
            scanf("%3s", positions[i]);
            toUpperPosition(positions[i]);
            while ((c = getchar()) != '\n' && c != EOF); 
            if (checkInputValidity(positions[i]) == 0)
            {
                printf("The input you entered is invalid. Please enter a character between A and J and a number between 1 and 10.\n");
                continue; 
            }
            printf("what orientation? H/V\n");
            scanf("%c", &orientation);
            orientation = toupper(orientation);
            if(orientation!='H'&& orientation!='V')
            {
                printf("wrong input, please try again and enter a valid input\n");
                continue;
            }
            if (checkPositionValidity(positions[i], carriersLength[i], battleGround, orientation) == 0) {
                printf("Another carrier exists in these coordinates, or the position is invalid. Please enter valid ones.\n");
                continue;
            }
            validInput = 1;
        }
        updateBattleField(battleGround, positions[i], orientation , carriersLength[i]);
        for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            printf("%d ", battleGround[i][j]);
        }
        printf("\n");
    }
    }

}
_Bool checkInputValidity(char input [])
{
    if(strlen(input) < 2 || strlen(input) > 3)
    {
        return 0;
    }
    if(strlen(input)==2)
    {
        if(input[0] <'A' || input[0] > 'J')
    {
        return 0;
    }
    else if ( input[1] - '0' < 1 ||  input[1] - '0' > 9)
    {
        return 0;
    }
    }
    else if(strlen(input)==3)
    {
          if(input[0] <'A' || input[0] > 'J')
    {
        return 0;
    }
    else if ( input[1] - '0' > 1 ||  input[2] - '0' != 0)
    {
        return 0;
    }
    }
    return 1;
}
_Bool checkPositionValidity(char input [],int lengthOfCarrier,int BattleGround[10][10],char orientation)
{   
    int valid = 0;
    int x = input[0] - 'A';
    int y;
    if(strlen(input)==2)
    {
        y = input[1]-'1';
    }
    else if(strlen(input)==3)
    {
        y = 9;
    }
    if(x<0 || x >= 10 || y < 0 || y >=10)
    {
        if (x < 0 || x >= 10 || y < 0 || y >= 10) {
        printf("Starting position out of bounds.\n");
        return 0;
    }
    }
    if (orientation == 'V')
    {
        
        for(int i = y; i < y + lengthOfCarrier ;i++)
        {
            if(y >=10)
            {
                printf("Ship goes out of bounds vertically.\n");
                return 0;
            }
            if(BattleGround[i][x]==0)
            {
                valid = 1;
            }
            else
            {
                return 0;
            }
        }
    }
    if(orientation=='H')
    {
        for(int i = x ; i < x + lengthOfCarrier ;i++)
        {
            if(x >=10)
            {
                printf("Ship goes out of bounds horizontally.\n");
                return 0;
            }
            if(BattleGround[y][i]==0)
            {
                valid = 1;
            }
            else
            {
                return 0;
            }
        }
    }
    return valid;
}
void updateBattleField(int battlefield[10][10], char input[],char orientation,int lengthOfCarrier)
{   
    int x = input[0] - 'A';
    int y;
    if(strlen(input)==2)
    {
        y = input[1]-'1';
    }
    else if(strlen(input)==3)
    {
        y = 9;
    }
    if (orientation == 'V')
    {
        for(int i = y; i < y + lengthOfCarrier ;i++)
        {
            battlefield[i][x]=1;
        }
    }
    else if(orientation=='H')
    {
        for(int i = x; i < x + lengthOfCarrier ;i++)
        {
            battlefield[y][i]=1;
        }
    }
    
}