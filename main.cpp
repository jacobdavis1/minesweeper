#include <iostream>
#include <string>
#include <random>
#include <functional>
#include <cstdio>

using namespace std;

/* Minesweeper
    Player inputs a command and two numbers as their move. These are coordinates
    of a square to be revealed or flagged. If there is a mine revealed, the
    player loses. If there is not, the number of mines adjacent to the space is
    displayed in the space. If the number is 0, all connected 0s are revealed.
    The player wins if they clear every space that is not a mine and flag every
    space that is.

    Player moves:
    reveal(x,y)
    flag(x,y)
*/

class Space
{
public:
    Space()
    {
        reset();
    }

    void reset()
    {
        text = "-";
        mine = false;
        state = State::Hidden;
    }

    bool mine;
    std::string text;
    enum class State { Hidden, Revealed, Flagged } state;
};

class Board
{
public:
    enum class MoveReturnState { BadMove, Hidden, Flagged, Empty, Mine };

    Board(int nwidth = 8) : width(nwidth), height(nwidth)
    {
        board = new Space*[height];
        for (int i = 0; i < height; ++i)
        {
            board[i] = new Space[width];
        }

        // As a shortcut, mines are equal to width
        // All maps are squares;
        flags = width;
        mines = width;

        resetBoard();
    }

    ~Board()
    {
        for (int i = 0; i < height; ++i)
        {
            delete board[i];
        }

        delete board;
    }

    MoveReturnState attemptFlag(int x, int y)
    {
        // Flags the space if it is hidden,
        // unflags it if it is flagged.
        if (board[y][x].state == Space::State::Hidden && flags > 0)
        {
            board[y][x].state = Space::State::Flagged;
            board[y][x].text = "F";
            --flags;
            return MoveReturnState::Flagged;
        }
        else if (board[y][x].state == Space::State::Flagged)
        {
            board[y][x].state = Space::State::Hidden;
            board[y][x].text = "-";
            ++flags;
            return MoveReturnState::Hidden;
        }
        else return MoveReturnState::BadMove;
    }

    MoveReturnState attemptReveal(int x, int y)
    {
        switch (board[y][x].state)
        {
        case Space::State::Flagged:
            --flags;
            // Drops to next line.
        case Space::State::Hidden:
            if (board[y][x].mine)
            {
                board[y][x].state = Space::State::Revealed;
                return MoveReturnState::Mine;
            }
            else
            {
                revealConnected(x, y);
                return MoveReturnState::Empty;
            }
        default:
            return MoveReturnState::BadMove;
        }
    }

    void resetBoard()
    {
        for (int i = 0; i < width; ++i)
        {
            for (int j = 0; j < height; ++j)
            {
                board[i][j].reset();
            }
        }

        std::uniform_int_distribution<int> dist(0, width - 1);
        int unusedMines = mines;
        while (unusedMines > 0)
        {
            int x = dist(randGen);
            int y = dist(randGen);

            if (!board[y][x].mine)
            {
                board[y][x].mine = true;
                --unusedMines;
            }
        }
    }

    bool isValidSpace(int x, int y)
    {
        if (x >= 0 && x < width
            && y >= 0 && y < height)
            return true;

        return false;
    }

    int countAdjacentMines(int x, int y)
    {
        // Returns number of adjacent mines

        int totalMines = 0;

        for (int i = -1; i < 2; ++i)
        {
            for (int j = -1; j < 2; ++j)
            {
                if (isValidSpace(x + i, y + j))
                {
                    if (board[y + j][x + i].mine)
                        ++totalMines;
                }
            }
        }

        return totalMines;
    }

    void revealConnected(int x, int y)
    {
        // Reveal all connected empty spaces with no adjacent mines
        // Invalid spaces do nothing

        if(isValidSpace(x, y) && !board[y][x].mine
           && board[y][x].state != Space::State::Revealed)
        {
            int adjMines = countAdjacentMines(x, y);
            board[y][x].state = Space::State::Revealed;

            // Silly code to print a number to a string
            char buff[2];
            if (adjMines > 0)
            {
                sprintf(buff, "%d", adjMines);
                board[y][x].text = buff;
            }
            else
            {
                board[y][x].text = "0";
                for (int i = -1; i < 2; ++i)
                {
                    for (int j = -1; j < 2; ++j)
                    {
                        revealConnected(x + i, y + j);
                    }
                }
            }
        }
    }

    void printBoard()
    {
        // Print line of numbers at top for convenience
        cout << "    ";
        for (int w = 0; w < width; ++w)
            cout << w << " ";
        cout << endl;

        for (int i = 0; i < width; ++i)
        {
            cout << i << " < ";
            for (int j = 0; j < height; ++j)
            {
                cout << board[i][j].text << " ";
            }
            cout << ">" << endl;
        }

        cout << "Flags: " << flags << endl;
    }

    bool winCheck()
    {
        for (int i = 0; i < width; ++i)
        {
            for (int j = 0; j < height; ++j)
            {
                if (board[i][j].state != Space::State::Flagged
                    && board[i][j].state != Space::State::Revealed)
                    return false;
            }
        }

        return true;
    }

private:
    int width, height;
    int flags, mines;
    Space** board;

    std::default_random_engine randGen;
};

class AIPlayer;

bool isNumber(string s)
{
    for (int i = 0; i < s.length(); ++i)
    {
        if (!isdigit(s.at(i)))
            return false;
    }

    return true;
}

int main()
{
    Board board;

    while (true)
    {
        int x, y;
        string args[3];
        Board::MoveReturnState retVal = Board::MoveReturnState::BadMove;

        board.printBoard();

        while (retVal == Board::MoveReturnState::BadMove)
        {
            cout << "Type r x y for reveal, f x y for flag: ";
            cin >> args[0] >> args[1] >> args[2];

            if (isNumber(args[1]) && isNumber(args[2]))
            {
                x = stoi(args[1]);
                y = stoi(args[2]);

                switch (args[0].at(0))
                {
                case 'r':
                    retVal = board.attemptReveal(x, y);
                    break;

                case 'f':
                    retVal = board.attemptFlag(x, y);
                    break;

                default:
                    cout << "BadCommand: Please use r or f to designate your move." << endl;
                }
            }
            else
            {
                cout << "BadCommand: Please provide valid coordinates." << endl;
            }
        }

        if (retVal == Board::MoveReturnState::Mine)
        {
            cout << "It's a mine! You lose." << endl;
            board.resetBoard();
        }
        else if (board.winCheck())
        {
            cout << "You win!" << endl;
            board.resetBoard();
        }
    }

    return 0;
}
