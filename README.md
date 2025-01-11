# 8-bit Arcade Game

This C program is an 8-bit arcade game that includes two classic games: Tic-Tac-Toe and Pong. The program allows users to play these games against the computer or another player, and it maintains leaderboards for both games.

## Features

1. **Tic-Tac-Toe**:
   - Play against the computer.
   - The computer uses the minimax algorithm to determine the best move.
   - The game keeps track of wins, losses, and draws for each player.
   - The leaderboard displays the top players based on their win rates.

2. **Pong**:
   - Two-player game where each player controls a paddle.
   - The game keeps track of scores and updates the leaderboard.
   - The leaderboard displays the top players based on their win rates.

3. **Leaderboards**:
   - Separate leaderboards for Tic-Tac-Toe and Pong.
   - Players' statistics are saved to files (`Tictactoe.txt` and `Pingpong.txt`).
   - Players can search for their statistics in the leaderboards.

## Prerequisites

- A C compiler (e.g., `gcc`).
- A terminal that supports ANSI escape codes.

## How to Compile and Run

1. **Clone the repository** (if applicable):
   ```sh
   git clone https://github.com/Ohmmykung09/8-bitArcadeGame.git
   cd 8-bitArcadeGame
