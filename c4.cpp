#include <iostream>
#include <random>
#include <ctime>
#include <cstdlib>
#include <tuple>
#include <vector>
#include <chrono>

using namespace std;

// ConnectFour command line game.
// Implemented AI with pure Monte-Carlo Tree Search
// CMPT 310 SFU

random_device dev;
const char EMPTY = '_';
const int numOfRows = 6;
const int numOfCols = 7;
const int RANDOM = 1;
const int HEURISTIC = 2;
int playouts_per_sec = 0;
class ConnectFour {
    private:
        char playerX = 'X'; // bot
        char playerO = 'O';
        char currPlayer;
        bool gameOver;
        char winner; 
        int plays; // to track if board is full
        char board[numOfRows][numOfCols];
        int playouts = 1500;
        double times_h;
        double times_r;
        double times_h_n;
        double times_r_n;
        // moves player to position (0-numOfRows)
        // returns 0 for fail - default
        // returns 1 for success
        int move(char player, int col) {
            // out of bounds
            if (col < 0 || col > numOfRows) {
                cout << "wrong col: " << col << endl;
                return 0;
            } else if (board[0][col] != '_') {
                //cout << "col: " << col << " full" << endl;
                return 0;
            } else {
                for (int row = 5; row >= 0; row--) {
                    if (board[row][col] == '_') {
                        board[row][col] = player;
                        //cout << "row: " << row << " col: " << col << endl;
                        ++plays;
                        winner = checkWinner(player, row, col);
                        if (winner != '\0') {
                            //cout << "winner: " << winner << endl;
                            gameOver = true;
                        }
                        return 1;
                    }
                    
                }
            }
            cout << "move failed" << endl;
            return 0;
        }

        // removes from a column
        void cancelMove(int col) { 
            for (int row = 0; row < 6; row++) {
                if (board[row][col] != '_') {
                    board[row][col] = '_';
                    plays = plays - 1;
                    if (gameOver) {
                        gameOver = false;
                    }
                    return;
                }
            }
        }

        // checks if the move results in a win
        char checkWinner(char player, int row_, int col_) {
            // row
            for (int col = 0; col < numOfCols-3; col++) {
                if (board[row_][col] == player && board[row_][col+1] == player && board[row_][col+2] == player && board[row_][col+3] == player) {
                    return player;
                }
            }

            // col
            for (int row = 5; row >= 3; row--) {
                if (board[row][col_] == player && board[row-1][col_] == player && board[row-2][col_] == player && board[row-3][col_] == player) {
                    return player;
                }
            }

            // \-diagonal
            for (int row = 3; row < numOfRows; row++) {
                for (int col = 3; col < numOfCols; col++) {
                    if (board[row][col] == player && board[row-1][col-1] == player && board[row-2][col-2] == player && board[row-3][col-3] == player) {
                        //cout << "z diagonal win. Player: " << player << endl;
                        return player;
                    }
                }
            }

            // /-diagonal
            for (int row = 3; row < numOfRows; row++) {
                for (int col = 0; col < 4; col++) {
                    if (board[row][col] == player && board[row-1][col+1] == player && board[row-2][col+2] == player && board[row-3][col+3] == player) {
                        //cout << "x diagonal win. Player: " << player << endl;
                        return player;
                    }
                }
            }
            
            // draw
            if (plays == numOfRows*numOfCols) {
                return 'D';
            }
            // no winner
            return '\0';
        }

        // scores each group (all combinations of four)
        // returns the sum of the score of all groups
        int getScore(char player) {
            int score = 0;
            // horizontal
            for (int row = 0; row < numOfRows; row++) {
                for (int col = 0; col < numOfCols-3; col++) {
                    char fourGroup[] = {board[row][col], board[row][col+1], board[row][col+2], board[row][col+3]};
                    score += groupScore(fourGroup, player);
                }
            }

            // vertical
            for (int col = 0; col < numOfCols; col++ ) {
                for (int row = 0; row < numOfRows-3; row++) {
                    char fourGroup[] = {board[row][col], board[row+1][col], board[row+2][col], board[row+3][col]};
                    score += groupScore(fourGroup, player);
                }
            }

            //\-diagonal
            for (int row = 3; row < numOfRows; row++) {
                for (int col = 3; col < numOfCols; col++) {
                    char fourGroup[] = {board[row][col], board[row-1][col-1], board[row-2][col-2], board[row-3][col-3]};
                    score += groupScore(fourGroup, player);
                }
            }

            // /-diagonal
            for (int row = 3; row < numOfRows; row++) {
                for (int col = 0; col < 4; col++) {
                    char fourGroup[] = {board[row][col], board[row-1][col+1], board[row-2][col+2], board[row-3][col+3]};
                    score += groupScore(fourGroup, player);
                }
            }
            return score;
        }

        // group size = 4
        // evaluates group with a score
        int groupScore(char group[], char player) {
            // for (int i = 0; i < 4; i++) {
            //     cout << group[i] << " ";
            // }
            // cout << endl;
            char opposition;
            if (player == playerX) {
                opposition = playerO;
            } else {
                opposition = playerX;
            }
            int score = 0;
            int goodCount = 0;
            int nullCount = 0;
            int badCount = 0;
            for (int i =0 ; i < 4; i++) {
                if (group[i] == player) goodCount++;
                if (group[i] == EMPTY) nullCount++;
                if (group[i] == opposition) badCount++;
            }
            //if (badCount == 4) score -= 100000000;
            // prevent XXX-
            if (badCount == 3 && nullCount == 1) score -= 1000;
            if (goodCount == 4) score += 1000;
            if (goodCount == 3 && nullCount == 1) score += 5;
            if (goodCount == 2 && nullCount == 2) score += 3;
            
            return score;
        }   
        

        // Creates a new ConnectFour game with current board to simulate play throughs
        // Makes random turns
        // uses regular random pMCTS
        // returns the player that won (char)
        char randomPlay(ConnectFour currGame, int move) {
            // 1 game simulation 
            ConnectFour sim = ConnectFour(currGame.getBoard());
            // make initial move for AI
            char simPlayer = currPlayer;
            sim.play(simPlayer, move);
            if (simPlayer == playerX) {
                simPlayer = playerO;
            } else {
                simPlayer = playerX;
            }
  
            // randomly play until game is over
            while (!sim.isOver()) {
                // determine columns that are still open 
                vector<int> legalMoves = sim.getLegalMoves();

                // pick random move from legalMoves
                mt19937 rng(dev());
                uniform_int_distribution<int> pick(0, legalMoves.size()-1);
                int pickCol = pick(rng);
                
                //cout << "pickar: " << pickar << endl;
                //cout << "curr player: " << currPlayer << endl;
                sim.play(simPlayer, legalMoves[pickCol]);
                if (simPlayer == playerO) {
                    simPlayer = playerX;
                } else {
                    simPlayer = playerO;
                }
            }
            //cout << "winner: " << sim.getWinner() << endl;
            return sim.getWinner();
        }

        // Creates a new ConnectFour game with current board to simulate one play through
        // Makes moves based on score function (getScore)
        // returns the player that won (char)
        int hPlay(ConnectFour currGame, int move) {
            //int depth = 3;
            // 1 game simulation 
            ConnectFour sim = ConnectFour(currGame.getBoard());
            // make initial move for AI
            char simPlayer = currGame.getCurrPlayer();
            sim.play(simPlayer, move);
            if (simPlayer == playerX) {
                simPlayer = playerO;
            } else {
                simPlayer = playerX;
            }
  
            // randomly play until game is over
            int score = sim.score(currPlayer);
            if (score < 0) {
                return simPlayer;
            }
            while (!sim.isOver()) {
                //sim.show();
                // determine columns that are still open 
                vector<int> legalMoves = sim.getLegalMoves();
                int legalMovesSize = legalMoves.size();
                int scores[legalMovesSize] = {0};
                for (int i = 0 ; i < legalMovesSize; i++) {
                    int col = legalMoves[i];
                    sim.play(simPlayer, col);
                    scores[i] = sim.getScore(simPlayer);
                    sim.cancelMove(col);
                    if (legalMoves[i] == 3) {
                        scores[i] += 5;
                    }
                    if (legalMoves[i] == 0 || legalMoves[i] == 6) {
                        scores[i] += 2;
                    }
                }

                int times = 0;
                for (int i = 0; i < legalMovesSize; i++) {
                    if (scores[i] < 0) {
                        times++;
                    }
                }
                if (times == legalMovesSize) {
                    if (simPlayer == playerX) {
                        return playerO;
                    } else {
                        return playerX;
                    }
                }
                // find column with the max score
                int max = 0;
                for (int i = 1; i < legalMovesSize; i++) {
                    if (scores[max] > scores[i]) {
                        max = i;
                    } else if (scores[max] == scores[i]) {
                        mt19937 rng(dev());
                        uniform_int_distribution<int> flip(0, 1);
                        int coin = flip(rng);
                        if (coin) {
                            max = i;
                        } else {
                            continue;
                        }
                    }
                }
                //cout << "max: " << max << endl;
                int pickCol = legalMoves[max];

                // cout << "playing: " << pickCol << endl;

                // make move with the column picked
                sim.play(simPlayer, pickCol);
                if (simPlayer == playerO) {
                    simPlayer = playerX;
                } else {
                    simPlayer = playerO;
                }
            }
            //sim.show();
            return sim.getWinner();
        }

        // Initializes pMCTS for heuristic AI
        // Sets number of playthroughs
        // Does play throughs for all the available moves
        // Gets the statistics of the playthroughs 
        // returns the "best" column based on measured statistics
        // formula  win * 3 + loss * (-3) 
        int botMoveH() {
            vector<int> legalMoves = getLegalMoves();

            vector<int> results(3, 0);
            vector<vector<int> > stats(legalMoves.size(), results);

            for (int i = 0; i < legalMoves.size(); i++) {
                // start timer to determine how long each playout is
                auto start = chrono::high_resolution_clock::now();
                for (int play = 0; play < playouts; play++) {
                    auto start_5s = chrono::high_resolution_clock::now();
                    winner = hPlay(*this, legalMoves[i]);
                    auto finish_5s = chrono::high_resolution_clock::now();
                    chrono::duration<double> elapsed_5s = finish_5s - start_5s;
                    if (elapsed_5s.count() > 5) {
                        return -1;
                    }
                    // scores[i] = hPlay(*this, legalMoves[i]);
                    if (winner == playerX) { // lose
                        stats[i][2]++;
                    } else if (winner == playerO) {
                        stats[i][0]++;
                    } else {
                        stats[i][1]++;
                    }
                }

                // save execution time for 1 playout
                auto finish = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = finish - start;
                times_h += elapsed.count();
                times_h_n++;
            }

            // find column with the best win/loss statistic 
            // ranking = win*3 + loss*(-3)
            // find column with the greatest goodness
            int ranking[legalMoves.size()] = {0};
            for (int i = 0; i < legalMoves.size(); i++) {
                ranking[i] = ((stats[i][0]*3) + (stats[i][2]*(-3)));
            };

            // best col = the 
            int best_col_index = 0;   
            for (int i = 1; i < legalMoves.size(); i++) {
                if (ranking[i] > ranking[best_col_index]) {
                    best_col_index = i;
                }
            }
            for (int i = 0; i < legalMoves.size(); i++) {
                cout << "LegalMoves: " << legalMoves[i] << " win: " << stats[i][0] << " draw: " << stats[i][1] << " loss: " << stats[i][2] << " ranking: " << ranking[i] << endl;
            }

            return legalMoves[best_col_index];
        
        }

        // finds best move for bot through pMCTS
        // does a number of random play throughs 
        // returns best move based on move with minimum number of losses
        int botMove() {
            // determine columns that are still open 
            vector<int> legalMoves = getLegalMoves();

            vector<int> results(3, 0);
            vector<vector<int> > stats(legalMoves.size(), results);

            for (int i = 0; i < legalMoves.size(); i++) {
                // start timer to determine how long each playout is
                auto start = chrono::high_resolution_clock::now();
                for (int play = 0; play < playouts; play++) {
                    // checking if its been 5 seconds...

                    //cout << "play: " << play << " for col: " << legalMoves[i] << endl;
                    auto start_5s = chrono::high_resolution_clock::now();
                    winner = randomPlay(*this, legalMoves[i]);
                    auto finish_5s = chrono::high_resolution_clock::now();
                    chrono::duration<double> elapsed_5s = finish_5s - start_5s;
                    if (elapsed_5s.count() > 5) {
                        return -1;
                    }
                    //cout << "winner: " << winner << endl;
                    if (winner == playerO) { // lose
                        stats[i][2]++;
                    } else if (winner == playerX) {
                        stats[i][0]++;
                    } else {
                        stats[i][1]++;
                    }
                }
                // save execution time for 1 playout
                auto finish = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = finish - start;
                times_r += elapsed.count();
                times_r_n++;
            }

            int best_col = 0;
            for (int i = 1; i < legalMoves.size(); i++) {
                if (stats[i][2] < stats[best_col][2]) {
                    best_col = i;
                }
            }
            for (int i = 0; i < legalMoves.size(); i++) {
                cout << "LegalMoves: " << legalMoves[i] << " win: " << stats[i][0] << " draw: " << stats[i][1] << " loss: " << stats[i][2] << endl;
            }
            return legalMoves[best_col];
        }

        vector<char> arrToVec(char arr[], int n) {
            vector<char> vec;
            vec.reserve(n);
            for (int i = 0; i < n; i++) {
                vec.push_back(arr[i]);
            }
            return vec;
        }
       

    public:
        ConnectFour() {
            gameOver = false;
            winner = '\0';
            plays = 0;
            times_h_n = 0;
            times_r_n = 0;
            for (int row = 0; row < numOfRows; row++) {
                for (int col = 0; col < numOfCols; col++) {
                    board[row][col] = '_';
                }
            }
        }

        ConnectFour(vector<vector<int> > board) {
            gameOver = false;
            winner = '\0';
            plays = 0;

            // copy board 
            for (int row = 0; row < numOfRows; row++) {
                for (int col = 0; col < numOfCols; col++) {
                    this->board[row][col] = board[row][col];
                    if (board[row][col] != '_') {
                        plays++;
                    }
                }
            }

        }

        // prints board to command line
        void show() {
            for (int col = 0; col < numOfCols; col++) {
                cout << col << ' ';
            }
            cout << "\n";
            for (int row = 0; row < numOfRows; row++) {
                for (int col = 0; col < numOfCols; col++) {
                    cout << board[row][col] << ' ';
                }
                cout << "\n";
            }
            for (int col = 0; col < numOfCols; col++) {
                cout << "==";
            }
            cout << endl;
            return;
        }

        // returns the current player's turn
        char getCurrPlayer() {
            return currPlayer;
        }

        // start game
        // plays regular pMCTS AI with my heursitic AI.
        // starts the game, loop turns until game is over
        void start() {
            // first player is 'X'
            int turns = 0;
            currPlayer = playerO;
            while (!gameOver) {
                turns++;
                show();
                cout << "curr player: " << currPlayer << endl;
                if (currPlayer == playerO) {
                    cout << "hbot picking: " << endl;
                    int pick = botMoveH();
                    cout << "hbot pick: " << pick << endl;
                    if (pick == -1) {
                        cout << "timed out trying again" << endl;
                    } else if (move(currPlayer, pick)) {
                        cout << "hPlayer: " << currPlayer << " has made a move: " << pick << endl;
                        currPlayer = playerX;
                    } else {
                        cout << "hbot failed pick: " << pick << endl;
                    }
                } else {
                    cout << "bot picking: " << endl;
                    int pick = botMove();
                    cout << "bot pick: " << pick << endl;
                    if (pick == -1) {
                        cout << "timed out trying again" << endl;
                    } else if (move(currPlayer, pick)) {
                        cout << "Player: " << currPlayer << " has made a move: " << pick << endl;
                        currPlayer = playerO;
                    } else {
                        cout << "bot failed pick: " << pick << endl;
                    }
                }
            }

            show();
            cout << "Game over! Winner: " << winner << endl;
        }

        // start game
        // plays regular pMCTS AI with my heursitic AI.
        // starts the game, loop turns until game is over
        void start2(int type) {
            cout << "You are player O" << endl;
            int turns = 0;
            currPlayer = playerO;
            while (!gameOver) {
                turns++;
                show();
                cout << "curr player: " << currPlayer << endl;
                if (currPlayer == playerO) {
                    int pick;
                    cout << "pick a column: " << endl;
                    cin >> pick;
                    if (move(currPlayer, pick)) {
                        cout << "Player: " << currPlayer << " has made a move: " << pick << endl;
                        currPlayer = playerX;
                    } else {
                        cout << "player O failed pick: " << pick << endl;
                    }
                } else {
                    cout << "bot picking: " << endl;
                    int pick;
                    if (type == 0) {
                        pick = botMove();
                    } else {
                        pick = botMoveH();
                    }
                    cout << "bot pick: " << pick << endl;
                    if (pick == -1) {
                        cout << "timed out trying again" << endl;
                    } else if (move(currPlayer, pick)) {
                        cout << "Player: " << currPlayer << " has made a move: " << pick << endl;
                        currPlayer = playerO;
                    } else {
                        cout << "bot failed pick: " << pick << endl;
                    }
                }
            }

            show();
            cout << "Game over! Winner: " << winner << endl;
        }

        // makes 1 move
        void play(char player, int colPick) {
            move(player, colPick);
        }

        // takes a piece off the board
        void cancelMove(char player, int colPick) {
            cancelMove(player, colPick);
        }

        // returns current score of board for this player
        int score(char player) {
            return getScore(player);
        }

        // average playout time for heuristic AI
        double playout_time_h() {
            return times_h / times_h_n;
        }

        // average playout time for random AI
        double playout_time_r() {
            return times_r / times_r_n;
        }

        vector<int> getLegalMoves() {
            vector<int> legalMoves;
                for (int col = 0; col < numOfCols; col++) {
                    if (board[0][col] == '_') {
                        legalMoves.push_back(col);
                    }
                }
            return legalMoves;
        }

        // returns game board as 2d vector
        vector<vector<int> > getBoard() {
            vector<int> vv(numOfCols,'_');
            vector<vector<int> > boardVect(numOfRows, vv);
            for (int row = 0; row < numOfRows; row++) {
                for (int col = 0; col < numOfCols; col++) {
                    boardVect[row][col] = board[row][col];
                }
            }
            return boardVect;
        }

        char getWinner() {
            return winner;
        }

        bool isOver() {
            return gameOver;
        }

        int turns() {
            return plays;
        }

};


int main() {

    // uncomment to play against the AI
    // n: 0 for random, 1 for heuristic

    // ConnectFour game = ConnectFour();
    // game.start2(n)

    int stats[4] = {0,0,0};
    ConnectFour games[20] = {ConnectFour()};
    for (int i = 0; i < 2; i++) {
        games[i].start();
        if (games[i].getWinner() == 'X') {
            stats[0]++;
        } else if (games[i].getWinner() == 'O') {
            stats[1]++;
        } else {
            stats[2]++;
        }
        stats[3] = games[i].turns();
        cout << games[i].playout_time_h() << "\n";
        cout << games[i].playout_time_r() << "\n";

    } 
    
    cout << "x win: " << stats[0] << " o win: " << stats[1] << " d: " << stats[2] << " turns: " << stats[3] << endl;
    for (int i = 0; i < 20; i++) {
        if (games[i].getWinner() == 'O') {
            games[i].show();
        }
    }


}