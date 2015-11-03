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
    void refreshUpdateInfo();

private slots:
    void fillView();

private:
    QString getStatusQString(enum UpdateStatus status);
    void addModuleItem(const ModuleInfo &moduleInfo, QListWidget *pListWidget);
    QIcon *getIcon();

    Ui::UpdateDialog *_ui;
    InfoModel *_model;
};

#endif // UPDATEDIALOG_H
