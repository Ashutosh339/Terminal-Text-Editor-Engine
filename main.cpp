/* * Minor Project: Terminal Text Editor Engine
 * Description: Final Master Code (Find & Replace added via Ctrl+R)
 */

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <vector>
#include <string>
#include <fstream>
#include <stack>

using namespace std;

// ==========================================
// --- STATE MACHINE & GLOBALS ---
// ==========================================
enum Mode { NORMAL, COMMAND, SEARCH, REPLACE_TARGET, REPLACE_WITH };
Mode currentMode = NORMAL;

std::string commandPrompt = "";
std::string searchPrompt = "";
std::string replaceTargetPrompt = "";
std::string replaceWithPrompt = "";

struct Match { int x; int y; };
std::vector<Match> searchMatches;
std::string match_word = "";

// ==========================================
// --- COMMAND PATTERN (UNDO / REDO) ---
// ==========================================
enum ActionType { TYPE_CHAR, DELETE_CHAR };
struct Action { ActionType type; char c; int x; int y; };

std::stack<Action> undoStack;
std::stack<Action> redoStack;

void clearRedoStack() { while (!redoStack.empty()) redoStack.pop(); }
void clearUndoStack() { while (!undoStack.empty()) undoStack.pop(); }

// ==========================================
// --- CLASS: THE GAP BUFFER ---
// ==========================================
class GapBuffer {
private:
    char* buffer; int gap_left; int gap_right; int total_size;

    void expandGap() {
        int new_size = total_size * 2;
        char* new_buffer = new char[new_size];
        for (int i = 0; i < gap_left; i++) new_buffer[i] = buffer[i];

        int right_text_size = total_size - 1 - gap_right;
        int new_gap_right = new_size - 1 - right_text_size;
        for (int i = 0; i < right_text_size; i++) {
            new_buffer[new_gap_right + 1 + i] = buffer[gap_right + 1 + i];
        }
        delete[] buffer; buffer = new_buffer;
        gap_right = new_gap_right; total_size = new_size;
    }

public:
    GapBuffer(int initial_capacity = 100) {
        total_size = initial_capacity;
        buffer = new char[total_size];
        gap_left = 0; gap_right = total_size - 1;
    }

    ~GapBuffer() { delete[] buffer; }

    void moveGapTo(int index) {
        if (index < 0) index = 0;
        if (index > length()) index = length();
        while (gap_left < index) { buffer[gap_left] = buffer[gap_right + 1]; gap_left++; gap_right++; }
        while (gap_left > index) { gap_left--; gap_right--; buffer[gap_right + 1] = buffer[gap_left]; }
    }

    void insertChar(char c) {
        if (gap_left > gap_right) expandGap();
        buffer[gap_left] = c; gap_left++;
    }

    char charBeforeGap() { return (gap_left > 0) ? buffer[gap_left - 1] : '\0'; }
    void backspace() { if (gap_left > 0) gap_left--; }

    std::string toString() {
        std::string result = "";
        for (int i = 0; i < gap_left; i++) result += buffer[i];
        for (int i = gap_right + 1; i < total_size; i++) result += buffer[i];
        return result;
    }

    int length() { return gap_left + (total_size - 1 - gap_right); }

    std::string splitAtGap() {
        std::string right_side = "";
        for (int i = gap_right + 1; i < total_size; i++) right_side += buffer[i];
        gap_right = total_size - 1; return right_side;
    }

    void appendString(std::string str) { for (char c : str) insertChar(c); }
};

// ==========================================
// --- THE LEXICAL SCANNER ---
// ==========================================
bool isKeyword(std::string word) {
    const char* keywords[] = {"int", "void", "return", "class", "public", "private", "if", "else", "while", "for", "string", "bool", "char"};
    for (int i = 0; i < 13; i++) if (word == keywords[i]) return true;
    return false;
}

std::string highlightLine(std::string raw_line) {
    std::string colored_line = ""; std::string current_word = "";
    for (int i = 0; i < (int)raw_line.length(); i++) {
        char c = raw_line[i];
        if (isalpha(c)) { current_word += c; }
        else {
            if (current_word.length() > 0) {
                if (isKeyword(current_word)) colored_line += "\x1b[38;5;14m" + current_word + "\x1b[0m";
                else colored_line += current_word;
                current_word = "";
            }
            colored_line += c;
        }
    }
    if (current_word.length() > 0) {
        if (isKeyword(current_word)) colored_line += "\x1b[38;5;14m" + current_word + "\x1b[0m";
        else colored_line += current_word;
    }
    return colored_line;
}

// ==========================================
// --- ENGINE VARIABLES & HARDWARE ---
// ==========================================
HANDLE hStdout; DWORD fdwSaveOldOutMode;
int cursor_x = 0; int cursor_y = 0; int row_offset = 0;

std::string statusMessage = " NORMAL | Ctrl+S: Save | Ctrl+O: Open | Ctrl+F: Find | Ctrl+R: Replace | Ctrl+Z: Undo | Ctrl+Y: Redo ";
std::vector<GapBuffer*> textBuffer;
std::string currentFilePath = "";

void showBlinkingCursor(bool isVisible) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = isVisible; SetConsoleCursorInfo(out, &cursorInfo);
}

void gotoxy(int x, int y) {
    COORD coord; coord.X = x; coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// ==========================================
// --- FILE I/O SYSTEM ---
// ==========================================
void initEditor() {
    system("mode con cols=105 lines=30");
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hStdout, &fdwSaveOldOutMode);
    SetConsoleMode(hStdout, fdwSaveOldOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    textBuffer.push_back(new GapBuffer());
}

void disableEditor() { SetConsoleMode(hStdout, fdwSaveOldOutMode); }

void saveFile() {
    if (currentFilePath == "") currentFilePath = "my_project_code.cpp";
    std::ofstream outFile(currentFilePath);
    if (outFile.is_open()) {
        for (int i = 0; i < (int)textBuffer.size(); i++) outFile << textBuffer[i]->toString() << "\n";
        outFile.close();
        statusMessage = " SUCCESS: Saved to " + currentFilePath + " ";
    } else {
        statusMessage = " ERROR: Could not save file! ";
    }
}

void loadFile(std::string path) {
    std::ifstream inFile(path);
    if (inFile.is_open()) {
        for (GapBuffer* gb : textBuffer) delete gb;
        textBuffer.clear();
        clearUndoStack(); clearRedoStack();

        std::string line;
        while (std::getline(inFile, line)) {
            GapBuffer* newLine = new GapBuffer();
            newLine->appendString(line);
            textBuffer.push_back(newLine);
        }
        inFile.close();

        if (textBuffer.empty()) textBuffer.push_back(new GapBuffer());

        currentFilePath = path;
        cursor_x = 0; cursor_y = 0; row_offset = 0;
        statusMessage = " SUCCESS: Loaded " + path + " ";
    } else {
        statusMessage = " ERROR: Could not find or open " + path + " ";
    }
}

// ==========================================
// --- THE SEARCH ALGORITHM ---
// ==========================================
void executeSearch() {
    searchMatches.clear();
    match_word = searchPrompt;
    bool found_first = false;

    for (int y = 0; y < (int)textBuffer.size(); y++) {
        std::string line = textBuffer[y]->toString();
        size_t pos = line.find(searchPrompt, 0);

        while (pos != std::string::npos) {
            searchMatches.push_back({(int)pos, y});

            if (!found_first) {
                cursor_y = y;
                cursor_x = (int)pos;

                row_offset = cursor_y - 10;
                if (row_offset < 0) row_offset = 0;

                found_first = true;
            }
            pos = line.find(searchPrompt, pos + searchPrompt.length());
        }
    }

    if (found_first) {
        statusMessage = " SUCCESS: Found " + std::to_string(searchMatches.size()) + " matches for '" + searchPrompt + "' ";
    } else {
        statusMessage = " ERROR: Could not find '" + searchPrompt + "' ";
        searchMatches.clear();
    }
}

// ==========================================
// --- NEW: FIND AND REPLACE ALGORITHM ---
// ==========================================
void executeReplace() {
    int replaceCount = 0;

    if (replaceTargetPrompt.empty()) {
        statusMessage = " ERROR: Cannot replace an empty word! ";
        return;
    }

    for (int y = 0; y < (int)textBuffer.size(); y++) {
        std::string line = textBuffer[y]->toString();
        size_t pos = line.find(replaceTargetPrompt, 0);

        while (pos != std::string::npos) {
            int start_x = (int)pos;

            textBuffer[y]->moveGapTo(start_x + replaceTargetPrompt.length());

            for (int i = 0; i < (int)replaceTargetPrompt.length(); i++) {
                char deletedChar = textBuffer[y]->charBeforeGap();
                textBuffer[y]->backspace();
                Action a = {DELETE_CHAR, deletedChar, start_x + (int)replaceTargetPrompt.length() - i - 1, y};
                undoStack.push(a);
            }

            textBuffer[y]->moveGapTo(start_x);
            int current_x = start_x;
            for (char c : replaceWithPrompt) {
                textBuffer[y]->insertChar(c);
                Action a = {TYPE_CHAR, c, current_x, y};
                undoStack.push(a);
                current_x++;
            }

            replaceCount++;
            clearRedoStack();

            line = textBuffer[y]->toString();
            pos = line.find(replaceTargetPrompt, start_x + replaceWithPrompt.length());
        }
    }

    if (replaceCount > 0) {
        statusMessage = " SUCCESS: Replaced " + std::to_string(replaceCount) + " occurrences. ";
    } else {
        statusMessage = " ERROR: Could not find '" + replaceTargetPrompt + "' to replace. ";
    }
}

// ==========================================
// --- THE PROJECTOR (With Line Numbers) ---
// ==========================================
void refreshScreen() {
    showBlinkingCursor(false);

    for (int y = 0; y < 24; y++) {
        gotoxy(0, y);
        std::string rawLine = "";
        int buffer_index = y + row_offset;

        std::string lineMargin = "";

        if (buffer_index < (int)textBuffer.size()) {
            rawLine = textBuffer[buffer_index]->toString();
            std::string numStr = std::to_string(buffer_index + 1);
            while (numStr.length() < 3) numStr = " " + numStr;
            lineMargin = "\x1b[90m" + numStr + " | \x1b[0m";
        } else {
            rawLine = "~";
            lineMargin = "\x1b[90m    | \x1b[0m";
        }

        if (rawLine.length() > 80) rawLine = rawLine.substr(0, 80);
        while (rawLine.length() < 84) rawLine += " ";

        std::cout << lineMargin << highlightLine(rawLine);
    }

    for (Match m : searchMatches) {
        if (m.y >= row_offset && m.y < row_offset + 24) {
            gotoxy(m.x + 6, m.y - row_offset);
            std::cout << "\x1b[43;30m" << match_word << "\x1b[0m";
        }
    }

    gotoxy(0, 24);
    std::string barText = "";
    if (currentMode == NORMAL) barText = statusMessage;
    else if (currentMode == COMMAND) barText = " OPEN FILE PATH: " + commandPrompt;
    else if (currentMode == SEARCH) barText = " SEARCH FOR: " + searchPrompt;
    else if (currentMode == REPLACE_TARGET) barText = " REPLACE: Find what? " + replaceTargetPrompt;
    else if (currentMode == REPLACE_WITH) barText = " REPLACE: '" + replaceTargetPrompt + "' with what? " + replaceWithPrompt;

    while (barText.length() < 105) barText += " ";
    std::cout << "\x1b[7m" << barText << "\x1b[0m";

    if (currentMode == COMMAND) gotoxy(17 + commandPrompt.length(), 24);
    else if (currentMode == SEARCH) gotoxy(13 + searchPrompt.length(), 24);
    else if (currentMode == REPLACE_TARGET) gotoxy(21 + replaceTargetPrompt.length(), 24);
    else if (currentMode == REPLACE_WITH) gotoxy(28 + replaceTargetPrompt.length() + replaceWithPrompt.length(), 24);
    else gotoxy(cursor_x + 6, cursor_y - row_offset);

    showBlinkingCursor(true);
    std::cout << std::flush;
}

// ==========================================
// --- MAIN EVENT LOOP ---
// ==========================================
int main() {
    initEditor();

    while (true) {
        refreshScreen();
        int c = _getch();

        if (currentMode == REPLACE_TARGET) {
            if (c == 27) {
                currentMode = NORMAL;
                statusMessage = " NORMAL | Ctrl+S: Save | Ctrl+O: Open | Ctrl+F: Find | Ctrl+R: Replace | Ctrl+Z: Undo | Ctrl+Y: Redo ";
            }
            else if (c == 13) {
                if (!replaceTargetPrompt.empty()) currentMode = REPLACE_WITH;
            }
            else if (c == 8) {
                if (replaceTargetPrompt.length() > 0) replaceTargetPrompt.pop_back();
            }
            else if (c >= 32 && c <= 126) replaceTargetPrompt += (char)c;
            continue;
        }

        if (currentMode == REPLACE_WITH) {
            if (c == 27) {
                currentMode = NORMAL;
                statusMessage = " NORMAL | Ctrl+S: Save | Ctrl+O: Open | Ctrl+F: Find | Ctrl+R: Replace | Ctrl+Z: Undo | Ctrl+Y: Redo ";
            }
            else if (c == 13) {
                executeReplace();
                currentMode = NORMAL;
            }
            else if (c == 8) {
                if (replaceWithPrompt.length() > 0) replaceWithPrompt.pop_back();
            }
            else if (c >= 32 && c <= 126) replaceWithPrompt += (char)c;
            continue;
        }

        if (currentMode == SEARCH) {
            if (c == 27) {
                currentMode = NORMAL;
                statusMessage = " NORMAL | Ctrl+S: Save | Ctrl+O: Open | Ctrl+F: Find | Ctrl+R: Replace | Ctrl+Z: Undo | Ctrl+Y: Redo ";
            }
            else if (c == 13) {
                executeSearch();
                currentMode = NORMAL;
            }
            else if (c == 8) {
                if (searchPrompt.length() > 0) searchPrompt.pop_back();
            }
            else if (c >= 32 && c <= 126) searchPrompt += (char)c;
            continue;
        }

        if (currentMode == COMMAND) {
            if (c == 27) {
                currentMode = NORMAL;
                statusMessage = " NORMAL | Ctrl+S: Save | Ctrl+O: Open | Ctrl+F: Find | Ctrl+R: Replace | Ctrl+Z: Undo | Ctrl+Y: Redo ";
            }
            else if (c == 13) {
                loadFile(commandPrompt);
                currentMode = NORMAL;
            }
            else if (c == 8) {
                if (commandPrompt.length() > 0) commandPrompt.pop_back();
            }
            else if (c >= 32 && c <= 126) commandPrompt += (char)c;
            continue;
        }

        if (c == 224 || c == 0 || c == 13 || c == 8 || (c >= 32 && c <= 126)) searchMatches.clear();

        if (c == 17) break; // Ctrl + Q
        if (c == 19) { saveFile(); continue; } // Ctrl + S

        if (c == 15) { // Ctrl + O
            currentMode = COMMAND;
            commandPrompt = "";
            continue;
        }

        if (c == 6) { // Ctrl + F
            currentMode = SEARCH;
            searchPrompt = "";
            continue;
        }

        if (c == 18) { // Ctrl + R
            currentMode = REPLACE_TARGET;
            replaceTargetPrompt = "";
            replaceWithPrompt = "";
            continue;
        }

        // BUG FIXED HERE: Replaced the rogue semicolon with the proper && operator
        if (statusMessage != " NORMAL | Ctrl+S: Save | Ctrl+O: Open | Ctrl+F: Find | Ctrl+R: Replace | Ctrl+Z: Undo | Ctrl+Y: Redo " &&
            statusMessage.find("SUCCESS:") == std::string::npos &&
            statusMessage.find("ERROR:") == std::string::npos) {
            statusMessage = " NORMAL | Ctrl+S: Save | Ctrl+O: Open | Ctrl+F: Find | Ctrl+R: Replace | Ctrl+Z: Undo | Ctrl+Y: Redo ";
        }

        if (c == 26) {
            if (!undoStack.empty()) {
                Action a = undoStack.top(); undoStack.pop();
                redoStack.push(a);

                if (a.type == TYPE_CHAR) {
                    cursor_x = a.x + 1; cursor_y = a.y;
                    textBuffer[cursor_y]->moveGapTo(cursor_x);
                    textBuffer[cursor_y]->backspace();
                    cursor_x--;
                }
                else if (a.type == DELETE_CHAR) {
                    cursor_x = a.x - 1; cursor_y = a.y;
                    textBuffer[cursor_y]->moveGapTo(cursor_x);
                    textBuffer[cursor_y]->insertChar(a.c);
                    cursor_x++;
                }
            }
            continue;
        }

        if (c == 25) {
            if (!redoStack.empty()) {
                Action a = redoStack.top(); redoStack.pop();
                undoStack.push(a);

                if (a.type == TYPE_CHAR) {
                    cursor_x = a.x; cursor_y = a.y;
                    textBuffer[cursor_y]->moveGapTo(cursor_x);
                    textBuffer[cursor_y]->insertChar(a.c);
                    cursor_x++;
                }
                else if (a.type == DELETE_CHAR) {
                    cursor_x = a.x; cursor_y = a.y;
                    textBuffer[cursor_y]->moveGapTo(cursor_x);
                    textBuffer[cursor_y]->backspace();
                    cursor_x--;
                }
            }
            continue;
        }

        if (c == 224 || c == 0) {
            int direction = _getch();
            switch (direction) {
            case 72: cursor_y--; break;
            case 80: cursor_y++; break;
            case 75: cursor_x--; break;
            case 77: cursor_x++; break;
            }
        }
        else if (c == 13) {
            textBuffer[cursor_y]->moveGapTo(cursor_x);
            std::string remainder = textBuffer[cursor_y]->splitAtGap();
            GapBuffer* newLine = new GapBuffer();
            newLine->appendString(remainder);
            textBuffer.insert(textBuffer.begin() + cursor_y + 1, newLine);
            cursor_y++; cursor_x = 0;
            clearRedoStack();
        }
        else if (c == 8) {
            if (cursor_x > 0) {
                textBuffer[cursor_y]->moveGapTo(cursor_x);
                char deletedChar = textBuffer[cursor_y]->charBeforeGap();
                textBuffer[cursor_y]->backspace();

                Action a = {DELETE_CHAR, deletedChar, cursor_x, cursor_y};
                undoStack.push(a);
                clearRedoStack();
                cursor_x--;
            } else if (cursor_y > 0) {
                std::string currentLine = textBuffer[cursor_y]->toString();
                cursor_x = textBuffer[cursor_y - 1]->length();
                textBuffer[cursor_y - 1]->appendString(currentLine);
                delete textBuffer[cursor_y];
                textBuffer.erase(textBuffer.begin() + cursor_y);
                cursor_y--;
                clearRedoStack();
            }
        }
        else if (c >= 32 && c <= 126) {
            if (cursor_x >= 80) {
                textBuffer.insert(textBuffer.begin() + cursor_y + 1, new GapBuffer());
                cursor_y++; cursor_x = 0;
            }
            textBuffer[cursor_y]->moveGapTo(cursor_x);
            textBuffer[cursor_y]->insertChar((char)c);

            Action a = {TYPE_CHAR, (char)c, cursor_x, cursor_y};
            undoStack.push(a);
            clearRedoStack();
            cursor_x++;
        }

        if (cursor_y < 0) cursor_y = 0;
        if (cursor_y >= (int)textBuffer.size()) cursor_y = (int)textBuffer.size() - 1;
        if (cursor_x < 0) cursor_x = 0;
        if (cursor_x > textBuffer[cursor_y]->length()) cursor_x = textBuffer[cursor_y]->length();

        if (cursor_y < row_offset) row_offset = cursor_y;
        if (cursor_y >= row_offset + 24) row_offset = cursor_y - 23;
    }

    for (GapBuffer* gb : textBuffer) delete gb;
    textBuffer.clear();

    disableEditor();
    system("cls");
    return 0;
}