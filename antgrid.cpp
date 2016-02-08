#include "antgrid.h"

AntGrid::AntGrid(Grid * displayGridP, AntSettings * settingsP, StateWidget *stateArrayP)
{
    //Store the pointers to the Grid and AntSettings objects and the StateWidget array
    displayGrid = displayGridP;
    settings = settingsP;
    stateArray = stateArrayP;

    calculateGridSize();

    //Dynamically create the 2D state array using the pointer to a pointer approach
    state = new int * [columnCount];
    for (int i=0; i<columnCount; i++)
        state[i] = new int[rowCount];

    //Place the ant at the starting location and set the state to zero everywhere.
    resetGrid();

    //SMALL ISSUE: THE calculateValues FUNCTION ACTUALLY GETS CALLED TWICE IN THE CONSTRUCTOR: ONCE DIRECTLY
    //AND ONCE AS PART OF resetGrid.  IT'S NOT A REAL PROBLEM - JUST INEFFICIENT.  THE REASON IS IT HAS TO BE
    //CALLED BEFORE THE ARRAY IS MADE AND ALSO HAS TO BE IN resetGrid BECAUSE CODE THAT TRIGGERS THAT SLOT
    //SOMETIMES NEEDS THE STARTING POSITION TO BE UPDATED.

}





AntGrid::~AntGrid()
{
    //Delete the 2D state array.
    for (int i=0; i<columnCount; i++)
        delete [] state[i];
    delete [] state;
}





//This function loops through the whole grid and resets the states to zero.  It also moves the ant back to the starting position.
void AntGrid::resetGrid()
{
    calculateStart();

    //Place the ant at the starting location.
    antX = startingColumn;
    antY = startingRow;
    antDirection = settings->startingDirection;

    //Loop through the whole grid and set the state to zero everywhere.
    for (int i = 0; i < columnCount; i++)
    {
        for (int j = 0; j < rowCount; j++)
            state[i][j] = 0;
    }

    //Make sure the ant is labeled as being in range
    outOfRange = false;

    //If the setting is enabled to draw the ant, draw it now on the reset grid.
    if (settings->showAntColor)
    {
        //Determine the coordinates of the square in display terms (i.e. without the buffer)
        int displayX = antX - settings->gridBuffer;
        int displayY = antY - settings->gridBuffer;

        //It is not necessary to check here to see if the displayX and displayY variables are within a proper range - the Grid class
        //does that check in the drawSquare function.
        displayGrid->drawSquare(displayX, displayY, settings->antColor);
    }
}




//This is a public function to return the state at a given row and column.  The main purpose of this function
//is to account for the grid buffer.  Things calling this function just use the column and row of visible
//squares - they don't need to know what the grid buffer is.
int AntGrid::getState(int column, int row)
{
    return state[column+settings->gridBuffer][row+settings->gridBuffer];
}





//This function recreates the grid when its size changes.  It pretty much just deletes the state array and then
//does all the same stuff as the constructor.  It doesn't need any arguments because it will get everything it
//needs from the AntSettings and Grid objects - pointers to which this class already has.
void AntGrid::resizeGrid()
{
    //Delete the 2D state array.
    for (int i=0; i<columnCount; i++)
        delete [] state[i];
    delete [] state;

    calculateGridSize();

    //Dynamically create the 2D state array using the pointer to a pointer approach
    state = new int * [columnCount];
    for (int i=0; i<columnCount; i++)
        state[i] = new int[rowCount];

    //Place the ant at the starting location and set the state to zero everywhere.
    resetGrid();
}





void AntGrid::calculateGridSize()
{
    //Determine the number of AntGrid columns and rows by adding the buffer to each end of both ranges.
    columnCount = 2 * settings->gridBuffer + displayGrid->columnCount;
    rowCount = 2 * settings->gridBuffer + displayGrid->rowCount;
}
void AntGrid::calculateStart()
{
    //Store the starting conditions in AntGrid terms (i.e. with the buffer added)
    startingColumn = settings->startingColumn + settings->gridBuffer;
    startingRow = settings->startingRow + settings->gridBuffer;
}






//This function does the real Langton's ant work: it moves the ant and makes the appropriate changes to
//the AntGrid object as it goes.
void AntGrid::moveAnt(int numberOfSteps, bool drawSquareAfterEachStep)
{
    //Advance the time.  This could be done one step at a time inside the above loop, but I chose not to for
    //two reason.  First, this should slightly increase performance.  Secondly, when rendering animations
    //to disk, I want to be able to have the counter continue even after the actual simulation has stopped
    //due to the ant being out of range.
    (settings->time) += numberOfSteps;

    //Quit now if the ant is out of range.  This way the only thing that is done is the time being advanced.
    if (outOfRange)
        return;

    //Variables to be used in the loop
    AntDirection instruction;
    int displayX, displayY;

    //Execute this loop once per step the ant is to take
    for (int i=0; i<numberOfSteps; i++)
    {
        //Look up the direction the ant should turn based on its current square.
        instruction = stateArray[ state[antX][antY] ].direction;

        //Add the instuction to the ant's direction - i.e. rotate the ant the appropriate amount.
        //The enum values of "instruction" can be automatically converted to an int for this addition.
        antDirection += instruction;

        //Make sure that the antDirection variable stays within the range of 0 to 3;
        if (antDirection > 3)
            antDirection -= 4;

        //Advance the color of the ant's square by one
        (state[antX][antY])++;

        //Make sure that the square is still within its range.  I.e. if it has reached a value equal to the state count, reset to zero.
        if (state[antX][antY] == settings->stateCount)
            state[antX][antY] = 0;

        //If the option to draw after each step is on, redraw the current square to its new color
        if (drawSquareAfterEachStep)
        {
            //Determine the coordinates of the square in display terms (i.e. without the buffer)
            displayX = antX - settings->gridBuffer;
            displayY = antY - settings->gridBuffer;

            //It is not necessary to check here to see if the displayX and displayY variables are within a proper range - the Grid class
            //does that check in the drawSquare function.
            displayGrid->drawSquare(displayX, displayY, stateArray[ state[antX][antY] ].color);
        }

        //Move the ant forward one square
        switch (antDirection)
        {
        case 0: //ant is facing up
            antX--;
            break;
        case 1: //ant is facing right
            antY--; //seems backwards to me, but makes the correct pattern
            break;
        case 2: //ant is facing down
            antX++;
            break;
        case 3: //ant is facing left
            antY++; //seems backwards to me, but makes the correct pattern
            break;
        }

        //If the option to draw the ant is on, as well as the option to draw after each step, draw the ant now
        if ( (settings->showAntColor)&&(drawSquareAfterEachStep) )
        {
            //Determine the coordinates of the square in display terms (i.e. without the buffer)
            displayX = antX - settings->gridBuffer;
            displayY = antY - settings->gridBuffer;

            //It is not necessary to check here to see if the displayX and displayY variables are within a proper range - the Grid class
            //does that check in the drawSquare function.
            displayGrid->drawSquare(displayX, displayY, settings->antColor);
        }

        //Check to see if the ant has moved out of range.
        if ( (antX<0)||(antY<0)||(antX==columnCount)||(antY==rowCount) )
        {
            outOfRange = true;
            break;
        }

    }

    //If the option to draw after each step is off, it is now necessary to redraw the entire image.
    if (!drawSquareAfterEachStep)
    {
        for (int i=0; i<displayGrid->columnCount; i++)
        {
            for (int j=0; j<displayGrid->rowCount; j++)
                displayGrid->drawSquare(i, j, stateArray[getState(i, j)].color);
        }
    }
}





//Since the stateArray pointer will change when the user alters the number of states (the array is actually
//deleted and recreated), it is necessary to have this function so the new pointer can be given to the AntGrid
//object.
void AntGrid::updateStateArrayPointer(StateWidget * stateArrayP)
{
    stateArray = stateArrayP;
}





//This function draws only the square that the ant is on.
void AntGrid::drawAntSquare()
{
    //Determine the coordinates of the square in display terms (i.e. without the buffer)
    int displayX = antX - settings->gridBuffer;
    int displayY = antY - settings->gridBuffer;

    //It is not necessary to check here to see if the displayX and displayY variables are within a proper range - the Grid class
    //does that check in the drawSquare function.

    if (settings->showAntColor)
        displayGrid->drawSquare(displayX, displayY, settings->antColor); //Draw the ant's color
    else
        displayGrid->drawSquare(displayX, displayY, stateArray[ state[antX][antY] ].color); //Draw the square's state color
}
