#ifndef ANTGRID_H
#define ANTGRID_H

#include "antdirection.h"
#include "grid.h"
#include "antsettings.h"
#include "statewidget.h"

class AntGrid
{
public:
    AntGrid(Grid * displayGridP, AntSettings * settingsP, StateWidget * stateArrayP);
    ~AntGrid();

    //Functions
    void resetGrid();
    void resizeGrid();
    int getState(int column, int row);
    void moveAnt(int numberOfSteps, bool drawSquareAfterEachStep);
    void updateStateArrayPointer(StateWidget * stateArrayP);
    void drawAntSquare();

    //Data members
    int antX, antY;
    int antDirection;
    bool outOfRange;

    //This is the main value in the class - it holds the current state of each square.
    int ** state; //pointer to a pointer for a dynamic 2D array





private:
    int columnCount;
    int rowCount;

    int startingColumn;
    int startingRow;

    Grid * displayGrid;
    AntSettings * settings;
    StateWidget * stateArray;

    void calculateGridSize();
    void calculateStart();

};

#endif // ANTGRID_H
