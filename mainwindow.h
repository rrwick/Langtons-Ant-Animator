#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <random>
#include <ctime>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>

#include "antsettings.h"
#include "statewidget.h"
#include "antdirection.h"
#include "grid.h"
#include "antgrid.h"
#include "imageblender.h"
#include "antcounter.h"
#include "searchdialog.h"

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void updateWidgetsFromSettings();
    void updateSettingsFromWidgets();
    void updateImageLabels();
    void renderSettingsChanged();
    void moveStartToCenter();
    void saveSettings();
    void loadSettings();
    void changeStateCount(int newCount);
    void randomizeColors();
    void randomizeColorOrder();
    void firstRandomChanged();
    void lastRandomChanged();
    void cellSizeChanged();
    void imageSizeChanged();
    void bufferSizeChanged();
    void resetToStart();
    void resetToStartAndRemakeAntGrid();
    void redrawImage();
    void updateOnce();
    void delayPerUpdateChanged();
    void renderToScreenStartStop();
    void makeOneSample();
    void renderToHDDStartStop();
    void antColorButtonPushed();
    void drawAntSquareAndRefreshImage();
    void fontButtonPushed();
    void searchAll();
    void searchRandom();
    void oneSearch();
    void finishSearch();
    void saveImage();

private:
    Ui::MainWindow *ui;
    AntSettings settings;

    void setUpConnections();
    void startTimer();
    void stopTimer();
    void startTimerToHDD();
    void stopTimerToHDD(bool forceStop);
    void updateTimeLabel();
    void saveFrameToHDD();
    bool setUpForSearch();
    void setRandomStates();
    void setSequentialStates();
    QString makeFileName();
    int integerPower(int x, int p);
    bool checkForStartingLeft();

    //The array of state widgets to be displayed at the top of the screen
    StateWidget * stateArray;

    //The spacer that helps the StateWidget objects to be displayed nicely
    QSpacerItem * spacer;

    //The random number generator
    std::default_random_engine randNum;

    //The grid that will show all of the interesting stuff!
    Grid * displayGrid;

    //The grid that will actually keep track of the ant
    AntGrid * antGrid;

    //The label that will hold the image in antGrid
    QLabel * gridLabel;

    //The scroll area that will hold the label
    QScrollArea * gridScrollArea;

    //The timer that will trigger updates when the user plays the ant
    QTimer timer;
    bool timerRunning;

    //The following pieces are for rendering an animation to HDD
    QTimer timerToHDD;
    bool timerToHDDRunning;
    QString filePath;
    int currentSample;
    int currentFrame;
    ImageBlender * imageBlender;
    QImage blendedImage;

    //This label will display the time in the status bar
    QLabel * timeLabel;

    //This object will display the counter on the image
    AntCounter * antCounter;

    //These members are for pattern searches
    QTimer timerForSearch;
    int searchType; //0 means "not running", 1 means "all" and 2 means "random"
    SearchDialog * searchDialog;
    QString searchFilePath;
    int patternIndex;
    int patternCount;
    int maxPatternCount;

};

#endif // MAINWINDOW_H
