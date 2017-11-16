#include <iostream>
#include <fstream>
#include <apmatrix.h>

using namespace std;

bool FileIntake(apmatrix<char> &maze);

int main()
{

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



    return 0;
}

bool FileIntake(apmatrix<char> &maze) {
    string fileName;

    cout << "Enter maze file: ";
    cin >> fileName;

    ifstream fin(fileName);

    if (!fin.is_open()) {
        cerr << "Unable to find/open file " << fileName << endl;
        return false;
    }

    int x, y;

    fin >> x >> y;

    maze.resize(x, y);

    for (int i = 0; i < x; i ++) {
        for (int j = 0; j < y; j++) {
            fin >> maze[i][j];
        }
    }

    return true;
}
