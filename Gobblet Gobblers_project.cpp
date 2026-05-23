#include <iostream>
#include <vector>
#include <string>
#include <chrono>   // เก็บไว้สำหรับฟังก์ชัน Sleep เท่านั้น
#include <cstdlib>
#include <cctype>
#include <sstream>
#include <thread> 
#include <windows.h>

using namespace std;

// ===================== CONFIG & COLORS =====================
const string RESET  = "\033[0m";
const string BOLD   = "\033[1m";
const string RED    = "\033[31m";
const string BLUE   = "\033[34m";
const string YELLOW = "\033[33m";
const string GREEN  = "\033[32m";
const string CYAN   = "\033[36m";
const string MAGENTA = "\033[35m";

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

enum Size { NONE = 0, SMALL = 1, MEDIUM = 2, LARGE = 3 };

string sizeToString(Size s) {
    if (s == SMALL) return "S";
    if (s == MEDIUM) return "M";
    if (s == LARGE) return "L";
    return " ";
}

// ===================== CLASS PIECE =====================
class Piece {
private:
    string color;
    Size size;
    int coveredTurns;

public:
    Piece(string c = " ", Size s = NONE) : color(c), size(s), coveredTurns(0) {}
    
    Size getSize() const { return size; }
    string getColor() const { return color; }
    int getCoveredTurns() const { return coveredTurns; }
    void incrementCoveredTurn() { coveredTurns++; }
    void resetCoveredTurn() { coveredTurns = 0; }
    
    string getDisplay() const { 
        string cCode = (color == "X") ? RED : BLUE;
        return cCode + BOLD + color + "-" + sizeToString(size) + RESET;
    }
};

// ===================== CLASS CELL =====================
class Cell {
private:
    int x, y;
    vector<Piece> piecesLayer;

public:
    Cell(int x = 0, int y = 0) : x(x), y(y) {}
    bool hasPiece() const { return !piecesLayer.empty(); }
    
    Piece getTopPiece() const {
        if (hasPiece()) return piecesLayer.back();
        return Piece(" ", NONE);
    }
    
    bool pushPiece(const Piece& p) {
        if (!hasPiece() || p.getSize() > getTopPiece().getSize()) {
            piecesLayer.push_back(p);
            return true;
        }
        return false;
    }
    
    Piece popTopPiece() {
        if (hasPiece()) {
            Piece top = piecesLayer.back();
            piecesLayer.pop_back();
            if (hasPiece()) {
                piecesLayer.back().resetCoveredTurn();
            }
            return top;
        }
        return Piece(" ", NONE);
    }
    
    void updateCoveredPiecesTurn() {
        if (piecesLayer.size() > 1) {
            for (size_t i = 0; i < piecesLayer.size() - 1; ++i) {
                piecesLayer[i].incrementCoveredTurn();
            }
        }
    }
    
    void removeSuffocatedPieces() {
        if (piecesLayer.size() > 1) {
            auto it = piecesLayer.begin();
            while (it != piecesLayer.end() - 1) {
                if (it->getCoveredTurns() > 3) {
                    it = piecesLayer.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
};

// ===================== CLASS BOARD =====================
class Board {
private:
    Cell grid[3][3];

public:
    Board() {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                grid[i][j] = Cell(i, j);
    }

    void displayBoard() const {
        cout << CYAN << "\n           0         1         2    (แกน Y)" << RESET << endl;
        cout << CYAN << "      ┏━━━━━━━━━┳━━━━━━━━━┳━━━━━━━━━┓" << RESET << endl;
        for (int i = 0; i < 3; i++) {
            cout << CYAN << "  " << i << "   ┃" << RESET;
            for (int j = 0; j < 3; j++) {
                if (grid[i][j].hasPiece()) {
                    cout << "   " << grid[i][j].getTopPiece().getDisplay() << "   " << CYAN << "┃" << RESET;
                } else {
                    cout << "         " << CYAN << "┃" << RESET;
                }
            }
            if (i < 2)
                cout << "\n" << CYAN << "      ┣━━━━━━━━━╋━━━━━━━━━╋━━━━━━━━━┫" << RESET << endl;
        }
        cout << "\n" << CYAN << "      ┗━━━━━━━━━┻━━━━━━━━━┻━━━━━━━━━┛" << RESET << endl;
        cout << CYAN << " (แกน X)" << RESET << endl;
    }

    bool placePiece(const Piece& p, int x, int y) {
        if (x < 0 || x > 2 || y < 0 || y > 2) return false;
        return grid[x][y].pushPiece(p);
    }

    bool movePiece(int startX, int startY, int endX, int endY, string playerColor) {
        if (startX < 0 || startX > 2 || startY < 0 || startY > 2 ||
            endX < 0 || endX > 2 || endY < 0 || endY > 2) return false;

        if (!grid[startX][startY].hasPiece()) return false;
        if (grid[startX][startY].getTopPiece().getColor() != playerColor) return false;

        Piece movingPiece = grid[startX][startY].getTopPiece();

        if (grid[endX][endY].pushPiece(movingPiece)) {
            grid[startX][startY].popTopPiece();
            return true;
        }
        return false;
    }

    void processTurnEndRules() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                grid[i][j].updateCoveredPiecesTurn();
                grid[i][j].removeSuffocatedPieces();
            }
        }
    }

    string checkThreeInARow() const {
        for (int i = 0; i < 3; i++) {
            if (grid[i][0].hasPiece() && grid[i][1].hasPiece() && grid[i][2].hasPiece())
                if (grid[i][0].getTopPiece().getColor() == grid[i][1].getTopPiece().getColor() &&
                    grid[i][1].getTopPiece().getColor() == grid[i][2].getTopPiece().getColor())
                    return grid[i][0].getTopPiece().getColor();
            if (grid[0][i].hasPiece() && grid[1][i].hasPiece() && grid[2][i].hasPiece())
                if (grid[0][i].getTopPiece().getColor() == grid[1][i].getTopPiece().getColor() &&
                    grid[1][i].getTopPiece().getColor() == grid[2][i].getTopPiece().getColor())
                    return grid[0][i].getTopPiece().getColor();
        }
        if (grid[0][0].hasPiece() && grid[1][1].hasPiece() && grid[2][2].hasPiece())
            if (grid[0][0].getTopPiece().getColor() == grid[1][1].getTopPiece().getColor() &&
                grid[1][1].getTopPiece().getColor() == grid[2][2].getTopPiece().getColor())
                return grid[0][0].getTopPiece().getColor();
        if (grid[0][2].hasPiece() && grid[1][1].hasPiece() && grid[2][0].hasPiece())
            if (grid[0][2].getTopPiece().getColor() == grid[1][1].getTopPiece().getColor() &&
                grid[1][1].getTopPiece().getColor() == grid[2][0].getTopPiece().getColor())
                return grid[0][2].getTopPiece().getColor();
        return "NONE";
    }
};

// ===================== CLASS PLAYER =====================
class Player {
private:
    string color;
    int countS, countM, countL;

public:
    Player(string c) : color(c), countS(2), countM(2), countL(2) {}
    string getColor() const { return color; }

    void displayHand() const {
        string cCode = (color == "X") ? RED : BLUE;
        cout << "  🎒 หมากในมือ [" << cCode << BOLD << color << RESET << "]: ";
        cout << "S=" << YELLOW << countS << RESET << " | ";
        cout << "M=" << YELLOW << countM << RESET << " | ";
        cout << "L=" << YELLOW << countL << RESET << "\n";
    }

    bool hasPiecesInHand() const {
        return (countS > 0 || countM > 0 || countL > 0);
    }

    bool playNewPiece(string sizeStr, Board& board, int x, int y) {
        Size s = NONE;
        if (sizeStr == "S" && countS > 0) s = SMALL;
        else if (sizeStr == "M" && countM > 0) s = MEDIUM;
        else if (sizeStr == "L" && countL > 0) s = LARGE;

        if (s == NONE) return false;

        Piece p(color, s);
        if (board.placePiece(p, x, y)) {
            if (s == SMALL) countS--;
            else if (s == MEDIUM) countM--;
            else if (s == LARGE) countL--;
            return true;
        }
        return false;
    }
};

// ===================== CLASS GAME MANAGER =====================
class GameManager {
private:
    Board board;
    Player p1, p2;
    Player* currentPlayer;

    void getCoordinates(string promptMsg, int& r, int& c) {
        while (true) {
            cout << CYAN << promptMsg << RESET;
            string pos;
            if (!(cin >> pos)) exit(0);
            if (pos.length() >= 2 && isdigit(pos[0]) && isdigit(pos[1])) {
                r = pos[0] - '0';
                c = pos[1] - '0';
                if (r >= 0 && r <= 2 && c >= 0 && c <= 2) return;
            }
            cout << RED << "  ⚠️ พิมพ์ผิด! ต้องเป็นเลข 0-2 สองตัวติดกัน (เช่น 11)\n" << RESET;
        }
    }

public:
    GameManager() : p1("X"), p2("O") { currentPlayer = &p1; }

    void switchTurn() { currentPlayer = (currentPlayer == &p1) ? &p2 : &p1; }

    void startGame() {
        while (true) {
            clearScreen();
            cout << MAGENTA << "╔═══════════════════════════════════════════════════╗" << endl;
            cout << "║            🎮 XO GOBBLE SPECIAL Edition 🎮        ║" << endl;
            cout << "╚═══════════════════════════════════════════════════╝" << RESET << endl;
            
            board.displayBoard();
            
            cout << "\n─────────────────────────────────────────────────────" << endl;
            string cCode = (currentPlayer->getColor() == "X") ? RED : BLUE;
            cout << " ➤ ตาของ: Player [" << cCode << BOLD << currentPlayer->getColor() << RESET << "]" << endl;
            currentPlayer->displayHand();
            cout << "─────────────────────────────────────────────────────" << endl;

            bool validMove = false;
            string choice;

            if (!currentPlayer->hasPiecesInHand()) {
                cout << YELLOW << " ⚠️ หมากหมดมือ! ระบบบังคับให้ [2] ย้ายหมาก" << RESET << endl;
                choice = "2";
            } else {
                cout << " เลือกการกระทำ: [" << GREEN << "1" << RESET << "] ลงใหม่  [" << GREEN << "2" << RESET << "] ย้ายหมาก\n";
                while (true) {
                    cout << " >> ";
                    if (!(cin >> choice)) exit(0);
                    if (choice == "1" || choice == "2") break;
                    cout << RED << " ❌ ใส่แค่ 1 หรือ 2 นะครับ" << RESET << endl;
                }
            }

            if (choice == "1") {
                string pSize;
                cout << " ระบุขนาด (" << YELLOW << "S, M, L" << RESET << "): ";
                cin >> pSize;
                pSize[0] = toupper(pSize[0]);
                int x, y;
                getCoordinates(" ลงที่ไหน? (เช่น 00): ", x, y);
                validMove = currentPlayer->playNewPiece(pSize, board, x, y);
            } else {
                int sx, sy, ex, ey;
                getCoordinates(" หยิบจาก (เช่น 00): ", sx, sy);
                getCoordinates(" ย้ายไปที่ (เช่น 11): ", ex, ey);
                validMove = board.movePiece(sx, sy, ex, ey, currentPlayer->getColor());
            }

            if (!validMove) {
                cout << RED << "\n ❌ เดินไม่ได้! (ลองใหม่อีกครั้ง)" << RESET << endl;
                this_thread::sleep_for(chrono::seconds(1));
                continue; 
            } else {
                cout << GREEN << "\n ✅ สำเร็จ!" << RESET << endl;
                this_thread::sleep_for(chrono::milliseconds(500));
            }

            board.processTurnEndRules();
            string winner = board.checkThreeInARow();
            if (winner != "NONE") {
                clearScreen();
                board.displayBoard();
                cout << GREEN << BOLD << "\n 🏆 ผู้ชนะคือ: Player [" << winner << "] ยินดีด้วย!! 🏆\n\n" << RESET;
                break;
            }
            switchTurn();
        }
    }
};

int main() {
    SetConsoleOutputCP(CP_UTF8); // ตั้งค่าให้แสดงผลเป็น UTF-8
    SetConsoleCP(CP_UTF8);       // ตั้งค่าให้รับข้อมูลพิมพ์เป็น UTF-8

    GameManager game;
    game.startGame();
    return 0;
}
    

