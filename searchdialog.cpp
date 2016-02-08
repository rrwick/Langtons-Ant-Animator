#include "searchdialog.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(QWidget *parent, int searchType, int maxPatternCount) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);

    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonPushed()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));

    //Set up for an "all" search
    if (searchType == 1)
    {
        ui->randomSearchLabel->setVisible(false);
        ui->progressBar->setMaximum(maxPatternCount);
    }

    //Set up for a random search
    if (searchType == 2)
    {
        ui->allSearchLabel->setVisible(false);
        ui->progressBar->setVisible(false);
    }

    //Set the size to be fixed
    layout()->setSizeConstraint(QLayout::SetFixedSize);

}

SearchDialog::~SearchDialog()
{
    delete ui;
}


void SearchDialog::cancelButtonPushed()
{
    emit cancelSearch();
}




void SearchDialog::updatePatternCount(int newPatternCount)
{
    QString newCountLabel = QString::number(newPatternCount);

    //If this is an "all" search, add the "out of" to the label
    if (ui->progressBar->isVisible()) //the progress bar is only visible in "all" searches
    {
        newCountLabel += " out of ";
        newCountLabel += QString::number(ui->progressBar->maximum());
    }

    ui->patternCountLabel->setText(newCountLabel);
    ui->progressBar->setValue(newPatternCount);
}
