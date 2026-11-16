
#include <iostream>
#include <array>
#include <vector>
#include <memory>
#include <fstream>
#include <random>
#include <limits>
#include <string>
#include <stdexcept>

using namespace std;

// ==========================================
// CLASS 1: BOARD
// ==========================================

class Board {
private:
    array<char, 9> cells;

public:
    Board() {
        cells.fill(' ');
    }

    void display() const {
        cout << "\n";
        cout << "       TIC TAC TOE\n\n";

        for (int i = 0; i < 9; i++) {
            cout << "   " << (cells[i] == ' ' ? char('1' + i) : cells[i]);

            if (i % 3 != 2)
                cout << "  |";
            else if (i != 8)
                cout << "\n   ---+---+---\n";
        }

        cout << "\n\n";
    }

    bool isEmpty(int position) const {
        return position >= 0 && position < 9
               && cells[position] == ' ';
    }

    void place(int position, char symbol) {
        if (position < 0 || position >= 9)
            throw out_of_range("Invalid position!");

        if (!isEmpty(position))
            throw invalid_argument("Position already occupied!");

        cells[position] = symbol;
    }

    bool isFull() const {
        for (char cell : cells) {
            if (cell == ' ')
                return false;
        }
        return true;
    }

    bool hasWon(char symbol) const {
        const int wins[8][3] = {
            {0, 1, 2},
            {3, 4, 5},
            {6, 7, 8},
            {0, 3, 6},
            {1, 4, 7},
            {2, 5, 8},
            {0, 4, 8},
            {2, 4, 6}
        };

        for (const auto& line : wins) {
            if (cells[line[0]] == symbol &&
                cells[line[1]] == symbol &&
                cells[line[2]] == symbol) {
                return true;
            }
        }

        return false;
    }

    vector<int> availableMoves() const {
        vector<int> moves;

        for (int i = 0; i < 9; i++) {
            if (cells[i] == ' ')
                moves.push_back(i);
        }

        return moves;
    }
};


// ==========================================
// CLASS 2: ABSTRACT PLAYER
// ==========================================

class Player {
protected:
    string name;
    char symbol;

public:
    Player(string n, char s)
        : name(n), symbol(s) {}

    virtual ~Player() = default;

    string getName() const {
        return name;
    }

    char getSymbol() const {
        return symbol;
    }

    // Pure virtual function
    virtual int getMove(const Board& board) = 0;
};


// ==========================================
// CLASS 3: HUMAN PLAYER
// ==========================================

class HumanPlayer : public Player {
public:
    HumanPlayer(string n, char s)
        : Player(n, s) {}

    int getMove(const Board& board) override {
        int position;

        while (true) {
            cout << name << " (" << symbol
                 << "), enter position (1-9): ";

            if (!(cin >> position)) {
                cout << "Please enter a valid number.\n";

                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            position--;

            if (board.isEmpty(position))
                return position;

            cout << "Invalid or occupied position. Try again.\n";
        }
    }
};


// ==========================================
// CLASS 4: COMPUTER PLAYER
// ==========================================

class ComputerPlayer : public Player {
private:
    mt19937 rng;

public:
    ComputerPlayer(string n, char s)
        : Player(n, s), rng(random_device{}()) {}

    int getMove(const Board& board) override {
        vector<int> moves = board.availableMoves();

        uniform_int_distribution<int> dist(
            0, static_cast<int>(moves.size()) - 1
        );

        int position = moves[dist(rng)];

        cout << "Computer chooses position "
             << position + 1 << endl;

        return position;
    }
};


// ==========================================
// CLASS 5: GAME STATISTICS
// ==========================================

class Statistics {
private:
    int xWins = 0;
    int oWins = 0;
    int draws = 0;

public:
    void load() {
        ifstream file("game_stats.txt");

        if (file.is_open()) {
            file >> xWins >> oWins >> draws;
        }
    }

    void record(char winner) {
        if (winner == 'X')
            xWins++;
        else if (winner == 'O')
            oWins++;
        else
            draws++;
    }

    void save() const {
        ofstream file("game_stats.txt");

        if (file.is_open()) {
            file << xWins << " "
                 << oWins << " "
                 << draws << endl;
        } else {
            cout << "Warning: Could not save statistics.\n";
        }
    }

    void display() const {
        cout << "\n===== GAME STATISTICS =====\n";
        cout << "X Wins : " << xWins << endl;
        cout << "O Wins : " << oWins << endl;
        cout << "Draws  : " << draws << endl;
        cout << "===========================\n";
    }
};


// ==========================================
// CLASS 6: GAME
// ==========================================

class Game {
private:
    Board board;

    unique_ptr<Player> player1;
    unique_ptr<Player> player2;

    Statistics& stats;

public:
    Game(unique_ptr<Player> p1,
         unique_ptr<Player> p2,
         Statistics& gameStats)
        : player1(move(p1)),
          player2(move(p2)),
          stats(gameStats) {}

    void play() {
        bool xTurn = true;

        cout << "\nGame Started!\n";

        while (true) {
            board.display();

            Player& current =
                xTurn ? *player1 : *player2;

            cout << "Turn: " << current.getName() << endl;

            int position = current.getMove(board);

            try {
                board.place(position, current.getSymbol());
            }
            catch (const exception& e) {
                cout << "Error: " << e.what() << endl;
                continue;
            }

            if (board.hasWon(current.getSymbol())) {
                board.display();

                cout << "\nCongratulations! "
                     << current.getName()
                     << " wins!\n";

                stats.record(current.getSymbol());
                break;
            }

            if (board.isFull()) {
                board.display();

                cout << "\nGame Draw!\n";

                stats.record('D');
                break;
            }

            xTurn = !xTurn;
        }

        stats.save();
    }
};


// ==========================================
// MAIN FUNCTION
// ==========================================

int main() {
    Statistics stats;
    stats.load();

    int choice;

    while (true) {
        cout << "\n================================\n";
        cout << "       TIC TAC TOE GAME\n";
        cout << "================================\n";
        cout << "1. Human vs Human\n";
        cout << "2. Human vs Computer\n";
        cout << "3. View Statistics\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            cout << "Please enter a valid number.\n";

            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            auto p1 = make_unique<HumanPlayer>(
                "Player 1", 'X'
            );

            auto p2 = make_unique<HumanPlayer>(
                "Player 2", 'O'
            );

            Game game(move(p1), move(p2), stats);
            game.play();
        }

        else if (choice == 2) {
            auto p1 = make_unique<HumanPlayer>(
                "You", 'X'
            );

            auto p2 = make_unique<ComputerPlayer>(
                "Computer", 'O'
            );

            Game game(move(p1), move(p2), stats);
            game.play();
        }

        else if (choice == 3) {
            stats.display();
        }

        else if (choice == 4) {
            cout << "\nThank you for playing!\n";
            break;
        }

        else {
            cout << "Invalid choice! Select 1-4.\n";
        }
    }

    return 0;
}
