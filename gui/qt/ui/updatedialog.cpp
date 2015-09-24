#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(UpdateInfoModel *model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
    doConnect(model);
}

void UpdateDialog::doConnect(UpdateInfoModel *model)
{
    fprintf(stdout, "UpdateDialog doConnect");
    struct uhuru_info * info = NULL;
    info = model->getUpdateInfo();

    info_to_stdout(info);

    // On libère tout
    uhuru_info_free(info); 
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}
