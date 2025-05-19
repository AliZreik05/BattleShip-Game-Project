#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>                 //this is needed for the random function so we can guarantee that the whoever gets first serve is random
#include <time.h>                   /*this is also needed to set a random seed for the random function, 
                                     this is so we dont get the same sequence of random numbers each time*/
void getDifficulity(char difficulity[2]);
void getPlayerInfo(char name[16],int i);
int getRandomTurn();
void displayBattleField(int battlefield[10][10], char difficulty);
void fillAray(int battleField[10][10]);
void updateBattleField(int battlefield[10][10], char input[],char orientation,int lengthOfCarrier);
void toUpperCase(char positions[4][3]);
void getPositions(char positions[4][3],char * arsenal[4],int battleGround[10][10]);
void toUpperPosition(char input[]);
void makeMove(int battlefield[10][10],int *numberOfShips);
_Bool checkInputValidity(char input []);
_Bool checkPositionValidity(char input [],int lengthOfCaerrier,int BattleGround[10][10],char orientation);
void clearScreen() 
{
#ifdef _WIN32
    system("cls");   // For Windows
#else
    system("clear"); // For Linux/MacOS
#endif
}
int main()
{
    int BattleGroundPlayer1[10][10];
    int BattleGroundPlayer2[10][10];
    fillAray(BattleGroundPlayer1);                                  //FILLING THE ARRAYS WITH 0
    fillAray(BattleGroundPlayer2);
    char difficulity[2];  
    getDifficulity(difficulity);                                        //GETTING DIFFICULITY
    clearScreen();  
    int c;
    char PlayerOneName [16];
    char PlayerTwoName [16];
    getPlayerInfo(PlayerOneName,1);                                         //getting names
    getPlayerInfo(PlayerTwoName,2);
    clearScreen();
    int currentPlayer = getRandomTurn();
    char* arsenal[] = {"carrier","battleship","destroyer","submarine"};
    char positions1 [4][3];
    char positions2 [4][3];
    getPositions(positions1,arsenal,BattleGroundPlayer1);                       //gettings positions
    clearScreen();
    getPositions(positions2,arsenal,BattleGroundPlayer2);
    toUpperCase(positions1);                                                    //making positions in upper case in order to make it easier for us to update array
    toUpperCase(positions2);
    int numberOfShipsPlayer1 = 4;
    int numberOfshipsPlayer2 = 4;
    do                                                                      //this needs some fine tuning when we implement our own methods i believe
    {
        if(currentPlayer == 1)
        {
            printf("Player 1's turn:\n");
            displayBattleField(BattleGroundPlayer1,difficulity[0]);
            makeMove(BattleGroundPlayer2,&numberOfshipsPlayer2);                                    
            currentPlayer = 2;                                                              
        }
        else
        {
            printf("Player 2's turn:\n");
            displayBattleField(BattleGroundPlayer2,difficulity[0]);
            makeMove(BattleGroundPlayer1,&numberOfShipsPlayer1);
            currentPlayer = 1;
        }
    } 
    while (numberOfShipsPlayer1 !=0 && numberOfshipsPlayer2 !=0);
    printf("Game Over!");
    if(numberOfShipsPlayer1 == 0)
    {
        printf("Player 2 is the winner");
    }
    else 
    {
        printf("Player 1 is the winner");
    }
    
}
void getDifficulity(char difficulity[2])
{
    int c;                                  //this matter later to clear the buffer
    printf("Please enter a tracking difficulty level: easy (e) or hard (h): ");
    scanf("%s",difficulity);
    while ((c = getchar()) != '\n' && c != EOF); //this is to clear the buffer otherwise it will repeat the message twice everytime (i took this from stack overflow because i could figure out what was going wrong but this seems to have fixed it)
    while (strlen(difficulity) > 1 || difficulity[0] != 'e' && difficulity [0] != 'h' && difficulity[0]!='E' && difficulity[0]!='H') {
        printf("The input you entered is invalid, please enter (e) for easy or (h) for hard: ");
        scanf("%s",difficulity);
        while ((c = getchar()) != '\n' && c != EOF); //this is to clear the buffer otherwise it will repeat the message twice everytime (i took this from stack overflow because i could figure out what was going wrong but this seems to have fixed it)
    }
}
void getPlayerInfo(char Player [16],int i)
{
    int c;
    printf("Player %d please enter your username: ",i);
    scanf("%s",Player);
    while ((c = getchar()) != '\n' && c != EOF);
}
int getRandomTurn()
{
    srand(time(NULL));                              //this function allows us to set the a different seed for the random number each time our program runs so we don't have the same sequence of random numbers everytime we run our code
    int random = rand() % 2 ;                    //%2 gives either 0 or 1 which would guarantee a 50/50 chance for each player 
    printf("Player %i will go first.\n",random+1);
    return random;
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
            if(battlefield[i][j] == 0 || battlefield[i][j] == 1)        // 0 or 1 means we didnt shoot in that position yet
            {
                printf("~  ");              
            }
            else
            {
                if(difficulty == 'e' || difficulty == 'E')                   //as stated in the given instructions, diffirent dificulities print different stuff hence why i had to take 2 cases
                {
                    if (battlefield[i][j] == 2)         
                    {                                   // 2 means hit
                        printf("*  ");                  // 3 (didnt actually hard code it just put else) means we shot there but didnt hit anything 
                    }
                    else 
                    {
                        printf("o  ");                      // we shot there but didnt hit anything 
                    }
                }
                else
                {
                    if (battlefield[i][j] == 2)
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
void toUpperPosition(char input[])                          //this is a method to turn string into uppercase since there is no default way to do this as far as i am aware
{
    for(int i = 0 ; i < input[i] != '\0';i++)
    {
        input[i] = toupper((unsigned char)input[i]);
    }
}
void makeMove(int battlefield[10][10],int* numberOfShips)               //this is a method that will be responsibly for making moves,
{                                                                       // in the future, this method shall be updated with if statements 
    char xchar;                                                         //we have to check for the inputted ability
    int y;                                                              //i.e: we check which ability is used and then we call it here
    printf("Enter coordinates (A-J, 1-10): ");                          //so we implement different methods for each ability and we just call them here
    scanf("%c %d", &xchar, &y);                                         //note: right now the written code is the one for the normal fire ability
    xchar = toupper(xchar);                                             // i shall soon make its own method for it and fix/ improve the firing
    int x = xchar - 'A';
    y = y-1;
    if (x < 0 || x > 9 || y < 0 || y > 9) {
        printf("Invalid input. Please enter a letter from A-J and a number from 1-10.\n");
        return;                                                        //check the move for any illegal arguments
    }
    if(battlefield[x][y]== 1)
    {
        printf("Hit!\n");
        battlefield[x][y] = 2;                                          //VERY IMPORTANT NOTE: FOR WHEN WE DO THE FIRE METHOD (IF I (ALI)DOESNT DO IT SOON)
        numberOfShips--;                                                //THIS METHOD DOESNT CHECK FOR COLLISIONS, IN OTHER WORDS: WE COULD HIT THE SAME
    }                                                                   // CARRIER EACH TIME AND IT WILL COUNT DOWN EACH TIME
    else                                                                // BE VERY AWAREEEEEE
    {                                                                   // AND ALSO APPLY COLLISION CHECKING IN YOUR METHODS FOR WHOEVER I DOING A
        printf("Miss!\n");                                              // METHOD THAT HAS FIRING IN IT!!!!!
        battlefield[x][y] = 3;
    }
    clearScreen();
}
void getPositions(char positions[4][3],char * arsenal[],int battleGround[10][10])               //method to get positions from user and update the battlefield with the coordinates
{
    int carriersLength [] = {5,4,3,2};                                      //self explanatory, so we can know the length of each carrier
    int c;  
    for(int i = 0 ; i  < 4;i++)
    {
        char orientation;                                                   //V/H
        int validInput = 0;
        while (!validInput) {                                               //so we can detect any illegal arguments passed by the user
            printf("Please enter the position of your %s.\n", arsenal[i]);
            scanf("%3s", positions[i]);
            toUpperPosition(positions[i]);                                     // make this easier for us to update the battlefield later
            while ((c = getchar()) != '\n' && c != EOF); 
            if (checkInputValidity(positions[i]) == 0)                         //this is to check if the input entered is the correct format or not
            {
                printf("The input you entered is invalid. Please enter a character between A and J and a number between 1 and 10.\n");
                continue; 
            }
            printf("what orientation? H/V\n");                                 //getting the orientation
            scanf("%c", &orientation);
            orientation = toupper(orientation);                                //it's one letter thats why i didnt use toupperposition
            if(orientation!='H'&& orientation!='V')                             //validity checks
            {
                printf("wrong input, please try again and enter a valid input\n");      
                continue;
            }
            if (checkPositionValidity(positions[i], carriersLength[i], battleGround, orientation) == 0) {       //this is to check if the entered position is taken,goes out of bound...etc
                printf("Another carrier exists in these coordinates, or the position is invalid. Please enter valid ones.\n");
                continue;
            }
            validInput = 1;
        }
        updateBattleField(battleGround, positions[i], orientation , carriersLength[i]);             //once all checks are done we update
        for (int i = 0; i < 10; i++) 
        {
        for (int j = 0; j < 10; j++) 
        {
            printf("%d ", battleGround[i][j]);                                                          //print the battlefield after each insertion
        }
        printf("\n");
    }
    }

}
_Bool checkInputValidity(char input [])                                                         //so we can check if the position inputed is in the correct format
{
    if(strlen(input) < 2 || strlen(input) > 3)                                                  //checking for length, less than 2 means if only A or 1... more than 3 is for sdhjah for example
    {
        return 0;
    }
    if(strlen(input)==2)                                                                //if it is from A-J and 1-9 only(2 characters long)
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
    else if(strlen(input)==3)                                                              // if it is from A-J and 10
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
_Bool checkPositionValidity(char input [],int lengthOfCarrier,int BattleGround[10][10],char orientation)            //after checking the input, we check if the position inputted is correct or not
{   
    int valid = 0;
    int x = input[0] - 'A';
    int y;
    if(strlen(input)==2)
    {
        y = input[1]-'1';                           //you may wonder why the 2 and 3 and u are correct
    }                                               //if it was made of 2 then it is in the form A0 for example
    else if(strlen(input)==3)                       // we can then directly get Y, but if it was 3 characters long, then it would be more of a hassle
    {                                               // to get y so it is better for us to hardcode
        y = 9;
    }
    if(x<0 || x >= 10 || y < 0 || y >=10)           //check for out of bounds inputs
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
                printf("Ship goes out of bounds vertically.\n");                    //if the ship goes out of bounds even though its first position is in bounds
                return 0;
            }
            if(BattleGround[i][x]==0)                                               //if it wasnt 0, then other ship exists there and the position is not available
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
                printf("Ship goes out of bounds horizontally.\n");                   //if the ship goes out of bounds even though its first position is in bounds
                return 0;   
            }
            if(BattleGround[y][i]==0)                                               //if it wasnt 0, then other ship exists there and the position is not available
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
void updateBattleField(int battlefield[10][10], char input[],char orientation,int lengthOfCarrier)              //Updating the battlefield after positions are entered
{   
    int x = input[0] - 'A';
    int y;
    if(strlen(input)==2)                                               //to get Y, this is the same tactic used in the get positions method
    {
        y = input[1]-'1';
    }
    else if(strlen(input)==3)                                           
    {
        y = 9;
    }
    if (orientation == 'V')                                     //very simple updating vertically/horizontally using length of the carriers
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
