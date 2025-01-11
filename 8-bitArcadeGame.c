#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#endif
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#define COMPUTER 1
#define HUMAN 2
#define DRAWS 9
#define SIDE 3
#define COMPUTERMOVE 'O'
#define HUMANMOVE 'X'

#define WIDTH 40
#define HEIGHT 10
#define MAX_SCORE 5
FILE *filePointer;
struct Move {
  int row, col;
};

// Global variables for Tic-Tac-Toe
char player = 'X', opponent = 'O';
char name_tictactoe[20];
// Global variables for Pong
int ballX, ballY;
int ballDirX = 1, ballDirY = 1;
int paddle1Y, paddle2Y;
int paddleHeight = 3;
int player1Score, player2Score;
int botWins = 0, humanWins = 0, draws = 0; // For Tic-Tac-Toe
int player1Wins = 0, player2Wins = 0;      // For Pong
int pongGames = 0, tictactoeGames = 0;     // To track total games

// Function declarations
void playTicTacToe(int whoseTurn);
void leadTic();
void initializeGame();
void drawBoard();
void moveBall();
void handleInput();
int kbhit();
void movePaddle(int player, int direction);
void showInstructions();
void showBoard(char board[][SIDE]);
void declareWinner(int whoseTurn);
void initialise(char board[][SIDE], int moves[]);
int gameOver(char board[][SIDE]);
int evaluate(char b[3][3]);
int minimax(char board[3][3], int depth, bool isMax);
struct Move findBestMove(char board[3][3]);
bool isMovesLeft(char board[3][3]);
int rowCrossed(char board[][SIDE]);
int columnCrossed(char board[][SIDE]);
int diagonalCrossed(char board[][SIDE]);
int getGameChoice();

//--------------------------- Ping pong game -------------------------------------------//

struct Player {
    char name[20];
    int wins;
    int loses;
    float winRate;
};

// Load players from file
int loadPlayers(struct Player players[], int *playerCount) {
    FILE *filePointer = fopen("Pingpong.txt", "r");
    if (filePointer == NULL) {
        return 0;
    }

    char line[100];
    *playerCount = 0;

    while (fgets(line, sizeof(line), filePointer)) {
        struct Player player;
        if (sscanf(line, "name: %s wins: %d loses: %d win-rate: %f%%",
                   player.name, &player.wins, &player.loses, &player.winRate) == 4) {
            players[(*playerCount)++] = player;
        }
    }
    fclose(filePointer);
    return 1;
}

// Find player in list
int findPlayerIndex(struct Player players[], int playerCount, const char *name) {
    for (int i = 0; i < playerCount; i++) {
        #ifdef _WIN32
        if (_stricmp(players[i].name, name) == 0) {
        #else
        if (strcasecmp(players[i].name, name) == 0) {
        #endif
            return i;
        }
    }
    return -1;
}

// Update player stats
void updatePlayer(struct Player players[], int *playerCount, const char *name, int win) {
    int index = findPlayerIndex(players, *playerCount, name);
    if (index == -1) {
        strcpy(players[*playerCount].name, name);
        players[*playerCount].wins = win ? 1 : 0;
        players[*playerCount].loses = win ? 0 : 1;
        players[*playerCount].winRate = win ? 100.0 : 0.0;
        (*playerCount)++;
    } else {
        if (win) {
            players[index].wins++;
        } else {
            players[index].loses++;
        }
        int totalGames = players[index].wins + players[index].loses;
        players[index].winRate = (float)players[index].wins / totalGames * 100.0;
    }
}

void savePlayers(struct Player players[], int playerCount) {
    FILE *filePointer = fopen("Pingpong.txt", "w");
    if (filePointer == NULL) {
        fprintf(stderr, "Error: Could not open Pingpong.txt for writing.\n");
        return;
    }

    for (int i = 0; i < playerCount; i++) {
        fprintf(filePointer, "name: %s wins: %d loses: %d win-rate: %.2f%%\n",
                players[i].name, players[i].wins, players[i].loses, players[i].winRate);
    }
    fclose(filePointer);
}

// Initialize the game variables for Pong
void initializeGame() {
  ballX = WIDTH / 2;
  ballY = HEIGHT / 2;
  paddle1Y = paddle2Y = HEIGHT / 2 - paddleHeight / 2;
}

// Draw the Pong game board
void drawBoard() {
  #ifdef _WIN32
  system("cls");
  #else
  system("clear");
  #endif
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      // Draw ball
      if (x == ballX && y == ballY) {
        printf("O");
      }
      // Draw paddles
      else if (x == 1 && y >= paddle1Y && y < paddle1Y + paddleHeight) {
        printf("|");
      } else if (x == WIDTH - 2 && y >= paddle2Y &&
                 y < paddle2Y + paddleHeight) {
        printf("|");
      }
      // Draw borders
      else if (y == 0 || y == HEIGHT - 1) {
        printf("-");
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }
  printf("Player 1: %d\tPlayer 2: %d\n", player1Score, player2Score);
  // Show controls at the bottom
  printf("\nControls:\n");
  printf("Player 1 (Left Paddle):\n");
  printf("  W - Move Up\n");
  printf("  S - Move Down\n");
  printf("Player 2 (Right Paddle):\n");
  printf("  I - Move Up\n");
  printf("  K - Move Down\n");
  printf("Press Q to Quit\n");
}

// Move the ball in Pong
void moveBall() {
  ballX += ballDirX;
  ballY += ballDirY;

  // Check collision with top and bottom
  if (ballY <= 0 || ballY >= HEIGHT - 1) {
    ballDirY = -ballDirY;
  }

  // Check collision with paddles
  if (ballX == 2 && ballY >= paddle1Y && ballY < paddle1Y + paddleHeight) {
    ballDirX = -ballDirX;
  } else if (ballX == WIDTH - 3 && ballY >= paddle2Y &&
             ballY < paddle2Y + paddleHeight) {
    ballDirX = -ballDirX;
  }

  // Check for scoring
  if (ballX <= 0) {
    player2Score++;
    initializeGame();
  } else if (ballX >= WIDTH - 1) {
    player1Score++;
    initializeGame();
  }
}

// Move paddles in Pong
void movePaddle(int player, int direction) {
  if (player == 1) {
    if (direction == -1 && paddle1Y > 0) {
      paddle1Y--;
    } else if (direction == 1 && paddle1Y < HEIGHT - paddleHeight) {
      paddle1Y++;
    }
  } else {
    if (direction == -1 && paddle2Y > 0) {
      paddle2Y--;
    } else if (direction == 1 && paddle2Y < HEIGHT - paddleHeight) {
      paddle2Y++;
    }
  }
}

// Check if input is available for Pong
int kbhit() {
#ifdef _WIN32
    return _kbhit();
#else
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
#endif
}

// Handle player input for Pong
void handleInput() {
  if (kbhit()) {
#ifdef _WIN32
    char input = _getch();
#else
    char input = getchar();
#endif
    switch (input) {
    case 'w':
      movePaddle(1, -1);
      break; // Player 1 up
    case 's':
      movePaddle(1, 1);
      break; // Player 1 down
    case 'i':
      movePaddle(2, -1);
      break; // Player 2 up
    case 'k':
      movePaddle(2, 1);
      break; // Player 2 down
    case 'q':
      exit(0);
      break; // Quit game
    }
  }
}

// Main function to choose the game
int getGameChoice() {
  int choice;
  printf("Choose a game to play:\n");
  printf("1. Tic-Tac-Toe\n");
  printf("2. Ping-Pong\n");
  printf("3. Leaderboard\n");
  printf("Enter your choice: ");
  scanf("%d", &choice);

  return choice;
}

void searchTicLeaderboard(char *searchName) {
    FILE *filePointer = fopen("Tictactoe.txt", "r");
    char line[100];
    int found = 0;

    if (filePointer == NULL) {
        perror("Error opening Tic-Tac-Toe leaderboard file");
        return;
    }

    printf("\n--- Tic-Tac-Toe Player Search Results ---\n");
    printf("%-15s %-8s %-8s %-8s %-8s\n", "Name", "Wins", "Losses", "Draws", "Win Rate (%)");
    printf("--------------------------------------------------------\n");

    while (fgets(line, sizeof(line), filePointer)) {
        char name[20];
        int wins, losses, draws;
        float winrate;

        if (sscanf(line, "name: %s wins: %d loses: %d draws: %d win-rate: %f",
                   name, &wins, &losses, &draws, &winrate) == 5) {
            #ifdef _WIN32
            if (_stricmp(name, searchName) == 0) {
            #else
            if (strcasecmp(name, searchName) == 0) {
            #endif
                printf("%-15s %-8d %-8d %-8d %-8.2f%%\n", 
                       name, wins, losses, draws, winrate);
                found = 1;
            }
        }
    }

    if (!found) {
        printf("No player found with the name: %s\n", searchName);
    }

    fclose(filePointer);
}

void searchPongLeaderboard(char *searchName) {
    FILE *filePointer = fopen("Pingpong.txt", "r");
    char line[100];
    int found = 0;

    if (filePointer == NULL) {
        perror("Error opening Ping-Pong leaderboard file");
        return;
    }

    printf("\n--- Ping-Pong Player Search Results ---\n");
    printf("%-15s %-8s %-8s %-8s\n", "Name", "Wins", "Losses", "Win Rate (%)");
    printf("--------------------------------------------------------\n");

    while (fgets(line, sizeof(line), filePointer)) {
        char name[20];
        int wins, losses;
        float winrate;

        if (sscanf(line, "name: %s wins: %d loses: %d win-rate: %f%%",
                   name, &wins, &losses, &winrate) == 4) {
            #ifdef _WIN32
            if (_stricmp(name, searchName) == 0) {
            #else
            if (strcasecmp(name, searchName) == 0) {
            #endif
                printf("%-15s %-8d %-8d %-8.2f%%\n", 
                       name, wins, losses, winrate);
                found = 1;
            }
        }
    }

    if (!found) {
        printf("No player found with the name: %s\n", searchName);
    }

    fclose(filePointer);
}

// Main game loop for Pong
void playPong() {
  player1Score = 0;
  player2Score = 0;
  char name_pong1[20];
  char name_pong2[20];

  printf("Enter player1 name: ");
  scanf("%s", name_pong1);
  printf("Enter player2 name: ");
  scanf("%s", name_pong2);

  struct Player players[100];
  int playerCount = 0;
  loadPlayers(players, &playerCount);

  initializeGame();
  while (1) {
      drawBoard();
      moveBall();

      if (player1Score == MAX_SCORE || player2Score == MAX_SCORE) {
          pongGames++;

          char name_winner[20], name_loser[20];
          if (player1Score == MAX_SCORE) {
              strcpy(name_winner, name_pong1);
              strcpy(name_loser, name_pong2);
              player1Wins++;
          } else {
              strcpy(name_winner, name_pong2);
              strcpy(name_loser, name_pong1);
              player2Wins++;
          }

          printf("%s wins!\n", name_winner);

          updatePlayer(players, &playerCount, name_pong1, player1Score == MAX_SCORE);
          updatePlayer(players, &playerCount, name_pong2, player2Score == MAX_SCORE);

          savePlayers(players, playerCount);

          break;
      }

      handleInput();
#ifdef _WIN32
      Sleep(100);
#else
      usleep(100000);
#endif
  }
}

//-------------------------------------------------------------------------------------------//
// ----------------------- Leaderboard for ping-pong -----------------------//
//Leaderboard
void leadPong() {
    struct PlayerStats {
        char name[20];
        int wins;
        int losses;
        float winrate;
    } players[20];
    int playerCount = 0;
    FILE *filePointer = fopen("Pingpong.txt", "r");

    if (filePointer == NULL) {
        perror("Error opening file");
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), filePointer)) {
        if (sscanf(line, "name: %s wins: %d loses: %d win-rate: %f%%",
                   players[playerCount].name, &players[playerCount].wins,
                   &players[playerCount].losses, &players[playerCount].winrate) == 4) {
            playerCount++;
        }
    }
    fclose(filePointer);

    for (int i = 0; i < playerCount - 1; i++) {
        for (int j = i + 1; j < playerCount; j++) {
            if (players[i].winrate < players[j].winrate) {
                struct PlayerStats temp = players[i];
                players[i] = players[j];
                players[j] = temp;
            }
        }
    }

    printf("\n--- Ping-Pong Leaderboard ---\n");
    printf("%-15s %-8s %-8s %-8s\n", "Name", "Wins", "Losses", "Win Rate (%)");
    printf("--------------------------------------------------------\n");

    for (int i = 0; i < playerCount; i++) {
        printf("%-15s %-8d %-8d %-8.2f%%\n",
               players[i].name, players[i].wins, players[i].losses,
               players[i].winrate);
    }
    printf("--------------------------------------------------------\n");
}

//---------------------------------------------------------------------//

// Tic-Tac-Toe functions follow...

// Function implementations for Tic-Tac-Toe

//----------------------------Tic-Tac-Toe-----------------------------//
bool isMovesLeft(char board[3][3]) {
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      if (board[i][j] == '_')
        return true;
  return false;
}

int evaluate(char b[3][3]) {
  for (int row = 0; row < 3; row++) {
    if (b[row][0] == b[row][1] && b[row][1] == b[row][2]) {
      if (b[row][0] == player)
        return +10;
      else if (b[row][0] == opponent)
        return -10;
    }
  }

  for (int col = 0; col < 3; col++) {
    if (b[0][col] == b[1][col] && b[1][col] == b[2][col]) {
      if (b[0][col] == player)
        return +10;
      else if (b[0][col] == opponent)
        return -10;
    }
  }

  if (b[0][0] == b[1][1] && b[1][1] == b[2][2]) {
    if (b[0][0] == player)
      return +10;
    else if (b[0][0] == opponent)
      return -10;
  }

  if (b[0][2] == b[1][1] && b[1][1] == b[2][0]) {
    if (b[0][2] == player)
      return +10;
    else if (b[0][2] == opponent)
      return -10;
  }

  return 0;
}

int minimax(char board[3][3], int depth, bool isMax) {
  int score = evaluate(board);

  if (score == 10)
    return score;

  if (score == -10)
    return score;

  if (isMovesLeft(board) == false)
    return 0;

  if (isMax) {
    int best = -1000;

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (board[i][j] == '_') {
          board[i][j] = player;
          int val = minimax(board, depth + 1, !isMax);
          best = (val > best) ? val : best;
          board[i][j] = '_';
        }
      }
    }
    return best;
  } else {
    int best = 1000;

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (board[i][j] == '_') {
          board[i][j] = opponent;
          int val = minimax(board, depth + 1, !isMax);
          best = (val < best) ? val : best;
          board[i][j] = '_';
        }
      }
    }
    return best;
  }
}

struct Move findBestMove(char board[3][3]) {
  int bestVal = -1000;
  struct Move bestMove;
  bestMove.row = -1;
  bestMove.col = -1;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (board[i][j] == '_') {
        board[i][j] = player;
        int moveVal = minimax(board, 0, false);
        board[i][j] = '_';
        if (moveVal > bestVal) {
          bestMove.row = i;
          bestMove.col = j;
          bestVal = moveVal;
        }
      }
    }
  }

  return bestMove;
}

void showBoard(char board[][SIDE]) {
  printf("\n\n");
  printf("\t\t\t %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
  printf("\t\t\t--------------\n");
  printf("\t\t\t %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
  printf("\t\t\t--------------\n");
  printf("\t\t\t %c | %c | %c \n\n", board[2][0], board[2][1], board[2][2]);
}

void showInstructions() {
  printf("\t\t\t Tic-Tac-Toe\n\n");
  printf("Choose a cell numbered from 1 to 9 as below and play\n\n");

  printf("\t\t\t 1 | 2 | 3 \n");
  printf("\t\t\t--------------\n");
  printf("\t\t\t 4 | 5 | 6 \n");
  printf("\t\t\t--------------\n");
  printf("\t\t\t 7 | 8 | 9 \n\n");

  printf("-\t-\t-\t-\t-\t-\t-\t-\t-\t-\n\n");
}

void initialise(char board[][SIDE], int moves[]) {
  srand(time(NULL));

  for (int i = 0; i < SIDE; i++) {
    for (int j = 0; j < SIDE; j++)
      board[i][j] = '_';
  }

  for (int i = 0; i < SIDE * SIDE; i++)
    moves[i] = i;

  for (int i = 0; i < SIDE * SIDE; i++) {
    int randIndex = rand() % (SIDE * SIDE);
    int temp = moves[i];
    moves[i] = moves[randIndex];
    moves[randIndex] = temp;
  }
}

void declareWinner(int whoseTurn) {
  tictactoeGames++;

  FILE *filePointer = fopen("Tictactoe.txt", "r+"); 
  if (filePointer == NULL) {
      filePointer = fopen("Tictactoe.txt", "w");
      if (filePointer == NULL) {
          perror("Error opening file");
          return;
      }
  }

  char line[100];
  int found = 0;
  int userWins = 0, userLosses = 0, userDraws = 0;

  FILE *tempFile = fopen("TempTictactoe.txt", "w");
  if (tempFile == NULL) {
      perror("Error creating temp file");
      fclose(filePointer);
      return;
  }

  while (fgets(line, sizeof(line), filePointer) != NULL) {
    char name[20];
    int wins, losses, draws;

    if (sscanf(line, "name: %s wins: %d loses: %d draws: %d", name, &wins,
               &losses, &draws) == 4) {
      if (strcmp(name, name_tictactoe) == 0) {
        found = 1;
        userWins = wins;
        userLosses = losses;
        userDraws = draws;

        if (whoseTurn == COMPUTER) {
          userLosses++;
        } else if (whoseTurn == HUMAN) {
          userWins++;
        } else {
          userDraws++;
        }
        fprintf(
            tempFile, "name: %s wins: %d loses: %d draws: %d win-rate: %.2f\n",
            name_tictactoe, userWins, userLosses, userDraws,
            (float)((float)userWins / (float)(userWins + userLosses)) * 100);
      } else {
        fputs(line, tempFile);
      }
    } else {
      fputs(line, tempFile);
    }
  }

  if (!found) {
    if (whoseTurn == COMPUTER) {
      userLosses++;
    } else if (whoseTurn == HUMAN) {
      userWins++;
    } else {
      userDraws++;
    }
    fprintf(tempFile, "name: %s wins: %d loses: %d draws: %d win-rate: %.2f\n",
            name_tictactoe, userWins, userLosses, userDraws,
            (float)((float)userWins / (float)(userWins + userLosses)) * 100);
  }

  fclose(filePointer);
  fclose(tempFile);

  remove("Tictactoe.txt");
  rename("TempTictactoe.txt", "Tictactoe.txt");

}

// ---------------------------------- Leader for Tic-Tac-Toe -----------------------//
struct PlayerStats {
    char name[20];
    int wins;
    int losses;
    int draws;
    float winrate;
};

void leadTic() {
    struct PlayerStats players[20];
    int playerCount = 0;
    FILE *filePointer = fopen("Tictactoe.txt", "r");

    if (filePointer == NULL) {
        perror("Error opening file");
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), filePointer)) {
        if (sscanf(line, "name: %s wins: %d loses: %d draws: %d win-rate: %f",
                   players[playerCount].name, &players[playerCount].wins,
                   &players[playerCount].losses, &players[playerCount].draws,
                   &players[playerCount].winrate) == 5) {
            playerCount++;
        }
    }
    fclose(filePointer);

    for (int i = 0; i < playerCount - 1; i++) {
        for (int j = i + 1; j < playerCount; j++) {
            if (players[i].winrate < players[j].winrate) {
                struct PlayerStats temp = players[i];
                players[i] = players[j];
                players[j] = temp;
            }
        }
    }

    printf("\n--- Tic-Tac-Toe Leaderboard ---\n");
    printf("%-15s %-8s %-8s %-8s %-8s\n", "Name", "Wins", "Losses", "Draws", "Win Rate (%)");
    printf("--------------------------------------------------------\n");

    for (int i = 0; i < playerCount; i++) {
        printf("%-15s %-8d %-8d %-8d %-8.2f%%\n",
               players[i].name, players[i].wins, players[i].losses,
               players[i].draws, players[i].winrate);
    }
    printf("--------------------------------------------------------\n");
}


int rowCrossed(char board[][SIDE]) {
  for (int i = 0; i < SIDE; i++) {
    if (board[i][0] == board[i][1] && board[i][1] == board[i][2] &&
        board[i][0] != '_')
      return 1;
  }
  return 0;
}

int columnCrossed(char board[][SIDE]) {
  for (int i = 0; i < SIDE; i++) {
    if (board[0][i] == board[1][i] && board[1][i] == board[2][i] &&
        board[0][i] != '_')
      return 1;
  }
  return 0;
}

int diagonalCrossed(char board[][SIDE]) {
  if ((board[0][0] == board[1][1] && board[1][1] == board[2][2] &&
       board[0][0] != '_') ||
      (board[0][2] == board[1][1] && board[1][1] == board[2][0] &&
       board[0][2] != '_'))
    return 1;

  return 0;
}

int gameOver(char board[][SIDE]) {
  return (rowCrossed(board) || columnCrossed(board) || diagonalCrossed(board));
}

void playTicTacToe(int whoseTurn) {
  char board[SIDE][SIDE];
  int moves[SIDE * SIDE];
  printf("Enter your name: ");
  scanf("%s", name_tictactoe);
  printf("\n");
  initialise(board, moves);
  showInstructions();

  int moveIndex = 0, x, y;

  while (!gameOver(board) && moveIndex != SIDE * SIDE) {
    if (whoseTurn == COMPUTER) {
      struct Move thisMove = findBestMove(board);
      x = thisMove.row;
      y = thisMove.col;

      board[x][y] = COMPUTERMOVE;
      printf("COMPUTER has put a %c in cell %d %d\n", COMPUTERMOVE, x, y);
      showBoard(board);
      moveIndex++;
      whoseTurn = HUMAN;
    } else if (whoseTurn == HUMAN) {
      int move;
      printf("Enter your move (1-9): ");
      scanf("%d", &move);
      if (move < 1 || move > 9) {
        printf("Invalid input! Please enter a number between 1 and 9.\n");
        continue;
      }
      x = (move - 1) / SIDE;
      y = (move - 1) % SIDE;
      if (board[x][y] == '_') {
        board[x][y] = HUMANMOVE;
        showBoard(board);
        moveIndex++;
        if (gameOver(board)) {
          declareWinner(HUMAN);
          return;
        }
        whoseTurn = COMPUTER;
      } else {
        printf("Cell %d is already occupied. Try again.\n", move);
      }
    }
  }

  if (!gameOver(board) && moveIndex == SIDE * SIDE) {
    printf("It's a draw\n");
    whoseTurn = DRAWS;
    declareWinner(whoseTurn);
  } else {
    if (whoseTurn == COMPUTER)
      whoseTurn = HUMAN;
    else if (whoseTurn == HUMAN)
      whoseTurn = COMPUTER;

    declareWinner(whoseTurn);
  }
}
//---------------------------- Get - Leaderboard --------------------//
int getLead() {
    int choice;
    printf("Leaderboard Options:\n");
    printf("1. Tic-Tac-Toe Leaderboard\n");
    printf("2. Ping-Pong Leaderboard\n");
    printf("3. Search Tic-Tac-Toe Player\n");
    printf("4. Search Ping-Pong Player\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    char searchName[20];
    switch(choice) {
        case 1:
            leadTic();
            break;
        case 2:
            leadPong();
            break;
        case 3:
            printf("Enter player name to search: ");
            scanf("%s", searchName);
            searchTicLeaderboard(searchName);
            break;
        case 4:
            printf("Enter player name to search: ");
            scanf("%s", searchName);
            searchPongLeaderboard(searchName);
            break;
        default:
            printf("Invalid choice!\n");
    }

    return choice;
}
// ----------------------------------------------------------------//
// -------------------------Main-----------------------------------//
int main() {
  char choice;
  do {
    int gameChoice = getGameChoice();

    switch (gameChoice) {
    case 1:
      playTicTacToe(COMPUTER);
      break;
    case 2:
      playPong();
      break;
    case 3:
      getLead();
      break;
    default:
      printf("Invalid choice! Please enter a valid game option.\n");
      continue;
    }

    printf("\nDo you want to play another game? Enter 'Q' to quit or any other "
           "key to continue: ");
    choice = getchar();
    while (choice == '\n') {
      choice = getchar();
    }

  } while (choice != 'Q' && choice != 'q');

  printf("Thank you for playing!\n");
  return 0;
}