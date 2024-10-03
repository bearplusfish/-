// name: Jiayu Xiong
// unikey: jxio5417
// SID: 500037218

#include <stdio.h>
#include <string.h>

#define SIZE 19
int board[SIZE][SIZE]={0};//board: 19X19   row*column
char history[1000] = "";// record history of placement  size is at most >=19*9*2+19*10*3+1=913
int gameRound = 1; // set gameRound from 1 when game starts, which means Black first
int xMist = 9, yMist = 9; // the hole of the Mist locates at the centre of the board before the first move, initial setting xMist(col), yMist(row)

void calc_mist_center(int xStone, int yStone){ // A2--> xStone(col)=1, yStone(row)=2
	xMist = ( 5*xStone*xStone + 3*xStone + 4) % 19; //col
	yMist = ( 4*yStone*yStone + 2*xStone - 4) % 19; //row
}

void display() // initially display the board
{
    char column = 'A';
    for (int i = SIZE - 1; i >= 0; i--)
    {
        for (int j = 0; j <= SIZE; j++)
        {
            if (j == 0)
            {
            	printf("%-2d", i + 1); // print row tag 19, 18, 17, ..., 1
            } else{
	            if (board[i][j-1] == 0)//blank
	                printf(" .");
	            else if (board[i][j-1] == 1)//black
	                printf(" #");
	            else if (board[i][j-1] == 2)//white
	                printf(" o");
            }
        }
        printf("\n");
    }
    printf("  ");
    for(int i = 0; i < SIZE; i++)
    { // print column tag A, B, C, ..., S
        printf(" %c", column + i);
    }
    printf("\n");
    return;
}


void who_next(){
	printf("%s\n", gameRound % 2 == 1? "B": "W");
	return;
}


int is_in_a_line(int row, int col){ // judge whether there are five stones (or more than 5) in a line with the current location (row, col) placed.
	int line = 1; // horizontally;    initially set 1, current location is placed with one stone
	for(int j=col-1; j>=0; j--){ // search left
		if(board[row][col] == board[row][j])
			line++;
		else
			break;
	}
	for(int j=col+1; j<=SIZE-1; j++){ // search right
		if(board[row][col] == board[row][j])
			line++;
		else
			break;
	}
	if(line >= 5)
		return 1;
		
	line = 1; // vertically 
	for(int i=row-1; i>=0; i--){ // search down
		if(board[row][col] == board[i][col])
			line++;
		else
			break;
	}
	for(int i=row+1; i<=SIZE-1; i++){ // search up
		if(board[row][col] == board[i][col])
			line++;
		else
			break;
	}
	if(line >= 5)
		return 1;
		
	line = 1; // diagonally 1 
	for(int i=-1; row+i>=0 && col+i>=0; i--){ // search left down
		if(board[row][col] == board[row+i][col+i])
			line++;
		else
			break;
	}
	for(int i=1; row+i<=SIZE-1 && col+i<=SIZE-1; i++){ // search right up
		if(board[row][col] == board[row+i][col+i])
			line++;
		else
			break;
	}
	if(line >= 5)
		return 1;
		
	line = 1; // diagonally 2 
	for(int i=1; row+i<=SIZE-1 && col-i>=0; i++){ // search left up
		if(board[row][col] == board[row+i][col-i])
			line++;
		else
			break;
	}
	for(int i=1; row-i>=0 && col+i<=SIZE-1; i++){ // search right down
		if(board[row][col] == board[row-i][col+i])
			line++;
		else
			break;
	}
	if(line >= 5)
		return 1;
		
	return 0;
}


int place_stone(char command[], char loc[]) // place a stone   command = "place A1";
{
	if(strlen(loc) > 3 || strlen(loc) < 2 || loc[1] == '0'){ // The length of coordinate is not 2 or 3.
		printf ("Invalid coordinate\n");	
		return 0;
	}
    char col = loc[0]; // column coordinate, e.g. A, B, C
    int row = 0; // row coordinate
    for(int i=1; i<strlen(loc); i++){
    	if(loc[i] < '0' || loc[i] > '9'){ // if row coordinate is not digit
    		printf ("Invalid coordinate\n");
    		return 0;
		}
    	row = row * 10 + (loc[i]-'0');
	}
    if(col < 'A' || col > 'S' || row < 1 || row > SIZE){ // out of range
    	printf ("Invalid coordinate\n");
    	return 0;
	}
	if(board[row-1][col-'A'] != 0){ // if there is a stone in the location
		printf("Occupied coordinate\n");
        return 0;
	}
	board[row-1][col-'A'] = gameRound % 2 == 1 ? 1 : 2; // alter the board with the current placement
	calc_mist_center(col-'A'+1, row); // calculate the mist center and update it
//	strncat(history, command+6, strlen(command)-7); // append history placement
	strcat(history, loc); // append history placement
	if(is_in_a_line(row-1, col-'A')){ // if there are 5 stones in a line
		printf("%s wins!\n", gameRound%2==1?"Black":"White");
		return 1;
	}		
	gameRound++; // game round will be added 1 after the placement
	if(gameRound > 19*19){ // end when no more stones can be placed 
		printf("Wow, a tie!\n");
		return 1;
	}
	return 0;
}


void resign_game(){ // one player resign the game
	if(gameRound % 2 == 1){ // judge who resigns the game
		printf("White wins!\n");
	} else{
		printf("Black wins!\n");
	}
	return;
}

void view_state(){ // print the 7 × 7 hole
	printf("%c%d,", 'A'+xMist, 1+yMist); // print the center location
	for(int i = yMist+3; i >= yMist-3; i--){
		for(int j = xMist-3; j <= xMist+3; j++){
			if(i<0 || i>=SIZE || j<0 || j>=SIZE){ // if out of range, print 'x'
				printf("x");
			} else if(board[i][j] == 0){
				printf(".");
			} else if(board[i][j] == 1){
				printf("#");
			} else if(board[i][j] == 2){
				printf("o");
			}
		}
	}
	printf("\n");
	return;
}

void lookup_history(){ // print history placement
	printf("%s\n", history);
	return;
}

int main()
{
//	display();
	char command[2000], loc[2000];
	char *token;
	int nParam = 0;
	while (fgets(command, 1999, stdin)) {  // fgets(command, 99, stdin)   gets(command)
		strtok(command, "\n"); // Remove the trailing newline symbol		
		if(strncmp(command, "place ", 6) == 0 && command[6] != ' ' && command[strlen(command)-1] != ' ') {
			nParam = 0;
			token = strtok(command, " ");
			while(token != NULL){
				nParam++;
				if(nParam == 2)
					strcpy(loc, token);
				token = strtok(NULL, " ");
			}
//			printf("%s, %d\n", loc, strlen(loc));
//			printf("%d %d\n", strlen(command), nParam);
			if(nParam != 2)
				printf("Invalid!\n");
			else if(place_stone(command, loc)){ // if one plaer wins or tie, jump out of the loop
				break;
			}
		} else if(strcmp(command, "who") == 0){
			who_next();
		} else if(strcmp(command, "term") == 0){ // if one plaer terms, exit with code 1
			return 1;
		} else if(strcmp(command, "resign") == 0){ // if one plaer resigns, jump out of the loop
			resign_game();
			break;
		} else if(strcmp(command, "view") == 0){
			view_state();
		} else if(strcmp(command, "history") == 0){
			lookup_history();
		} else{
			printf("Invalid!\n");
		}
	}
	lookup_history();
	printf("Thank you for playing!\n");
	return 0;
}
