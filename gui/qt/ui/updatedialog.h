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

public slots:
    void RefreshUpdateInfo();

private:
    QString getStatusQString(int status);
#if 0
    void AddModuleItem(struct uhuru_module_info **m, QListWidget *pListWidget);
#endif
    void fillView(UpdateInfoModel *model);
    void SetupRefreshButton();
    QIcon *getIcon();
    Ui::UpdateDialog *ui;
    UpdateInfoModel *_model;
};

#endif // UPDATEDIALOG_H
