#include <ncurses.h>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;
/*
//            pre : [NAME] is a modal piano roll / midi manipulator.
//                  [NAME] contains several "modes" that allow you to perform diffrent operations on notes.
//                  you can only operate in 1 mode at a time.
// changing modes : the number keys are used to select and change mode. inside of each mode, modes may have "mods";
//                  use the top row of your keyboard to change mod (qwertyuiop).
//                  when you switch modes, some modes can carry a group of selected notes into the next mode,
//                  modes may opperate within a group of selected notes, for example; edit mode needs to have notes to edit of course
//             #MODES#
//      move mode : move the cursor around, with jumps, single moves, or octaves
//   keybinds 1/m
//    insert mode : adds notes to piano roll at cursor when you enter
//   keybinds 2/i   move cursor around just like in move mode except dragging the note with
//     nothing in   shift+right/left arrow to control size when placing
// selection? out   if you enter insert mode while in insert mode another note is placed at the end of current note
//                     -(q/g)group mod : all placed notes are added to same selection
//    select mode : lets you jump cursor with arrow keys to dif notes on piano roll
//   keybinds 3/s   when you do, note will become "highlighted";
//   selection in   if you choose to select highlighted note by pressing a key
//  selection out   it will be either be
//                     -(q/a)added to selection
//                     -(w/t)toggle selection
//                     -(e/d)removed from selection
//                     -(r/r)replace selection
//                  based on what key you pressed.
//                  jumping to another note from the highlighted note skips the note.
//      edit mode : entered from select or insert mode with a selection of notes;
//   keybinds 4/e   move with arrow keys shifts notes around; you can use ctrl + arrow to shift up octaves
//   selection in    shift + left/right arrow will do the special move
// selection? out    you can change current special move by pressing a key
//                     -(q/b)   border mod : works with 2 adjecent groups of notes selected, shifts where they meet with left / right arrows;
//                     -(w/v) velocity mod : adjust note velocity with up / down arrow;
//                     -(e/p)      pan mod : adjust note panning with up / down arrow;
//                     -(r/s)     size mod : adjust notes with left / right arrow;
//   command mode : lets you perform commands on currently selected notes
//    selction in   usally to exit a mode, you enter another, in command mode, you need to press escape, 
// selection? out   due to all keys being used for typing
//                  after command entered, go to mode we were in before unless command specifies other
//                  #file commands#
//          :e <filepath> opens file
//                     :w writes file
//                     :q exits program
//                    :wq writes and exits program
//          #manipulation commands#
//                  :n(i) makes new note at cursor, starts edit mode
//                  :b(i) duplicates selection i times
//                     :d deletes selected notes
//                     :c copies selected notes relitive to cursor
//                     :x cuts selected notes relitive to cursor
//                     :p paste notes 
//                   :con connect notes in selection
//              :c(i) (a) splits selected notes at the start of the note + a, and then repeats with all children chopped
//              #variable commands#
//                        sets notes placed in insert modes
//                 :l (a) length,
//                 :v (a) velocity,
//                 :P (a) panning, 
//                 :s (a) or start
//                        to a.
//                        changes selected notes
//                :rl (a) length,
//                :rv (a) velocity,
//                :rp (a) panning,
//                :rs (a) or start
//                        to a.
*/
enum direction{
    UP,
    DOWN,
    LEFT,
    RIGHT
};
enum modes{
    MOVE, INSERT, SELECT, EDIT
};
enum mods{
    MOD1 = 1, MOD2 = 2, MOD3 = 3, MOD4 = 4, MOD5 = 5, MOD6 = 6,
};
enum programKeys{
    moveKey = 49,
    insertKey = 50,
    selectKey = 51,
    editKey = 52,
    modKey1 = 113, modKey2 = 119, modKey3 = 101, modKey4 = 114, modKey5 = 116, modKey6 = 121,
    upKey = 107, downKey = 106, leftKey = 104, rightKey = 108,
    sUpKey = 75, sDownKey = 74, sLeftKey = 72, sRightKey = 76,
    cUpKey = 11, cDownKey = 10, cLeftKey = 8, crightKey = 12,
    commandKey = 58, escapeKey = 27
};

void moveModeInput(int in);
void insertModeInput(int in);
void editModeInput(int in);
void selectModeInput (int in);

void Render(int notes, int collums);
void renderAt(int x, int y, int rows, char c);
int jumpCursor(enum direction dir);
int cursorx = 0, cursory = 0;
class note{
    public:
        int startPos, length, velocity, pan, pitch;
        note(int startPos_, int length_, int velocity_, int pan_, int pitch_){
            startPos = startPos_;
            length = length_;
            velocity = velocity_;
            pan = pan_;
            pitch = pitch_;
        }
};
string noteNames[12] = {
    "A", 
    "A#", 
    "B", 
    "C", 
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#"
};
note ghostNote(0, 4, 127, 0, 12);
vector<note> notes;
vector<int> selected;
int highlightIndex;
enum mods mod;
enum modes mode = MOVE;
int main(){
    // init screen and sets up screen
    initscr();
    //add notes n hsit delete later
    notes.push_back(note(8, 4, 127, 0, 3));
    notes.push_back(note(4, 4, 127, 0, 6));
    notes.push_back(note(4, 4, 127, 0, 8));
    //io
    keypad(stdscr, true);
    int in;
    while (true) {
        //read input
        in = getch();
        clear();
        //switch to see if a mode key is pressed
        switch (in) {
            case moveKey:   mode = MOVE;   break;
            case insertKey: mode = INSERT; break;
            case selectKey: mode = SELECT; break;
            case editKey:   mode = EDIT;   break;
            default:
            switch (mode) {
                case MOVE:   moveModeInput(in);   break;
                case INSERT: insertModeInput(in); break;
                case SELECT: selectModeInput(in); break;
                case EDIT:   editModeInput(in);   break;
            }   break;
        }
        Render(12*2, 64);
        refresh();
    }
    // deallocates memory and ends ncurses
    endwin();
    return 0;
}
void moveModeInput(int in){
    switch (in) {
        //shift cursor by one
        case upKey: cursory+=1;
        break;
        case downKey: cursory-=1; 
        break;
        case leftKey: cursorx-=1; 
        break;
        case rightKey: cursorx+=1;
        break;
        //
        //jump cursor to notes
        case sUpKey: highlightIndex = jumpCursor(UP);
        break;
        case sDownKey: highlightIndex = jumpCursor(DOWN);
        break;
        case sLeftKey: highlightIndex = jumpCursor(LEFT);
        break;
        case sRightKey: highlightIndex = jumpCursor(RIGHT);
        break;
        //
        //octave up / down
        case cUpKey: cursory += 12;
        break;
        case cDownKey: cursory -= 12;
        break;
    }
}
void insertModeInput(int in){
    //insert mode switch
    switch (in) {
        //mod keys
        case modKey1:
            //place note
            notes.push_back(note(cursorx, ghostNote.length, ghostNote.velocity, ghostNote.pan, cursory));
            selected.push_back(notes.size() - 1);
        break;
        case modKey2:
            //single key : notes not part of selection
        break;
        //nav keys
        //shift cursor by one
        case upKey: cursory+=1;
        break;
        case downKey: cursory-=1;
        break;
        case leftKey: cursorx-=1;
        break;
        case rightKey: cursorx+=1;
        break;
        //shift hjkl: jump cursor to notes
        case sUpKey: highlightIndex = jumpCursor(UP);
        break;
        case sDownKey: highlightIndex = jumpCursor(DOWN);
        break;
        case sLeftKey: highlightIndex = jumpCursor(LEFT);
        break;
        case sRightKey: highlightIndex = jumpCursor(RIGHT);
        break;
        //octave up / down
        case cUpKey: cursory += 12; //ctrl k
        break;
        case cDownKey: cursory -= 12; //ctrl j
        break;
    }
}
void selectModeInput(int in){
    switch (in) {
        case modKey1:
            //add highlighted
            selected.push_back(highlightIndex);
        break;
        case modKey2:{
            //toggle highlighted
            auto it = find(selected.begin(), selected.end(), highlightIndex);
            if (it != selected.end())
                selected.erase(it);
            else
                selected.push_back(highlightIndex);
        }   break;
        case modKey3:{
            //remove from selection
            auto it = find(selected.begin(), selected.end(), highlightIndex);
            if (it != selected.end())
                selected.erase(it);
        }   break;
        //replace selection
        case modKey4:
            //replace selection with highlighted
            selected.clear();
            selected.push_back(highlightIndex);
        break;
        //nav keys
        case upKey: highlightIndex = jumpCursor(UP);
        break;
        case downKey: highlightIndex = jumpCursor(DOWN);
        break;
        case leftKey: highlightIndex = jumpCursor(LEFT);
        break;
        case rightKey: highlightIndex = jumpCursor(RIGHT);
        break;
    }
}
void editModeInput(int in){
    switch (in) {
        //mod keys
        case modKey1: //size mod
            mod = MOD1;
        break;
        case modKey2: //velocity mod
            mod = MOD2;
        break;
        case modKey3: //pan mod
            mod = MOD3;
        break;
        case modKey4: //border mod
            mod = MOD4;
        break;
        //edit keys
        case upKey: case rightKey: case downKey: case leftKey:
            switch (mod) {
                case MOD1: //size
                    for(int i = 0; i < selected.size(); i++)
                        notes[selected[i]].length += (upKey || rightKey ) ? 1 : -1;
                break;
                case MOD2: //velocity
                    for(int i = 0; i < selected.size(); i++)
                        notes[selected[i]].velocity += (upKey || rightKey ) ? 1 : -1;
                break;
                case MOD3: //pan
                    for(int i = 0; i < selected.size(); i++)
                        notes[selected[i]].pan += (upKey || rightKey ) ? 1 : -1;
                break;
                case MOD4: //border
                break;
                default: break;
            }
        break;
    }
}
int jumpCursor(enum direction dir){
    switch (dir) {
        case UP:{
            int closest = cursory;
            bool highest = true;
            int index;
            for(int i = 0; i < notes.size(); i++){
                int pitch = notes[i].pitch;
                if (notes[i].startPos == cursorx && pitch > cursory){ //are we higher and on same plane?
                    if (highest || pitch < closest){
                        closest = pitch;
                        index = i;
                    }
                    highest = false;
                }
            }
            cursory = closest;
            return index;
        }break;
        case DOWN:{
            int closest = cursory;
            bool lowest = true;
            int index;
            for(int i = 0; i < notes.size(); i++){
                int pitch = notes[i].pitch;
                if (notes[i].startPos == cursorx && pitch < cursory){ //are we lower and on same plane?
                    if (lowest || pitch > closest){
                        closest = pitch;
                        index = i; 
                    }
                    lowest = false;
                }
            }
            cursory = closest;
            return index;
        }break;
        case LEFT:{
            int closest = cursorx;
            int lowest = cursory;
            bool leftmost = true;
            int index;
            for(int i = 0; i < notes.size(); i++){
                int start = notes[i].startPos;
                if (start < cursorx){     //are we left of cursor?
                    if (leftmost          //first time note is further right
                    ||  start > closest){ //or note is closer than previous closest
                        closest = start;
                        lowest = notes[i].pitch;
                        index = i;
                    }
                    else if (start == closest     //we are just as close as the closest
                    && notes[i].pitch < lowest) { //and we are also lower than the closest
                        lowest = notes[i].pitch;
                        index = i;
                    }
                    leftmost = false;
                }
            }
            cursorx = closest;
            cursory = lowest;
            return index;
        }break;
        case RIGHT:{
            int closest = cursorx;
            int lowest = cursory;
            bool rightmost = true;
            int index;
            for(int i = 0; i < notes.size(); i++){
                int start = notes[i].startPos;
                if (start > cursorx){     //are we right of cursor?
                    if (rightmost         //first time note is further right
                    ||  start < closest){ //or note is closer than previous closest
                        closest = start;
                        lowest = notes[i].pitch;
                        index = i;
                    }
                    else if (start == closest     //we are just as close as the closest
                    && notes[i].pitch < lowest) { //and we are also lower than the closest
                        lowest = notes[i].pitch;
                        index = i;
                    }
                    rightmost = false;
                }
            }
            cursorx = closest;
            cursory = lowest;
            return index;
        }break;
    }
}
void Render(int rows, int collums){
    //draw background
    for (int note = 0; note < rows; note++){
        //note names
        mvaddstr((rows-1) - note, 0, noteNames[note % 12].c_str());
        //render background
        for (int c = 0; c < collums; c++){ 
            //decide for bg if note needs to be marked
            switch ((c) % 16) {
                case 0: renderAt(c, note, rows, '|');
                break;
                case 4: case 8: case 12:
                renderAt(c, note, rows, ',');
                break;
                default: renderAt(c, note, rows, '.');
                break;
            }
        }
    }
    //draw notes
    for (note note : notes){
        //render note
        for (int l = 0; l < note.length; l++){
            renderAt(note.startPos + l, note.pitch, rows, '#');
        }
    }
    //draw cursor
    renderAt(cursorx, cursory, rows, '@');

    switch (mode) {

    case MOVE:   mvaddstr(LINES-4, 0, "move");   break;
    case INSERT: mvaddstr(LINES-4, 0, "insert"); break;
    case SELECT: mvaddstr(LINES-4, 0, "select"); break;
    case EDIT:   mvaddstr(LINES-4, 0, "edit");   break;
    }
}
void renderAt(int x, int y, int rows, char c){
    mvaddch((rows-1) - y, x + 3, c);
}