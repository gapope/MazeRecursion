#include <iostream>
#include <fstream>
#include <apmatrix.h>

using namespace std;

bool FileIntake(apmatrix<char> &maze);
bool FindPath(apmatrix<char> &maze, int x, int y);

int main() {

    apmatrix<char> maze;

    while(!FileIntake(maze));

    //FOR TESTING REMOVE LATER
    for (int i =0; i < maze.numcols(); i++) {
        for (int j = 0; j < maze.numrows(); j++) {
            cout << maze[i][j];
        }
        cout << "\n";
    }
    //^^REMOVE LATER

    FindPath(maze, 0, 0);


    return 0;
}

bool FileIntake(apmatrix<char> &maze) {
    string fileName;

    cout << "Enter maze file: ";
    cin >> fileName;

    ifstream fin(fileName);

    //Error checking
    if (!fin.is_open()) {
        cerr << "Unable to find/open file " << fileName << endl;
        return false;
    }

    int x, y;

    fin >> x >> y;

    // Size safety
    if (x < 1 || y < 1) {
        cerr << "Maze of improper dimensions" << endl;
        fin.close();
        return false;
    }

    maze.resize(x, y);

    for (int i = 0; i < x; i ++) {
        for (int j = 0; j < y; j++) {
            fin >> maze[i][j];
        }
    }

    fin.close();

    return true;
}

bool FindPath(apmatrix<char> &maze, int x, int y) {
    static apmatrix<bool> solution(maze.numcols(),maze.numrows(), false);

    //Out of bounds
    if (x < 0 || y < 0)
        return false;

    //Success
    if (maze[x][y] == 'G')
        return true;

    if (maze[x][y] == '#')
        return false;

    solution[x][y] = true;

    if (FindPath(maze, x, y - 1))
        return true;
    if (FindPath(maze, x + 1, y))
        return true;
    if (FindPath(maze, x, y + 1))
        return true;
    if (FindPath(maze, x - 1, y))
        return true;

    solution[x][y] = false;

    return false;
}
bool SolveMaze(apmatrix<char> &maze)
