#include "systray.h"
#include "utils/stdpaths.h"
#include "utils/daemon.h"
#include "model/scanmodel.h"
#include "model/infomodel.h"
#include "scanwindow.h"
#include "aboutdialog.h"
#include "updatedialog.h"

#include <QObject>
#include <QtGui>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QGraphicsColorizeEffect>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <stdlib.h>

Systray::Systray()
{
  createActions();
  createTrayIcon();

  trayIcon->show();
}

void Systray::createActions()
{
  scanAction = new QAction(tr("&Scan"), this);
  connect(scanAction, SIGNAL(triggered()), this, SLOT(scan()));

  updateAction = new QAction(tr("&Update"), this);
  connect(updateAction, SIGNAL(triggered()), this, SLOT(update()));

  aboutAction = new QAction(tr("&About"), this);
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
}

QIcon *Systray::getIcon()
{
  return new QIcon(":/icons/uhuru_white.svg");
}

void Systray::createTrayIcon()
{
  QMenu *trayIconMenu = new QMenu();

  trayIconMenu->addAction(scanAction);
#if 0
  trayIconMenu->addSeparator();
  recentScanMenu = trayIconMenu->addMenu(tr("&Recent analysis"));
#endif
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(updateAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(aboutAction);

  trayIcon = new QSystemTrayIcon(this);

  QByteArray category = qgetenv("SNI_CATEGORY");
  if (!category.isEmpty()) {
    trayIcon->setProperty("_qt_sni_category", QString::fromLocal8Bit(category));
  }

  trayIcon->setContextMenu(trayIconMenu);
  trayIcon->setIcon(*getIcon());
}

static void popUpCannotConnectToServer()
{
  QMessageBox msgBox;

  msgBox.setText(QObject::tr("Cannot connect to Uhuru scan daemon"));
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.exec();
}

void Systray::addRecentScan(ScanModel *model)
{
  RecentScanAction *action = new RecentScanAction(model->startDate().toString(Qt::DefaultLocaleShortDate), this, model);

  recentScanMenu->addAction(action);

  connect(action, SIGNAL(triggered()), action, SLOT(showScan()));
}

void Systray::scan()
{
  DaemonConnection conn;
  int ioFd;

  ioFd = conn.connect();

  if (ioFd < 0) {
    popUpCannotConnectToServer();
    return;
  }

  QString fileName = "";

  fileName = QFileDialog::getExistingDirectory(0, tr("Scan Directory"), "", 0);

  if(fileName.isNull())
    return;

  ScanModel *model = new ScanModel(ioFd, fileName);
  ScanWindow *w = new ScanWindow(model);
  w->show();
  w->raise();
  w->activateWindow();

#if 0
  addRecentScan(model);
#endif
}

void Systray::update()
{
  DaemonConnection conn;
  int ioFd;

  ioFd = conn.connect();

  if (ioFd < 0) {
    popUpCannotConnectToServer();
    return;
  }

  InfoModel *model = new InfoModel(ioFd);
  UpdateDialog *d = new UpdateDialog(model);

  d->show();
  d->raise();
  d->activateWindow();

  // Init model => this model will be re-used each time user click on refresh button
  //UpdateInfoModel *model = new UpdateInfoModel();

  // Init UpdateDialog ui, set first values and init connexions
  //UpdateDialog *update = new UpdateDialog(model);
  //update->show();
}


RecentScanAction::RecentScanAction(QObject *parent, ScanModel *model)
  : QAction(parent), _model(model)
{
}

RecentScanAction::RecentScanAction(const QString &text, QObject *parent, ScanModel *model)
  : QAction(text, parent), _model(model)
{
}

RecentScanAction::RecentScanAction(const QIcon &icon, const QString &text, QObject *parent, ScanModel *model)
  : QAction(icon, text, parent), _model(model)
{
}

void RecentScanAction::showScan()
{
  ScanWindow *w = new ScanWindow(_model);
  w->show();
}

void Systray::about()
{
  AboutDialog *about = new AboutDialog();
  about->show();
}

