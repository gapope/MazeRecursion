/*****************************************************************************
 *	Name: Greg  Pope                                                         *
 *	Course: ICS4U                                                            *
 *	Date: Nov. 15, 2017                                                      *
 *	                                                                         *
 *	Purpose:                                                                 *
 *	    Loading and solving a maze from a file of characters, can use depth  *
 *      or breadth first search.                                             *
 *	Usage:                                                                   *
 *	    Follow instructions provided by program                              *
 *	Revision History:                                                        *
 *	                                                                         *
 *	Known Issues:                                                            *
 *	                                                                         *
 *****************************************************************************/

#include <iostream>
#include <fstream>
#include <apmatrix.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <deque>

using namespace std;

bool loadMaze(apmatrix<char> &maze);
bool alInits(ALLEGRO_DISPLAY* &display, ALLEGRO_EVENT_QUEUE* &event_queue);
bool drawMaze(apmatrix<char> &maze, int x, int y);
bool findStart(const apmatrix<char> &maze, int &x, int &y);
bool findGoal(const apmatrix<char> &maze, int &x, int &y);
bool findPath(apmatrix<char> &maze, int x, int y);
bool findPathBreadth(apmatrix<char> &maze, apmatrix<int> &direction, deque<int> &xD, deque<int> &yD);
bool backTrace(apmatrix<char> &maze, const apmatrix<int> &direction, int x, int y);

int SCREEN_W = 480;
const int SCREEN_H = 480;

int main() {
    apmatrix<char> maze;

    char input;
    bool success = false;
    while(!success) {

        cout << "Would you like to load a maze file (y/n): ";
        cin >> input;

        if (toupper(input) == 'N') return 0;

        success = loadMaze(maze);
    }

    while (input != 'D' && input != 'B') {
        cout << "Solve with (D)epth first search or (B)readth first: ";
        cin >> input;
        input = toupper(input);
    }

    //Resizing screen relative to maze dimensions
    SCREEN_W = SCREEN_H * float(maze.numcols() / maze.numrows());

    //Allegro setup
    ALLEGRO_DISPLAY *display = nullptr;
    ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
    if (!alInits(display, event_queue)) return -1;
    al_set_window_title(display, "Maze Solving...");


    int x, y;

    if (!findStart(maze, x, y)) {
        cout << "Maze has no start" << endl;
        return -1;
    }

    drawMaze(maze, x, y);

    if (input == 'D') {
        if (findPath(maze, x, y))
            al_set_window_title(display, "Maze solved! (press ESC to quit)");
        else
            al_set_window_title(display, "Unable to solve maze! (press ESC to quit)");
    }
    else if (input == 'B') {
        apmatrix<int> direction(maze.numrows(), maze.numcols(), 0);
        deque<int> xD, yD;

        //Mounting start position
        xD.push_back(x);
        yD.push_back(y);

        //Looping until path is found
        while (!xD.empty() && !findPathBreadth(maze, direction, xD, yD));

        findGoal(maze, x, y);

        if (backTrace(maze, direction, x, y)) {
            drawMaze(maze, x, y);
            al_set_window_title(display, "Maze solved! (press ESC to quit)");
        } else
            al_set_window_title(display, "Unable to solve maze! (press ESC to quit)");
    }


    bool quit = false;

    //Loop to quit on ESC
    while (!quit) {
        ALLEGRO_EVENT ev;

        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { //Quitting if window is closed
            break;
        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) { //Quitting on ESC
            quit = true;
        }
    }

    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0;
}

//Loads maze from file provided by user
bool loadMaze(apmatrix<char> &maze) {
    string fileName;

    cout << "Enter maze file: ";
    cin >> fileName;

    ifstream fin(fileName);

    //Error checking
    if (!fin.is_open()) {
        cerr << "Unable to find/open file " << fileName << endl;
        return false;
    }

    //Maze dimensions
    int x, y;
    fin >> x >> y;

    //Size safety
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

//Allegro component initializations
bool alInits(ALLEGRO_DISPLAY* &display, ALLEGRO_EVENT_QUEUE* &event_queue) {
    //Allegro
    if (!al_init()) {
        cerr << "Failed to initialize allegro" << endl;
        return false;
    }

    //Window
    display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) {
        cerr << "Failed to initialize display" << endl;
        return false;
    }

    //Keyboard
    if (!al_install_keyboard()) {
        cerr << "Failed to initialize keyboard" << endl;
        return false;
    }

    //Event system
    event_queue = al_create_event_queue();
    if(!event_queue) {
        cerr << "Failed to create event_queue" << endl;
        return false;
    }
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    //Images
    al_init_image_addon();

    return true;
}

//Draws maze based off characters and current location being searched
bool drawMaze(apmatrix<char> &maze, int x, int y) {
    static ALLEGRO_BITMAP *character = al_load_bitmap("Data/character.bmp");
    if (!character) return false;

    static ALLEGRO_BITMAP *invalid = al_load_bitmap("Data/invalid.bmp");
    if (!invalid) return false;

    static ALLEGRO_BITMAP *valid = al_load_bitmap("Data/valid.bmp");
    if (!valid) return false;

    static ALLEGRO_BITMAP *searched = al_load_bitmap("Data/searched.bmp");
    if (!searched) return false;

    static ALLEGRO_BITMAP *failed = al_load_bitmap("Data/failed.bmp");
    if (!failed) return false;

    static ALLEGRO_BITMAP *goal = al_load_bitmap("Data/goal.bmp");
    if (!goal) return false;

    //Scaling ratios
    static const int width = SCREEN_W / maze.numcols(), height = SCREEN_H / maze.numrows();

    for (int i =0; i < maze.numrows(); i++) {
        for (int j = 0; j < maze.numcols(); j++) {
            if (i == x && j == y) //Location being search
                al_draw_scaled_bitmap(character, 0, 0, 80, 80,
                                      j * width, i * height, width, height, 0);
            else if (maze[i][j] == '#') //Invalid blocks
                al_draw_scaled_bitmap(invalid, 0, 0, 80, 80,
                                      j * width, i * height, width, height, 0);
            else if (maze[i][j] == '.') //Unsearched blocks
                al_draw_scaled_bitmap(valid, 0, 0, 80, 80,
                                      j * width, i * height, width, height, 0);
            else if (maze[i][j] == '+') //Searched blocks
                al_draw_scaled_bitmap(searched, 0, 0, 80, 80,
                                      j * width, i * height, width, height, 0);
            else if (maze[i][j] == 'x') //Seached blocks leading to dead-end
                al_draw_scaled_bitmap(failed, 0, 0, 80, 80,
                                      j * width, i * height, width, height, 0);
            else if (maze[i][j] == 'G') //Goal
                al_draw_scaled_bitmap(goal, 0, 0, 80, 80,
                                      j * width, i * height, width, height, 0);
        }
    }

    al_flip_display();

    al_rest(0.2);
    return true;
}

//Finds coordinates of start in a materix maze
bool findStart(const apmatrix<char> &maze, int &x, int &y) {
    for (int i = 0; i < maze.numrows(); i++) {
        for (int j = 0; j < maze.numcols(); j++) {
            if (maze[i][j] == 'S') {
                x = i;
                y = j;
                return true;
            }
        }
    }

    return false;
}

//Finds coordinates of goal in a materix maze
bool findGoal(const apmatrix<char> &maze, int &x, int &y) {
    for (int i = 0; i < maze.numrows(); i++) {
        for (int j = 0; j < maze.numcols(); j++) {
            if (maze[i][j] == 'G') {
                x = i;
                y = j;
                return true;
            }
        }
    }

    return false;
}

//Location goal of a maze using recursive a depth first search
bool findPath(apmatrix<char> &maze, int x, int y) {
    //Out of bounds
    if ((x < 0 || y < 0) || (x >= maze.numrows() || y >= maze.numcols()))
        return false;

    //Walls and already visited
    if (maze[x][y] == '#' || maze[x][y] == '+' || maze[x][y] == 'x')
        return false;

    //Visuals
    //Visuals
    if (!drawMaze(maze, x, y)) {
        cerr << "Image loading fail" << endl;
        return false;
    }

    //Success
    if (maze[x][y] == 'G')
        return true;

    maze[x][y] = '+';

    if (findPath(maze, x, y - 1)) //Up
        return true;
    if (findPath(maze, x + 1, y)) //Right
        return true;
    if (findPath(maze, x, y + 1)) //Down
        return true;
    if (findPath(maze, x - 1, y)) //Left
        return true;

    //Marking paths which reach a dead-end
    maze[x][y] = 'x';

    return false;
}

//Locating goal of a maze using a recursive breadth first search
bool findPathBreadth(apmatrix<char> &maze, apmatrix<int> &direction, deque<int> &xD, deque<int> &yD) {
    int x = xD.front(), y = yD.front();

    xD.pop_front();
    yD.pop_front();

    //Out of bounds
    if ((x < 0 || y < 0) || (x >= maze.numrows() || y >= maze.numcols()))
        return false;

    //Walls and already visited
    if (maze[x][y] == '#' || maze[x][y] == 'x')
        return false;

    //Marking direction block was arrived by from
    if (maze[x][y] == 'S');
    else if (y < maze.numcols() - 1 && maze[x][y + 1] == 'x') //Came from below
        direction[x][y] = 1;
    else if (x < maze.numrows() - 1 && maze[x + 1][y] == 'x') //Came from right
        direction[x][y] = 2;
    else if (y > 0 && maze[x][y - 1] == 'x') //Came from left
        direction[x][y] = 3;
    else if (x > 0 && maze[x - 1][y] == 'x') //Came from above
        direction[x][y] = 4;

    //Visuals
    if (!drawMaze(maze, x, y)) {
        cerr << "Image loading fail" << endl;
        return false;
    }

    //Success
    if (maze[x][y] == 'G')
        return true;

    //Up
    xD.push_back(x);
    yD.push_back(y - 1);

    //Left
    xD.push_back(x - 1);
    yD.push_back(y);

    //Down
    xD.push_back(x);
    yD.push_back(y + 1);

    //Right
    xD.push_back(x + 1);
    yD.push_back(y);

    //Feeding new round of searches
    if (findPathBreadth(maze, direction, xD, yD))
        return true;

    //Marking searched blocks
    maze[x][y] = 'x';

    return false;
}

//Finds the path between goal and start of a maze searched breadth first
bool backTrace(apmatrix<char> &maze, const apmatrix<int> &direction, int x, int y) {

    if (maze[x][y] == 'S') return true; //Found beginning

    //marking path
    if (maze[x][y] != 'G' && maze[x][y] != 'S') maze[x][y] = '+';

    if (direction[x][y] == 0) //Invalid
        return false;
    if (direction[x][y] == 1) //Came from below
        return backTrace(maze, direction, x, y + 1);
    if (direction[x][y] == 2) //Came from right
        return backTrace(maze, direction, x + 1, y);
    if (direction[x][y] == 3) //Came from below
        return backTrace(maze, direction, x, y - 1);
    if (direction[x][y] == 4) //Came from left
        return backTrace(maze, direction, x - 1, y);

    return false;
}
