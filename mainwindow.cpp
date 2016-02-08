#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QFontDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Create all of the UI stuff that I made in Qt Creator.
    ui->setupUi(this);

    QApplication::setWindowIcon(QIcon(QPixmap(":/icons/images/ant64.png")));

    //Make the widgets show what is in the settings object.  The settings object sets up default values
    //when it is created.
    updateWidgetsFromSettings();

    //Create all of the necessary connections for MainWindow.
    setUpConnections();

    //Create the array of state widgets - just two as a default.
    stateArray = new StateWidget [2];

    //Initialise the two default StateWidget objects and add them to the docking widget.
    stateArray[0].initialize(1, antRight, qRgb(255, 255, 255));
    stateArray[1].initialize(2, antLeft, qRgb(0, 0, 0));
    ui->antStatesHorizontalLayout->addWidget(stateArray);
    ui->antStatesHorizontalLayout->addWidget(stateArray+1);
    connect(&(stateArray[0]), SIGNAL(colorChanged()), this, SLOT(redrawImage()));
    connect(&(stateArray[1]), SIGNAL(colorChanged()), this, SLOT(redrawImage()));

    //Set some sizes on the docking widgets to make them look nice
    ui->stateDockWidget->setFixedHeight(170); //The value was determined by trial and error - might present an issue if porting to a new environment
    ui->settingsDockWidget->setFixedWidth(ui->settingsScrollAreaWidgetContents->sizeHint().width() + 40); //The value of 40 was determined by trial and error - enough to make sure the horizonal scroll bar doesn't appear

    //Create the spacer that will make all of the state widgets size nicely.
    spacer = new QSpacerItem(20, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
    ui->antStatesHorizontalLayout->addSpacerItem(spacer);

    //Create the grid.
    displayGrid = new Grid(settings.pixelWidth, settings.pixelHeight, settings.cellSize, stateArray[0].color);

    //Create and set up the label that will display the grid's image.
    gridLabel = new QLabel(this);
    gridLabel->setPixmap(QPixmap::fromImage( *(displayGrid->gridImage)) );
    gridLabel->resize(gridLabel->pixmap()->size());
    gridLabel->setFrameStyle(QFrame::Box | QFrame::Plain);
    gridLabel->setLineWidth(0);

    //Create and set up the scroll area that will hold the label.
    gridScrollArea = new QScrollArea(this);
    gridScrollArea->setWidget(gridLabel); //Make the label the scroll area's main widget.
    gridScrollArea->setAlignment(Qt::AlignCenter); //Make the image centred in the scroll area.
    setCentralWidget(gridScrollArea); //Make the scroll area the central widget of MainWindow.

    //Seed the random number generator - used for randomizing and shuffling colors.
    randNum.seed(time(NULL));

    //Create the AntGrid object
    antGrid = new AntGrid(displayGrid, &settings, stateArray);

    //Make sure the flags are false
    timerRunning = false;
    timerToHDDRunning = false;

    //Create the time label, put it in the taskbar, and set its time to 0.
    timeLabel = new QLabel();
    ui->statusBar->addPermanentWidget(timeLabel);
    updateTimeLabel();

    //Create the image blender to be used
    imageBlender = new ImageBlender(&timerToHDDRunning);

    //Create the AntCounter object
    antCounter = new AntCounter(displayGrid, &settings, stateArray);

    //Reset everything to the start!  This is a bit redundant (redoes a couple of things in the constructor), but
    //it keeps things simpler.
    resetToStart();
}





MainWindow::~MainWindow()
{  
    delete antCounter;
    delete imageBlender;
    delete timeLabel;
    delete antGrid;
    delete gridLabel; //This has to be deleted BEFORE gridScrollArea - causes a crash if otherwise.
    delete gridScrollArea;
    delete displayGrid;
    delete [] stateArray;
    delete ui;
}





void MainWindow::setUpConnections()
{
    //Connections for the file menu
    connect(ui->actionLoadSettings, SIGNAL(triggered()), this, SLOT(loadSettings()));
    connect(ui->actionSaveSettings, SIGNAL(triggered()), this, SLOT(saveSettings()));
    connect(ui->actionSaveImage, SIGNAL(triggered()), this, SLOT(saveImage()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    //Connections for the view menu (showing/hiding UI components)
    connect(ui->actionShowColoursandRules, SIGNAL(triggered(bool)), ui->stateDockWidget, SLOT(setVisible(bool)));
    connect(ui->actionShowSettings, SIGNAL(triggered(bool)), ui->settingsDockWidget, SLOT(setVisible(bool)));
    connect(ui->actionFileToolbar, SIGNAL(triggered(bool)), ui->fileToolBar, SLOT(setVisible(bool)));
    connect(ui->actionViewToolbar, SIGNAL(triggered(bool)), ui->viewToolBar, SLOT(setVisible(bool)));
    connect(ui->actionRenderToolbar, SIGNAL(triggered(bool)), ui->renderToolBar, SLOT(setVisible(bool)));
    connect(ui->actionSearchToolbar, SIGNAL(triggered(bool)), ui->searchToolBar, SLOT(setVisible(bool)));

    //Connections for the render menu
    connect(ui->actionStartAnimation, SIGNAL(triggered()), this, SLOT(renderToScreenStartStop()));
    connect(ui->actionResetAnimation, SIGNAL(triggered()), this, SLOT(resetToStart()));
    connect(ui->actionUpdateOnce, SIGNAL(triggered()), this, SLOT(updateOnce()));
    connect(ui->actionRenderAnimationToHDD, SIGNAL(triggered()), this, SLOT(renderToHDDStartStop()));

    //Connections for the search menu
    connect(ui->actionSearchAllRules, SIGNAL(triggered()), this, SLOT(searchAll()));
    connect(ui->actionSearchRandomRules, SIGNAL(triggered()), this, SLOT(searchRandom()));

    //Connection for changing the state count
    //This connection is made before the connection that updates the settings object.  This is so when changeStateCount is run,
    //it can refer to the old state count (in settings.stateCount) and the new state count (passed to the function).  If the
    //settings object was updated first, the old count would be lost.
    connect(ui->stateCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeStateCount(int)));

    //Connections for updating settings whenever a widget is changed
    //These connections are the first set up for most of the settings widgets so they will be the first executed when any setting
    //widget changes.  This ensures that we can always refer to the settings object, not the widget value, when we need
    //a setting.
    connect(ui->stateCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->firstRandomStateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->lastRandomStateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->pixelWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->pixelHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->cellSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->gridBufferSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->startingDirectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->startingColumnSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->startingRowSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->showCounterCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->counterLocationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->showRulesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->rulesLocationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->colorAntCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->stepsPerUpdateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->delayPerUpdateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->stepsPerSampleSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->samplesPerFrameSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->frameCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->saveZeroFrameCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->searchStepsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidgets()));
    connect(ui->includeBackCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidgets()));

    //Connections for updating labels in settings
    connect(ui->pixelWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateImageLabels()));
    connect(ui->pixelHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateImageLabels()));
    connect(ui->cellSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateImageLabels()));
    connect(ui->stepsPerSampleSpinBox, SIGNAL(valueChanged(int)), this, SLOT(renderSettingsChanged()));
    connect(ui->samplesPerFrameSpinBox, SIGNAL(valueChanged(int)), this, SLOT(renderSettingsChanged()));
    connect(ui->frameCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(renderSettingsChanged()));

    //Connections for buttons in settings
    connect(ui->randomizeColorsButton, SIGNAL(clicked()), this, SLOT(randomizeColors()));
    connect(ui->randomizeColorOrderButton, SIGNAL(clicked()), this, SLOT(randomizeColorOrder()));
    connect(ui->startInCenterButton, SIGNAL(clicked()), this, SLOT(moveStartToCenter()));
    connect(ui->counterFontButton, SIGNAL(clicked()), this, SLOT(fontButtonPushed()));
    connect(ui->colorAntButton, SIGNAL(clicked()), this, SLOT(antColorButtonPushed()));

    //Connections for check boxes in settings
    connect(ui->showCounterCheckBox, SIGNAL(stateChanged(int)), this, SLOT(redrawImage()));
    connect(ui->showRulesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(redrawImage()));
    connect(ui->colorAntCheckBox, SIGNAL(stateChanged(int)), this, SLOT(drawAntSquareAndRefreshImage()));

    //Connections for spin boxes in settings
    connect(ui->firstRandomStateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(firstRandomChanged()));
    connect(ui->lastRandomStateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(lastRandomChanged()));
    connect(ui->delayPerUpdateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(delayPerUpdateChanged()));
    connect(ui->cellSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(cellSizeChanged()));
    connect(ui->pixelWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(imageSizeChanged()));
    connect(ui->pixelHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(imageSizeChanged()));
    connect(ui->gridBufferSpinBox, SIGNAL(valueChanged(int)), this, SLOT(bufferSizeChanged()));
    connect(ui->startingRowSpinBox, SIGNAL(valueChanged(int)), this, SLOT(resetToStart()));
    connect(ui->startingColumnSpinBox, SIGNAL(valueChanged(int)), this, SLOT(resetToStart()));

    //Connections for combo boxes in settings
    connect(ui->startingDirectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(resetToStart()));
    connect(ui->counterLocationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(redrawImage()));
    connect(ui->rulesLocationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(redrawImage()));

    //Connections for timers
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateOnce()));
    connect(&timerToHDD, SIGNAL(timeout()), this, SLOT(makeOneSample()));
    connect(&timerForSearch, SIGNAL(timeout()), this, SLOT(oneSearch()));
}




//This function updates each setting widget with the value from the settings object.  It is called when the program
//starts up and when a settings file is loaded.
void MainWindow::updateWidgetsFromSettings()
{

    //Block the signals from all settings widgets.  This is to ensure that when their
    //values are changed they don't trigger the updateSettingsFromWidgets function.
    ui->stateCountSpinBox->blockSignals(true);
    ui->firstRandomStateSpinBox->blockSignals(true);
    ui->lastRandomStateSpinBox->blockSignals(true);
    ui->pixelWidthSpinBox->blockSignals(true);
    ui->pixelHeightSpinBox->blockSignals(true);
    ui->cellSizeSpinBox->blockSignals(true);
    ui->gridBufferSpinBox->blockSignals(true);
    ui->startingDirectionComboBox->blockSignals(true);
    ui->startingColumnSpinBox->blockSignals(true);
    ui->startingRowSpinBox->blockSignals(true);
    ui->showCounterCheckBox->blockSignals(true);
    ui->counterLocationComboBox->blockSignals(true);
    ui->showRulesCheckBox->blockSignals(true);
    ui->rulesLocationComboBox->blockSignals(true);
    ui->colorAntCheckBox->blockSignals(true);
    ui->stepsPerUpdateSpinBox->blockSignals(true);
    ui->delayPerUpdateSpinBox->blockSignals(true);
    ui->stepsPerSampleSpinBox->blockSignals(true);
    ui->samplesPerFrameSpinBox->blockSignals(true);
    ui->frameCountSpinBox->blockSignals(true);
    ui->saveZeroFrameCheckBox->blockSignals(true);
    ui->searchStepsSpinBox->blockSignals(true);
    ui->includeBackCheckBox->blockSignals(true);

    //Before settings are loaded into the widgets, adjust the min and max values for the random range spin boxes.
    //This is to make sure that the min or max values don't prevent a setting from loading and to make sure they
    //behave well for the user right after loading.
    ui->lastRandomStateSpinBox->setMaximum(settings.stateCount);
    ui->lastRandomStateSpinBox->setMinimum(settings.firstRandom);
    ui->firstRandomStateSpinBox->setMaximum(settings.lastRandom);

    //Change the values in the widgets by using the settings object
    ui->stateCountSpinBox->setValue(settings.stateCount);
    ui->firstRandomStateSpinBox->setValue(settings.firstRandom);
    ui->lastRandomStateSpinBox->setValue(settings.lastRandom);
    ui->pixelWidthSpinBox->setValue(settings.pixelWidth);
    ui->pixelHeightSpinBox->setValue(settings.pixelHeight);
    ui->cellSizeSpinBox->setValue(settings.cellSize);
    ui->gridBufferSpinBox->setValue(settings.gridBuffer);
    ui->startingDirectionComboBox->setCurrentIndex(settings.startingDirection);
    ui->startingColumnSpinBox->setValue(settings.startingColumn);
    ui->startingRowSpinBox->setValue(settings.startingRow);
    ui->showCounterCheckBox->setChecked(settings.showCounter);
    ui->counterLocationComboBox->setCurrentIndex(settings.counterLocation);
    ui->showRulesCheckBox->setChecked(settings.showRules);
    ui->rulesLocationComboBox->setCurrentIndex(settings.rulesLocation);
    ui->colorAntCheckBox->setChecked(settings.showAntColor);
    const QString COLOR_STYLE("QPushButton { background-color : %1 }");
    ui->colorAntButton->setStyleSheet(COLOR_STYLE.arg(settings.antColor.name()));
    ui->stepsPerUpdateSpinBox->setValue(settings.stepsPerUpdate);
    ui->delayPerUpdateSpinBox->setValue(settings.delayPerUpdate);
    ui->stepsPerSampleSpinBox->setValue(settings.stepsPerSample);
    ui->samplesPerFrameSpinBox->setValue(settings.samplesPerFrame);
    ui->frameCountSpinBox->setValue(settings.frameCount);
    ui->saveZeroFrameCheckBox->setChecked(settings.saveZeroFrame);
    ui->searchStepsSpinBox->setValue(settings.searchSteps);
    ui->includeBackCheckBox->setChecked(settings.includeBack);

    //Now that the settings are loaded, update the labels in the settings.
    updateImageLabels();
    renderSettingsChanged();

    //The signals can now be unblocked.
    ui->stateCountSpinBox->blockSignals(false);
    ui->firstRandomStateSpinBox->blockSignals(false);
    ui->lastRandomStateSpinBox->blockSignals(false);
    ui->pixelWidthSpinBox->blockSignals(false);
    ui->pixelHeightSpinBox->blockSignals(false);
    ui->cellSizeSpinBox->blockSignals(false);
    ui->gridBufferSpinBox->blockSignals(false);
    ui->startingDirectionComboBox->blockSignals(false);
    ui->startingColumnSpinBox->blockSignals(false);
    ui->startingRowSpinBox->blockSignals(false);
    ui->showCounterCheckBox->blockSignals(false);
    ui->counterLocationComboBox->blockSignals(false);
    ui->showRulesCheckBox->blockSignals(false);
    ui->rulesLocationComboBox->blockSignals(false);
    ui->colorAntCheckBox->blockSignals(false);
    ui->stepsPerUpdateSpinBox->blockSignals(false);
    ui->delayPerUpdateSpinBox->blockSignals(false);
    ui->stepsPerSampleSpinBox->blockSignals(false);
    ui->samplesPerFrameSpinBox->blockSignals(false);
    ui->frameCountSpinBox->blockSignals(false);
    ui->saveZeroFrameCheckBox->blockSignals(false);
    ui->searchStepsSpinBox->blockSignals(false);
    ui->includeBackCheckBox->blockSignals(false);
}





//This function updates each setting in the settings object with the value in the widgets.  It is called whenever a
//setting widget is changed.  This is not the most efficient approach, as every setting will be updated when a single
//one has changed.  However, it greatly simplifies the code, as the the alternative would be to make a function for
//each and every setting.
void MainWindow::updateSettingsFromWidgets()
{
    settings.stateCount = ui->stateCountSpinBox->value();
    settings.firstRandom = ui->firstRandomStateSpinBox->value();
    settings.lastRandom = ui->lastRandomStateSpinBox->value();
    settings.pixelWidth = ui->pixelWidthSpinBox->value();
    settings.pixelHeight = ui->pixelHeightSpinBox->value();
    settings.cellSize = ui->cellSizeSpinBox->value();
    settings.gridBuffer = ui->gridBufferSpinBox->value();
    settings.startingDirection = ui->startingDirectionComboBox->currentIndex();
    settings.startingColumn = ui->startingColumnSpinBox->value();
    settings.startingRow = ui->startingRowSpinBox->value();
    settings.showCounter = ui->showCounterCheckBox->isChecked();
    settings.counterLocation = ui->counterLocationComboBox->currentIndex();
    settings.showRules = ui->showRulesCheckBox->isChecked();
    settings.rulesLocation = ui->rulesLocationComboBox->currentIndex();
    settings.showAntColor = ui->colorAntCheckBox->isChecked();
    settings.stepsPerUpdate = ui->stepsPerUpdateSpinBox->value();
    settings.delayPerUpdate = ui->delayPerUpdateSpinBox->value();
    settings.stepsPerSample = ui->stepsPerSampleSpinBox->value();
    settings.samplesPerFrame = ui->samplesPerFrameSpinBox->value();
    settings.frameCount = ui->frameCountSpinBox->value();
    settings.saveZeroFrame = ui->saveZeroFrameCheckBox->isChecked();
    settings.searchSteps = ui->searchStepsSpinBox->value();
    settings.includeBack = ui->includeBackCheckBox->isChecked();
}





//This function updates the labels in the image size settings when one of the settings they depend on changes.
void MainWindow::updateImageLabels()
{
    //For setting the labels in the settings, we do the calculations with doubles so the user can see
    //what proportion of a partial square is visible.
    QString visibleColumns, visibleRows;
    visibleColumns.setNum(double(settings.pixelWidth) / double(settings.cellSize), 'g', 6);
    visibleRows.setNum(double(settings.pixelHeight) / double(settings.cellSize), 'g', 6);

    ui->visibleColumnsLabel->setText(visibleColumns);
    ui->visibleRowsLabel->setText(visibleRows);
}





//This function updates the labels in the render to HDD settings when one of the settings they depend on changes.
void MainWindow::renderSettingsChanged()
{
    QString stepsPerFrame, stepsPerSecond, totalSteps;
    stepsPerFrame.setNum(settings.stepsPerSample * settings.samplesPerFrame);
    stepsPerSecond.setNum(settings.stepsPerSample * settings.samplesPerFrame * 30);
    totalSteps.setNum(settings.stepsPerSample * settings.samplesPerFrame * settings.frameCount);

    ui->stepsPerFrameLabel->setText(stepsPerFrame);
    ui->stepsPerSecondLabel->setText(stepsPerSecond);
    ui->totalStepsLabel->setText(totalSteps);
}




//This function moves the ant's starting position settings to the grid square closest to the center of the image.
void MainWindow::moveStartToCenter()
{
    //determine how many columns and rows are visible
    double visibleColumns = double(settings.pixelWidth) / double(settings.cellSize);
    double visibleRows = double(settings.pixelHeight) / double(settings.cellSize);

    //find the center by halving the visible columns and then rounding to a whole number
    int newStartingColumn = int(visibleColumns/2.0 + 0.5);
    int newStartingRow = int(visibleRows/2.0 + 0.5);

    //set the widgets to the new values (the settings object's values will then be changed automatically due to connections)
    ui->startingColumnSpinBox->setValue(newStartingColumn);
    ui->startingRowSpinBox->setValue(newStartingRow);

    //SMALL ISSUE: WHEN EITHER THE STARTING COLUMN OR STARTING ROW CHANGES, IT TRIGGERS THE resetToStart SLOT.  SO WHEN
    //THIS FUNCTION SETS BOTH, BOTH OF THEM WILL TRIGGER IT AND THE SLOT WILL BE DONE TWICE.  I DON'T THINK THERE'S ANY
    //HARM - IT'S JUST INEFFICIENT.

}





void MainWindow::saveSettings()
{
    //Construct a default file name
    QString defaultFileName = makeFileName();

    //Call up a modal save file dialog to get a filename and path from the user
    QString fileName = QFileDialog::getSaveFileName(this, "Save Settings", defaultFileName, "Langton's Ant Settings (*.las)");

    //If the user didn't hit cancel, call the "save to file" function of the settings object
    if ( !(fileName == "") )
        settings.saveToFile(fileName, stateArray);
}





void MainWindow::loadSettings()
{
    //Call up a modal load file dialog to get a filename and path from the user
    QString fileName = QFileDialog::getOpenFileName(this, "Load Settings", "", "Langton's Ant Settings (*.las)");

    //If the user didn't hit cancel, call the "load from file" function of the settings object
    if ( !(fileName == "") )
    {
        settings.loadFromFile(fileName, &stateArray);
        updateWidgetsFromSettings();

        //The loadFromFile function will delete and recreate the stateWidget array.
        //It is therefore now necessary to display the newly created widgets.

        //Remove the spacer from the layout
        ui->antStatesHorizontalLayout->removeItem(spacer);

        //Add the new widgets to the layout
        for (int i = 0; i < settings.stateCount; i++)
            ui->antStatesHorizontalLayout->addWidget(stateArray+i);

        //Add the spacer back to the layout
        ui->antStatesHorizontalLayout->addSpacerItem(spacer);

        //The AntGrid and AntCounter objects now need to know the location of the new stateArray, so pass them the new pointer.
        antGrid->updateStateArrayPointer(stateArray);
        antCounter->updateStateArrayPointer(stateArray);

        //Assume the image size changed - this function will recreate and redraw the image
        imageSizeChanged();

    }
}





void MainWindow::changeStateCount(int newCount)
{
    //If the on-screen animation is running, stop it now.
    if (timerRunning)
        stopTimer();

    //By making the scroll area's contents not visible for this function, things look a bit less jittery as the
    //StateWidget objects are made and added to the scroll area.
    ui->stateScrollAreaWidgetContents->setVisible(false);

    //If the new size is the same as the old size, there is nothing to do - just quit.
    //I'm not sure why this would ever be the case, but I'm putting this in just in case it does happen for some reason.
    if (newCount==settings.stateCount)
        return;

    //Create a new StateWidget array of the new size.
    StateWidget * newArray = new StateWidget [newCount];

    //Copy the contents of the existing array into the new one.
    if (newCount<settings.stateCount)
    {
        //there are less states than there used to be

        //initialize all the new states using the values from the equivalent old states
        for (int i = 0; i < newCount; i++)
        {
            newArray[i].initialize(i+1, stateArray[i].direction, stateArray[i].color);
        }
    }
    else
    {
        //there are more states than there used to be

        //initialize as many new states as possible using the values from the equivalent old states
        for (int i = 0; i < settings.stateCount; i++)
        {
            newArray[i].initialize(i+1, stateArray[i].direction, stateArray[i].color);
        }
        //initialize the remaining new states using the value from the last old state
        for (int i = settings.stateCount; i < newCount; i++)
        {
            newArray[i].initialize(i+1, stateArray[settings.stateCount-1].direction, stateArray[settings.stateCount-1].color);
        }

    }

    //Delete the existing array.  This will also serve to remove the existing widgets from the layout.
    delete [] stateArray;

    //Remove the spacer from the layout
    ui->antStatesHorizontalLayout->removeItem(spacer);

    //set the state count to the new count
    settings.stateCount = newCount;

    //set the stateArray pointer to the new array
    stateArray = newArray;

    //Add the new widgets to the layout and create their connection too.  This connection is so they cause MainWindow
    //to redraw the image when their colors are changed.
    for (int i = 0; i < settings.stateCount; i++)
    {
        ui->antStatesHorizontalLayout->addWidget(stateArray+i);
        connect(&(stateArray[i]), SIGNAL(colorChanged()), this, SLOT(redrawImage()));
        connect(&(stateArray[i]), SIGNAL(directionChanged()), this, SLOT(redrawImage()));
    }

    //Add the spacer back to the layout
    ui->antStatesHorizontalLayout->addSpacerItem(spacer);

    //Now that everything visual is done, make the scroll area's contents visible again.
    ui->stateScrollAreaWidgetContents->setVisible(true);

    //Unrelated to the states themselves, it is necessary to set the random ranges spin boxes to have a maximum of the
    //new state count.  This is to prevent a user from trying to randomize states that don't exist.
    ui->lastRandomStateSpinBox->setMaximum(settings.stateCount);

    //The AntGrid and AntCounter objects now need to know the location of the new stateArray, so pass them the new pointer.
    antGrid->updateStateArrayPointer(stateArray);
    antCounter->updateStateArrayPointer(stateArray);

    //Reset everything to time 0
    resetToStart();
}





//This function randomizes the colors for the states in the selected random range.
void MainWindow::randomizeColors()
{
    int randomRed, randomGreen, randomBlue;

    for (int i = settings.firstRandom; i <=settings.lastRandom; i++)
    {
        randomRed = randNum()%256;
        randomGreen = randNum()%256;
        randomBlue = randNum()%256;
        stateArray[i-1].changeColor(QColor(randomRed, randomGreen, randomBlue));
    }

    //Redraw the image so the change can be seen.
    redrawImage();
}





//This function shuffles the order of the colors for the states in the selected random range.  It does
//not change directions, just colors.
void MainWindow::randomizeColorOrder()
{

    int numberOfColorsToShuffle = settings.lastRandom - settings.firstRandom + 1;
    int randomChoice;
    int j;

    //create an array to store the colors
    QColor * colorsToShuffle = new QColor [numberOfColorsToShuffle];
    for (int g=0; g < numberOfColorsToShuffle; g++)
        colorsToShuffle[g]=stateArray[settings.firstRandom-1+g].color; //-1 is because the array starts at 0 but the states are numbered starting at 1

    //make a corresponding array to track which of the colors in that array have been reassigned
    bool * colorUsed = new bool [numberOfColorsToShuffle];
    for (int h=0; h < numberOfColorsToShuffle; h++)
        colorUsed[h]=false;

    //make a variable to track how many colors are left to be reassigned
    int colorsLeft = numberOfColorsToShuffle;

    //reassign the colors one at a time
    for (int i=0; i < numberOfColorsToShuffle; i++)
    {
        randomChoice = randNum()%colorsLeft;

        for (j=0; ; j++) //will loop until it encounters a break
        {
            if ( (randomChoice == 0)&&( !(colorUsed[j]) ) )
               break;
            else if ( !(colorUsed[j]) )
               randomChoice--;
        }

        //stateArray[settings.firstRandom-1+i] is now the recipient of the randomly-chosen color at colorsToShuffle[j]
        stateArray[settings.firstRandom-1+i].changeColor(colorsToShuffle[j]);
        colorUsed[j]=true;

        //decrement colorsLeft
        colorsLeft--;
    }

    //clean up the dynamic memory
    delete [] colorsToShuffle;
    delete [] colorUsed;

    //Redraw the image so the change can be seen.
    redrawImage();
}





//The following two functions are slots that are executed when one of the random ranges is changed.  They serve to
//adjust the min and max of the random range spinboxes to make sure that the ranges make sense, i.e. the min is always
//less than or equal to the max.
void MainWindow::firstRandomChanged()
{
    ui->lastRandomStateSpinBox->setMinimum(settings.firstRandom);
}
void MainWindow::lastRandomChanged()
{
    ui->firstRandomStateSpinBox->setMaximum(settings.lastRandom);
}



//This function redraws the entire image.  It is used after the user makes some change such as resetting the counter or
//changing a color.
void MainWindow::redrawImage()
{

    //This code loops through all of the squares and redraws them according to their state.
    for (int i=0; i<displayGrid->columnCount; i++)
    {
        for (int j=0; j<displayGrid->rowCount; j++)
            displayGrid->drawSquare(i, j, stateArray[antGrid->getState(i, j)].color);
    }

    //Now draw the ant too if that setting is on.
    if (settings.showAntColor)
        antGrid->drawAntSquare();

    //If we are showing the counter or rules, draw them onto the image now
    if ( (settings.showCounter)||(settings.showRules) )
        antCounter->paintCountAndRules();

    //Make the redrawn image visible on the label.
    gridLabel->setPixmap(QPixmap::fromImage( *(displayGrid->gridImage)) );
    gridLabel->resize(gridLabel->pixmap()->size());
}





void MainWindow::cellSizeChanged()
{
    //Set the new square size in the grid.  This will clear the image to the passed color.
    displayGrid->changeSquareSize(settings.cellSize, stateArray[0].color);

    //We should now set the maximum values on the starting positions to be within the image.
    ui->startingColumnSpinBox->setMaximum(displayGrid->columnCount - 1);
    ui->startingRowSpinBox->setMaximum(displayGrid->rowCount - 1);

    //Reset the ant grid to the beginning and recreate its state array to account for the new sizes
    resetToStartAndRemakeAntGrid();

    //If we are showing the counter or rules, draw them onto the image now
    if ( (settings.showCounter)||(settings.showRules) )
        antCounter->paintCountAndRules();

    //Make the cleared image visible on the label.
    gridLabel->setPixmap(QPixmap::fromImage( *(displayGrid->gridImage)) );
}





void MainWindow::imageSizeChanged()
{
    //Set the new image size in the grid.  This will clear the image to the passed color.
    displayGrid->changeImageSize(settings.pixelWidth, settings.pixelHeight, settings.cellSize, stateArray[0].color);

    //We should now set the maximum values on the starting positions to be within the image.
    ui->startingColumnSpinBox->setMaximum(displayGrid->columnCount - 1);
    ui->startingRowSpinBox->setMaximum(displayGrid->rowCount - 1);

    //Reset the ant grid to the beginning and recreate its state array to account for the new sizes
    resetToStartAndRemakeAntGrid();

    //If we are showing the counter or rules, draw them onto the image now
    if ( (settings.showCounter)||(settings.showRules) )
        antCounter->paintCountAndRules();

    //Make the cleared image visible on the label.
    gridLabel->setPixmap(QPixmap::fromImage( *(displayGrid->gridImage)) );
    gridLabel->resize(gridLabel->pixmap()->size());


}





void MainWindow::bufferSizeChanged()
{
    //Changing the square size will reset the ant grid to the beginning
    resetToStartAndRemakeAntGrid();

    //Reset the image to the first color
    displayGrid->fillImage(stateArray[0].color);

    //Now draw the ant if that setting is on.
    if (settings.showAntColor)
        antGrid->drawAntSquare();

    //If we are showing the counter or rules, draw them onto the image now
    if ( (settings.showCounter)||(settings.showRules) )
        antCounter->paintCountAndRules();

    //Make the cleared image visible on the label.
    gridLabel->setPixmap(QPixmap::fromImage( *(displayGrid->gridImage)) );
}





//This function resets the simulation to the start: the time goes to zero, all states go to zero and the
//ant moves back to the starting position.  Notably, this function does not actually recreate the AntGrid
//object's arrays, so if the image, cell or buffer sizes change, it is necessary to call the next function,
//resetToStartAndRemakeAntGrid, instead.
void MainWindow::resetToStart()
{
    //Stop the on-screen animation if it is running.  There shouldn't be a need to stop the animation to disk,
    //because the UI elements that would trigger this function should be disabled during animation to disk.
    if (timerRunning)
        stopTimer();

    //Reset the time to zero
    settings.time = 0;
    updateTimeLabel();

    //Reset the counter-painting object
    antCounter->reset();

    //Reset the image to the first color
    displayGrid->fillImage(stateArray[0].color);

    //If we are showing the counter or rules, draw them onto the image now
    if ( (settings.showCounter)||(settings.showRules) )
        antCounter->paintCountAndRules();

    //Reset all of the states on the antGrid to zero and move the ant back to the starting position.  If drawing the
    //ant is enabled, this will also draw the ant at the starting position.
    antGrid->resetGrid();

    //Make the reset image visible on the label.
    gridLabel->setPixmap(QPixmap::fromImage( *(displayGrid->gridImage)) );

    //Clear the status bar
    ui->statusBar->clearMessage();
}





//This function resets the simulation to the start: the time goes to zero, all states go to zero and the
//ant moves back to the starting position.  It also recreates the AntGrid object's arrays, so it is used
//when the image, cell or buffer sizes change.
void MainWindow::resetToStartAndRemakeAntGrid()
{
    //Stop the on-screen animation if it is running.  There shouldn't be a need to stop the animation to disk,
    //because the UI elements that would trigger this function should be disabled during animation to disk.
    if (timerRunning)
        stopTimer();

    //Reset the time to zero
    settings.time = 0;
    updateTimeLabel();

    //Reset the counter-painting object
    antCounter->reset();

    //This function will recreate the antGrid, reset all of the states to zero and move the ant back
    //to the starting position.
    antGrid->resizeGrid();

    //This function does not clear and redisplay the image - that is up to the code that calls this
    //function.  The reason why is that in some cases (e.g. the image size changing), the entire image
    //has to be remade.  In other cases (e.g. the buffer size changing), the image just needs to be
    //cleared.
}





void MainWindow::updateOnce()
{
    //Move the ant!!!!
    antGrid->moveAnt(settings.stepsPerUpdate, true);

    //If we are showing the counter or rules, draw them onto the image now
    if ( (settings.showCounter)||(settings.showRules) )
        antCounter->paintCountAndRules();

    //Make the updated image visible on the label.
    gridLabel->setPixmap(QPixmap::fromImage( *(displayGrid->gridImage)));

    //Update the status bar
    updateTimeLabel();
    if (antGrid->outOfRange)
        ui->statusBar->showMessage("Out of range - simulation stopped");

}




void MainWindow::delayPerUpdateChanged()
{
    timer.setInterval(settings.delayPerUpdate);
}





void MainWindow::renderToScreenStartStop()
{
    //If the animation is running, stop it.
    if (timerRunning)
        stopTimer();

    //If the animation is not running, start it.
    else
        startTimer();
}




void MainWindow::startTimer()
{
    //Display a message in the status bar
    ui->statusBar->showMessage("Rendering animation to screen...");

    ui->actionStartAnimation->setIcon(QIcon(":/icons/images/stop64.png"));
    ui->actionStartAnimation->setText("Stop Animation");

    timerRunning = true;
    timer.start();
}
void MainWindow::stopTimer()
{
    //Display a message in the status bar
    ui->statusBar->showMessage("Animation to screen stopped");

    ui->actionStartAnimation->setIcon(QIcon(":/icons/images/renderred64.png"));
    ui->actionStartAnimation->setText("Start Animation");

    timerRunning = false;
    timer.stop();
}





void MainWindow::makeOneSample()
{
    //Move the ant!!!!
    antGrid->moveAnt(settings.stepsPerSample, true);

    //If we are showing the counter or rules, draw them onto the image now
    if ( (settings.showCounter)||(settings.showRules) )
        antCounter->paintCountAndRules();

    //Add the updated image to the blender
    imageBlender->addImage( *(displayGrid->gridImage), currentSample);

    //Increment the current sample
    currentSample++;

    //Quit the function if that wasn't the last sample in the frame
    if (currentSample < settings.samplesPerFrame)
        return;

    //If it was the last sample in the frame...

    //Save the blended image
    blendedImage = imageBlender->blendImages();

    //If the returned image is null, that probably means that the user cancelled the animation.  If that is the
    //case, reset the time, display an appropriate message in the status bar and quit.
    if (blendedImage.isNull())
    {
        resetToStart();
        ui->statusBar->showMessage("Animation to disk stopped");
        return;
    }

    //Display the blended image on the screen
    gridLabel->setPixmap(QPixmap::fromImage(blendedImage));

    //Call a function to do the actual saving
    saveFrameToHDD();

    //Reset the current sample and increment the current frame
    currentSample = 0;
    currentFrame++;

    //If this was the last frame in the animation, stop the timer
    if (currentFrame>settings.frameCount)
        stopTimerToHDD(false);

}





void MainWindow::renderToHDDStartStop()
{
    //If the animation is running, stop it.
    if (timerToHDDRunning)
        stopTimerToHDD(true);

    //If the animation is not running, start it.
    else
        startTimerToHDD();
}





void MainWindow::startTimerToHDD()
{
    //First prompt the user for a place to save the files
    filePath = QFileDialog::getExistingDirectory(this, tr("Directory to Save Frames"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    //Quit if the user hit cancel in the file path dialog
    if (filePath == "")
        return;

    //Display a message in the status bar
    ui->statusBar->showMessage("Rendering animation to disk...");

    //Disable UI stuff so the user can't screw with things when the render is occuring
    ui->settingsDockWidget->setEnabled(false);
    ui->stateDockWidget->setEnabled(false);
    ui->actionLoadSettings->setEnabled(false);
    ui->actionStartAnimation->setEnabled(false);
    ui->actionUpdateOnce->setEnabled(false);
    ui->actionResetAnimation->setEnabled(false);

    //Reset the time to zero
    resetToStart();

    //Set the sample and frame to zero
    currentSample = 0; //sample starts at 0 to line up with C++ arrays
    currentFrame = 0; //frame starts at 1 to line up with user values

    //Now initialise the image blender to hold the samples and create the frames
    imageBlender->initialize(settings.samplesPerFrame);

    ui->actionRenderAnimationToHDD->setIcon(QIcon(":/icons/images/stophdd64.png"));
    ui->actionRenderAnimationToHDD->setText("Stop Animation to HDD");

    //If the setting to save a zero frame is on, save it now
    if (settings.saveZeroFrame)
        saveFrameToHDD();

    //Whether or not a zero frame was saved, we should now set the frame to 1
    currentFrame++;


    timerToHDDRunning = true;
    timerToHDD.start();
}





void MainWindow::stopTimerToHDD(bool forceStop)
{
    ui->actionRenderAnimationToHDD->setIcon(QIcon(":/icons/images/renderhdd64.png"));
    ui->actionRenderAnimationToHDD->setText("Render Animation to HDD");

    timerToHDDRunning = false;
    timerToHDD.stop();

    //Display a finished message in the status bar
    if (forceStop)
        ui->statusBar->showMessage("Animation to disk stopped");
    else
        ui->statusBar->showMessage("Animation to disk finished!");

    //Re-enable UI stuff
    ui->settingsDockWidget->setEnabled(true);
    ui->stateDockWidget->setEnabled(true);
    ui->actionLoadSettings->setEnabled(true);
    ui->actionStartAnimation->setEnabled(true);
    ui->actionUpdateOnce->setEnabled(true);
    ui->actionResetAnimation->setEnabled(true);
}





void MainWindow::updateTimeLabel()
{
    QString timeText = "Time: ";
    timeText += AntCounter::addCommasToNumber(settings.time);
    timeText += " "; //a bit of space so the text doesn't squeeze up too far to the right of the status bar
    timeLabel->setText(timeText);
}





void MainWindow::antColorButtonPushed()
{
    QColor chosenColor = QColorDialog::getColor(settings.antColor, 0, "Choose an Ant Colour");

    if (chosenColor.isValid()) //if the user clicked OK instead of Cancel
    {
        //set the state's color to the chosen color
        settings.antColor = chosenColor;

        //make the button reflect the color
        const QString COLOR_STYLE("QPushButton { background-color : %1 }");
        ui->colorAntButton->setStyleSheet(COLOR_STYLE.arg(settings.antColor.name()));

        //Make the necessary updates to the image
        drawAntSquareAndRefreshImage();
    }
}




//This function updates the image after the user changes one of the ant color settings.  If the ant color is turned
//on, this will draw the ant.  If it is turned off, this will get rid of the ant.  If the ant color is changed, it will
//change the color of the ant.
void MainWindow::drawAntSquareAndRefreshImage()
{
    antGrid->drawAntSquare();

    //Make the reset image visible on the label.
    gridLabel->setPixmap(QPixmap::fromImage( *(displayGrid->gridImage)) );
}





//This function saves a frame to the disk.  It doesn't need any arguments because it gets everything from data members.
void MainWindow::saveFrameToHDD()
{
    //Update the status bar
    updateTimeLabel();
    QString statusBarMessage = "Rendering animation to disk: frame ";
    statusBarMessage = statusBarMessage + QString::number(currentFrame) + " of " + QString::number(settings.frameCount);
    if (antGrid->outOfRange)
        statusBarMessage += "   Out of range - simulation stopped";
    ui->statusBar->showMessage(statusBarMessage);

    //Construct a full file name with path for this frame
    QString fullPath = filePath + QDir::separator() + QString::number(currentFrame).rightJustified(5, '0') + ".png";

    //Save the image to file.  Which image depends on whether this is a frame zero or a blended frame in the animation.
    if (currentFrame == 0)
        displayGrid->gridImage->save(fullPath);
    else
        blendedImage.save(fullPath);
}





void MainWindow::fontButtonPushed()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, settings.counterFont, this);
    if (ok)
    {
        //Save the font in the AntSettings object
        settings.counterFont = font;

        //Reset the max sizes of the AntCounter object.  If the new font is smaller, the max sizes currently set might
        //be way too big.
        antCounter->reset();

        //Redraw the image.  This is necessary because the new font counter might take up less space than the old font
        //counter.  It is therefore necessary to redraw everything to completely get rid of the old font counter.
        redrawImage();
    }
}





void MainWindow::searchAll()
{
    //Set search type to "all".
    searchType = 1;

    //Set up for the search and quit if the setup didn't succeed.
    if ( !setUpForSearch() )
    {
        searchType = 0;
        return;
    }

    //Create and show the search dialog
    searchDialog = new SearchDialog(this, 1, maxPatternCount);
    searchDialog->show();
    connect(searchDialog, SIGNAL(cancelSearch()), this, SLOT(finishSearch()));

    //Start the search timer!
    timerForSearch.start();
}





void MainWindow::searchRandom()
{
    //Set search type to "random".
    searchType = 2;

    //Set up for the search and quit if the setup didn't succeed.
    if ( !setUpForSearch() )
    {
        searchType = 0;
        return;
    }

    //Create and show the search dialog
    searchDialog = new SearchDialog(this, 2, 0);
    searchDialog->show();
    connect(searchDialog, SIGNAL(cancelSearch()), this, SLOT(finishSearch()));

    //Start the search timer!
    timerForSearch.start();
}




//This function does the preparatory work for running a search.  It covers the tasks that are necessary
//for both a random and an all-inclusive search.  It returns true if things went well, false if they didn't.
bool MainWindow::setUpForSearch()
{
    //Check to make sure there are at least 3 states.  Quit with a return value of false if not.
    if (settings.stateCount < 3)
    {
        QMessageBox::warning(this, "Search error", "Three or more states are\nnecessary to run a search.");
        return false;
    }

    //If the search type is "all" - do some extra stuff here
    if (searchType == 1)
    {
        //Quit if the state count is too high!
        if (settings.stateCount > 15)
        {
            QMessageBox::warning(this, "Search error", "The state count must be\n15 or fewer to run an \"all\"\nsearch");
            return false;
        }

        //Set the search pattern index to 0
        patternIndex = 0;

        //Determine the number of possible patterns.
        if (settings.includeBack)
        {
            //This somewhat confusing math is necessary to calculate how many possible patterns there are
            //that do NOT have a left as their first direction.
            maxPatternCount = integerPower(3, settings.stateCount);
            for (int i = settings.stateCount-1; i >= 0; i--)
            {
                maxPatternCount = maxPatternCount - integerPower(3, i);
            }
        }
        else
            maxPatternCount = integerPower(2, settings.stateCount-1);
    }


    //Prompt the user for a save directory
    searchFilePath = QFileDialog::getExistingDirectory(this, tr("Directory to Save Search Results"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    //Quit if the user hit cancel.
    if (searchFilePath == "")
        return false;


    //Change the necessary settings...

    //Move the starting position to the center
    double visibleColumns = double(settings.pixelWidth) / double(settings.cellSize);
    double visibleRows = double(settings.pixelHeight) / double(settings.cellSize);
    settings.startingColumn = int(visibleColumns/2.0 + 0.5);
    settings.startingRow = int(visibleRows/2.0 + 0.5);

    //Make sure that the counter and rules are NOT displayed
    settings.showCounter = false;
    settings.showRules = false;

    //Make sure the starting direction is up
    settings.startingDirection = 0;

    //Turn off the ant's color
    settings.showAntColor = false;

    //Update the widgets so the altered settings can be seen
    updateWidgetsFromSettings();



    //Disable UI stuff so the user can't screw with things when the search is occuring
    ui->settingsDockWidget->setEnabled(false);
    ui->stateDockWidget->setEnabled(false);
    ui->actionLoadSettings->setEnabled(false);
    ui->actionStartAnimation->setEnabled(false);
    ui->actionUpdateOnce->setEnabled(false);
    ui->actionResetAnimation->setEnabled(false);
    ui->actionSaveSettings->setEnabled(false);
    ui->actionRenderAnimationToHDD->setEnabled(false);
    ui->actionSearchAllRules->setEnabled(false);
    ui->actionSearchRandomRules->setEnabled(false);

    //Set the pattern count to 0
    patternCount = 0;

    //If the code got here, all is well!  Return true to indicate a successful setup!
    return true;
}




//This function does concluding work for a search.  It covers the tasks that are necessary
//for both a random and an all-inclusive search.
void MainWindow::finishSearch()
{
    //Stop the search timer
    timerForSearch.stop();

    //delete the search dialog
    delete searchDialog;

    //Set the searchType variable to 0 which means "search not running"
    searchType = 0;

    //Re-enable UI stuff
    ui->settingsDockWidget->setEnabled(true);
    ui->stateDockWidget->setEnabled(true);
    ui->actionLoadSettings->setEnabled(true);
    ui->actionStartAnimation->setEnabled(true);
    ui->actionUpdateOnce->setEnabled(true);
    ui->actionResetAnimation->setEnabled(true);
    ui->actionSaveSettings->setEnabled(true);
    ui->actionRenderAnimationToHDD->setEnabled(true);
    ui->actionSearchAllRules->setEnabled(true);
    ui->actionSearchRandomRules->setEnabled(true);
}



void MainWindow::oneSearch()
{
    if (searchType == 1) //search type of "all"
    {
        //End the search if all the patterns have been checked
        if (patternCount >= maxPatternCount)
        {
            ui->statusBar->showMessage("Pattern search finished!");
            finishSearch();
            return;
        }

        //If the code got here, then the pattern count is still below the max pattern count, so do another one!
        setSequentialStates();
    }

    if (searchType == 2) //search type of "random"
    {
        setRandomStates();
    }

    //Check to see if the pattern's first direction is a left.  If so, reject this pattern and try again.
    if (checkForStartingLeft())
        return;

    //Increment the pattern count
    patternCount++;

    //Reset the ant to time zero
    resetToStart();

    //Move the ant!!!!
    antGrid->moveAnt(settings.searchSteps, false);

    //Update the search dialog
    searchDialog->updatePatternCount(patternCount);

    //Make the updated image visible on the label.
    gridLabel->setPixmap(QPixmap::fromImage( *(displayGrid->gridImage)));

    //Save the image to disk
    QString fullPath = searchFilePath + QDir::separator() + makeFileName() + ".png";
    displayGrid->gridImage->save(fullPath);

}


void MainWindow::setRandomStates()
{
    int randomChoice;

    //Set the direction for state 1.  L is not a possible choice here to avoid mirror images.  This state
    //is always set to white.
    if (settings.includeBack)
    {
        //if including back is on, choose either R or B as the direction
        if (randNum()%2 == 0) //50% chance
            stateArray[0].initialize(1, antRight, QColor(255,255,255));
        else
            stateArray[0].initialize(1, antBack, QColor(255,255,255));
    }
    else
    {
        //if including back is off, the direction has to be R
        stateArray[0].initialize(1, antRight, QColor(255,255,255));
    }

    //Set the direction for the rest of the states.  These get random colors, but state 2 will be set to black later.
    for (int i = 1; i < settings.stateCount; i++)
    {
        if (settings.includeBack)
        {
            //if including back is on, choose R, L or B as the direction
            randomChoice = randNum()%3;

            switch (randomChoice)
            {
            case 0:
                stateArray[i].initialize(i+1, antRight, QColor(randNum()%256,randNum()%256,randNum()%256));
                break;
            case 1:
                stateArray[i].initialize(i+1, antLeft, QColor(randNum()%256,randNum()%256,randNum()%256));
                break;
            case 2:
                stateArray[i].initialize(i+1, antBack, QColor(randNum()%256,randNum()%256,randNum()%256));
                break;
            }
        }
        else
        {
            //if including back is off, choose between R and L
            if (randNum()%2 == 0) //50% chance
                stateArray[i].initialize(i+1, antRight, QColor(randNum()%256,randNum()%256,randNum()%256));
            else
                stateArray[i].initialize(i+1, antLeft, QColor(randNum()%256,randNum()%256,randNum()%256));
        }
    }

    //Now go back and change state 2's color to black
    stateArray[1].changeColor(QColor(0,0,0));

}




QString MainWindow::makeFileName()
{
    //Construct a default file name
    QString defaultFileName = "";

    for (int i=0; i<settings.stateCount; i++)
    {
        switch (stateArray[i].direction)
        {
        case antRight:
            defaultFileName += "R";
            break;
        case antLeft:
            defaultFileName += "L";
            break;
        case antBack:
            defaultFileName += "B";
            break;
        }
    }

    return defaultFileName;
}




//This function uses the patternIndex variable to derive a pattern.  It is used when running an "all" search.
void MainWindow::setSequentialStates()
{
    int tempPatternIndex = patternIndex;
    int choice;

    if (settings.includeBack)
    {
        //Use patternIndex to derive the rules.
        for (int i = settings.stateCount-1; i > 0; i--) //Loop through the states from the last state to the second state.
        {
            choice = tempPatternIndex%3;

            if (choice == 0)
                stateArray[i].initialize(i+1, antRight, QColor(randNum()%256,randNum()%256,randNum()%256));
            if (choice == 1)
                stateArray[i].initialize(i+1, antBack, QColor(randNum()%256,randNum()%256,randNum()%256));
            if (choice == 2)
                stateArray[i].initialize(i+1, antLeft, QColor(randNum()%256,randNum()%256,randNum()%256));

            tempPatternIndex = tempPatternIndex - tempPatternIndex%3;
            tempPatternIndex = tempPatternIndex/3;
        }

        //Now go back and change state 2's color to black
        stateArray[1].changeColor(QColor(0,0,0));

        //Now set state 1
        if (tempPatternIndex%2 == 0)
            stateArray[0].initialize(1, antRight, QColor(255,255,255));
        else
            stateArray[0].initialize(1, antBack, QColor(255,255,255));


    }
    else
    {
        //if including back is off, the first direction has to be R
        stateArray[0].initialize(1, antRight, QColor(255,255,255));

        //For the rest of the directions, use patternIndex to derive the rules.
        for (int i = settings.stateCount-1; i > 0; i--) //Loop through the states from the last state to the second state.
        {
            if (tempPatternIndex%2 == 0)
                stateArray[i].initialize(i+1, antRight, QColor(randNum()%256,randNum()%256,randNum()%256));
            else
                stateArray[i].initialize(i+1, antLeft, QColor(randNum()%256,randNum()%256,randNum()%256));

            tempPatternIndex = tempPatternIndex - tempPatternIndex%2;
            tempPatternIndex = tempPatternIndex/2;
        }

        //Now go back and change state 2's color to black
        stateArray[1].changeColor(QColor(0,0,0));
    }

    //Increment patternIndex by one so the next pattern is different.
    patternIndex++;
}





//There isn't an integer power function in C++, so I had to make my own.
int MainWindow::integerPower(int x, int p)
{
  if (p == 0)
      return 1;
  if (p == 1)
      return x;
  return x * integerPower(x, p-1);
}





//This function checks to see if the first direction in the pattern is a left.  If so, it returns true.  It is
//used to prevent mirror-image duplicates when doing pattern searches.  For example, BRRL and BLLR are the same
//pattern, just mirror images.
bool MainWindow::checkForStartingLeft()
{
    AntDirection firstDirection = antBack;
    for (int i = 0; i < settings.stateCount; i++) //loop through each state
    {
        if (stateArray[i].direction == antRight)
        {
            firstDirection = antRight;
            break;
        }
        if (stateArray[i].direction == antLeft)
        {
            firstDirection = antLeft;
            break;
        }
    }
    if (firstDirection == antLeft)
        return true;

    return false;
}




void MainWindow::saveImage()
{
    QString defaultFileName = makeFileName() + ".png";

    //get a filename and location from the user by using the file dialog
    QString fileName = QFileDialog::getSaveFileName(this, "Save Image", defaultFileName, "PNG Image (*.png);;JPEG Image (*.jpg);;BMP Image (*.bmp)");

    //If the user didn't hit cancel, save the file
    if ( !(fileName == "") )
        displayGrid->gridImage->save(fileName);
}
