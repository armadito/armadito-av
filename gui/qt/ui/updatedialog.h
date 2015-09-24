#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include "model/updateinfomodel.h"

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(UpdateInfoModel *model = NULL, QWidget *parent = 0);
    ~UpdateDialog();

private:
    void doConnect(UpdateInfoModel *model);
    Ui::UpdateDialog *ui;
};

#endif // UPDATEDIALOG_H
