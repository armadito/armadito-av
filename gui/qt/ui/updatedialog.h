#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QListWidget>
#include "model/infomodel.h"

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(InfoModel *model = NULL, QWidget *parent = 0);
    ~UpdateDialog();

public slots:
    void RefreshUpdateInfo();

private slots:
    void fillView();

private:
    QString getStatusQString(int status);
#if 0
    void AddModuleItem(struct uhuru_module_info **m, QListWidget *pListWidget);
#endif
    void SetupRefreshButton();
    QIcon *getIcon();
    Ui::UpdateDialog *ui;
    InfoModel *_model;
};

#endif // UPDATEDIALOG_H
