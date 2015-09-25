#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QListWidget>
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
    QString getStatusQString(int status);
    void AddModuleItem(struct uhuru_module_info **m, QListWidget *pListWidget);
    void fillView(UpdateInfoModel *model);
    QIcon *getIcon();
    Ui::UpdateDialog *ui;
};

#endif // UPDATEDIALOG_H
