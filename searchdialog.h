#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QtWidgets/QDialog>

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SearchDialog(QWidget *parent, int searchType, int maxPatternCount);
    ~SearchDialog();
    void updatePatternCount(int newPatternCount);
    
private:
    Ui::SearchDialog *ui;

private slots:
    void cancelButtonPushed();

signals:
    void cancelSearch();
};

#endif // SEARCHDIALOG_H
