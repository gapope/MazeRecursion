#include <iostream>
#include <fstream>
#include <apmatrix.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

using namespace std;

bool loadMaze(apmatrix<char> &maze);
bool alInits(ALLEGRO_DISPLAY* &display, ALLEGRO_EVENT_QUEUE* &event_queue);
bool drawMaze(apmatrix<char> &maze, int x, int y);
bool findStart(apmatrix<char> &maze, int &x, int &y);
bool findPath(apmatrix<char> &maze, int x, int y);

const int SCREEN_W = 480;       // screen width
const int SCREEN_H = 480;       // screen height

int main() {

    apmatrix<char> maze;
    while(!loadMaze(maze));

    //Allegro setup
    ALLEGRO_DISPLAY *display = nullptr;
    ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
    if (!alInits(display, event_queue)) return -1;

    //FOR TESTING REMOVE LATER

    for (int i =0; i < maze.numcols(); i++) {
        for (int j = 0; j < maze.numrows(); j++) {
            cout << maze[i][j];
        }
        cout << "\n";
    }
    //^^REMOVE LATER

    int x, y;

    if (!findStart(maze, x, y))
        cout << "Maze has no start" << endl;

    drawMaze(maze, x, y);

    if (findPath(maze, x, y)) cout << "yay" << endl;

    bool quit = false;

    //Loop to quit on ESC
    while (!quit) {
        ALLEGRO_EVENT ev;

        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { //Quitting if window is closed
            break;
        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            quit = true;
        }
    }




    for (int i =0; i < maze.numcols(); i++) {
        for (int j = 0; j < maze.numrows(); j++) {
            cout << maze[i][j];
        }
        cout << "\n";
    }


    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0;
}

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

bool alInits(ALLEGRO_DISPLAY* &display, ALLEGRO_EVENT_QUEUE* &event_queue) {
    if (!al_init()) {
        cerr << "Failed to initialize allegro" << endl;
        return false;
    }

    display = al_create_display(SCREEN_W, SCREEN_W);

    if (!display) {
        cerr << "Failed to initialize display" << endl;
        return false;
    }

    //Keyboard
    if (!al_install_keyboard()) {
        cerr << "Failed to initialize keyboard" << endl;
        return false;
    }

    event_queue = al_create_event_queue();
    if(!event_queue) {
        cerr << "Failed to create event_queue" << endl;
        return false;
    }

    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_init_image_addon();

    return true;
}

bool drawMaze(apmatrix<char> &maze, int x, int y) {
     static ALLEGRO_BITMAP *character = al_load_bitmap("Data/character.bmp");
     if (!character) return false;
     static ALLEGRO_BITMAP *invalid = al_load_bitmap("Data/invalid.bmp");
     if (!invalid) return false;
     static ALLEGRO_BITMAP *valid = al_load_bitmap("Data/valid.bmp");
     if (!valid) return false;
     static ALLEGRO_BITMAP *searched = al_load_bitmap("Data/searched.bmp");
     if (!searched) return false;

    for (int i =0; i < maze.numcols(); i++) {
        for (int j = 0; j < maze.numrows(); j++) {
                if (i == x && j == y)
                    al_draw_bitmap(character, j * 80, i * 80, 0);
                else if (maze[i][j] == '#')
                    al_draw_bitmap(invalid, j * 80, i * 80, 0);
                else if (maze[i][j] == '.')
                    al_draw_bitmap(valid, j * 80, i * 80, 0);
                else if (maze[i][j] == '+')
                    al_draw_bitmap(searched, j * 80, i * 80, 0);
        }
    }

    al_flip_display();

    al_rest(1);
}

bool findStart(apmatrix<char> &maze, int &x, int &y) {
    for (int i = 0; i < maze.numcols(); i++) {
        for (int j = 0; j < maze.numrows(); j++) {
            if (maze[i][j] == 'S') {
                x = i;
                y = j;
                return true;
            }
        }
    }

    return false;
}

bool findPath(apmatrix<char> &maze, int x, int y) {
    //Out of bounds
    if ((x < 0 || y < 0) || (x >= maze.numcols() || y >= maze.numrows()))
        return false;

    //Success
    if (maze[x][y] == 'G')
        return true;
    if (maze[x][y] == '#' || maze[x][y] == '+')
        return false;

    drawMaze(maze, x, y);

    maze[x][y] = '+';

    if (findPath(maze, x, y - 1))
        return true;
    if (findPath(maze, x + 1, y))
        return true;
    if (findPath(maze, x, y + 1))
        return true;
    if (findPath(maze, x - 1, y))
        return true;

    maze[x][y] = 'x';



    return false;
}
