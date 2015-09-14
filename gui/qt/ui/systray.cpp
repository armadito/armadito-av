#include "systray.h"
#include "utils/stdpaths.h"
#include "model/scanmodel.h"
#include "scanwindow.h"
#include "aboutdialog.h"
#include "updatedialog.h"

#include <QtGui>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QGraphicsColorizeEffect>
#include <QFileDialog>
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
#if 0
  QSvgRenderer renderer(QString(":/icons/uhuru.svg"));

  QPixmap pixmap(24, 24);
  //  pixmap.fill(0xaaA08080);  // partly transparent red-ish background

  // Get QPainter that paints to the image
  QPainter painter(&pixmap);
  painter.setPen(Qt::white);
  renderer.render(&painter);

  //  QIcon icon(":/icons/uhuru.svg");
  // QGraphicsSvgItem *red = new QGraphicsSvgItem();
  // QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect();
  // red.setGraphicsEffect(effect);

  return new QIcon(pixmap);
#endif
#if 1
  return new QIcon(":/icons/uhuru.svg");
#endif
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

void Systray::addRecentScan(ScanModel *model)
{
  RecentScanAction *action = new RecentScanAction(model->startDate().toString(Qt::DefaultLocaleShortDate), this, model);

  recentScanMenu->addAction(action);

  connect(action, SIGNAL(triggered()), action, SLOT(showScan()));
}

void Systray::scan(const QString &path)
{
  std::cerr << "scanning " << path.toStdString().c_str() << "\n";

  ScanModel *model = new ScanModel(path);
  ScanWindow *w = new ScanWindow(model);
  w->show();
  w->raise();
  w->activateWindow();

#if 0
  addRecentScan(model);
#endif
}

void Systray::scan()
{
  QString fileName = "";

#if 0
  QFileDialog *fileDialog = new QFileDialog(0);

  fileDialog->setFileMode(QFileDialog::DirectoryOnly);
  fileDialog->setOption(QFileDialog::DontUseNativeDialog, false);
  fileDialog->setLabelText(QFileDialog::Accept, "&Choisir");
  fileDialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  fileDialog->setDirectory(StdPaths::desktopLocation());

  if (fileDialog->exec() == QDialog::Accepted) {
    QStringList list = fileDialog->selectedFiles();
    if(list.size() > 0)
      fileName = list[0];
  }
#endif
#if 0
  fileName = QFileDialog::getOpenFileName(0, tr("Scan File"), StdPaths::desktopLocation(), "", 0, QFileDialog::ShowDirsOnly);
#endif
#if 0
  fileName = QFileDialog::getExistingDirectory(0, tr("Scan Directory"), "", QFileDialog::ShowDirsOnly);
#endif
#if 1
  fileName = QFileDialog::getExistingDirectory(0, tr("Scan Directory"), "", 0);
#endif

  scan(fileName);
}

void Systray::update()
{
  UpdateDialog *update = new UpdateDialog();
  update->show();
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

