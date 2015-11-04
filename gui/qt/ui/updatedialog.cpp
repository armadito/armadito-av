#include "updatedialog.h"
#include "ui_updatedialog.h"

#include <QTextStream>
#include <QListWidgetItem>
#include <QList>

UpdateDialog::UpdateDialog(InfoModel *model, QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::UpdateDialog)
{
  _ui->setupUi(this);
	
  _model = model;

  QObject::connect(_model, SIGNAL(updated()), this, SLOT(fillView()));

  _model->doUpdate();

  QPushButton *refreshButton =  findChild<QPushButton*>("RefreshButton");

  // Create connexion onClick() -> refreshUpdateInfo
  connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshUpdateInfo()));
}

QIcon *UpdateDialog::getIcon()
{
  return new QIcon(":/icons/uhuru_grey.svg");
}

void UpdateDialog::refreshUpdateInfo()
{
  _model->doUpdate();
}

void UpdateDialog::fillView()
{
  _model->debug();

  QListWidget *pListWidget = findChild<QListWidget*>("listWidget"); 

  for (int i = 0; i < _model->moduleInfos().size(); ++i)
    addModuleItem(_model->moduleInfos().at(i), pListWidget);
}

void UpdateDialog::addModuleItem(const ModuleInfo &moduleInfo, QListWidget *pListWidget)
{
  QIcon *icon_uhuru = getIcon();
  QIcon *icon_clamav = new QIcon(":/icons/clamav.ico");

  // Find item if already exists
  QListWidgetItem *item = NULL;
  QString pattern = QString("Module ") +  QString(moduleInfo.name());
  QList<QListWidgetItem *> items = pListWidget->findItems(pattern, Qt::MatchStartsWith);
  if(items.isEmpty())
    item = new QListWidgetItem();
  else
    item = items.first();

  // We retrieve the QString correspondig to the status
  QString status = getStatusQString(moduleInfo.status());

  // We format the line to be shown
  item->setText( QString("Module ") +  QString(moduleInfo.name()) + QString(" -- Last update : ") + QString(moduleInfo.updateDate()) + QString(" -- ") + status);

  // Set Icon
  if (moduleInfo.name() == "clamav")
    item->setIcon(*icon_clamav);
  else
    item->setIcon(*icon_uhuru);

  pListWidget->addItem(item);
}

QString UpdateDialog::getStatusQString(enum UpdateStatus status)
{
   switch(status){
   case UPDATE_OK:
     return QString("OK");
   case UPDATE_LATE:
     return QString("LATE");
   case UPDATE_CRITICAL:
     return QString("CRITICAL");
   case UPDATE_NON_AVAILABLE:
       return QString("NON_AVAILABLE");
   default:
     return QString("UNKNOWN");
   }
}

UpdateDialog::~UpdateDialog()
{
    delete _ui;
}
