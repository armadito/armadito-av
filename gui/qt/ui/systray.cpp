#include "systray.h"
#include "utils/stdpaths.h"
#include "model/scanmodel.h"
#include "scanwindow.h"
#include "aboutdialog.h"

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

#if 0
  QString user = QString(getenv("USER"));
  QString r = "/media/";
  QString watchDir = r + user;
  _watchThread = new WatchThread(watchDir);

  QObject::connect(_watchThread, SIGNAL(watched(const QString &)), this, SLOT(scan(const QString &)));
  _watchThread->start();
#endif
}

void Systray::createActions()
{
  scanAction = new QAction(tr("&Scan"), this);
  connect(scanAction, SIGNAL(triggered()), this, SLOT(scan()));

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
  QFileDialog *fileDialog = new QFileDialog(0);

  fileDialog->setFileMode(QFileDialog::DirectoryOnly);
  fileDialog->setOption(QFileDialog::DontUseNativeDialog, false);
  fileDialog->setLabelText(QFileDialog::Accept, "&Choisir");
  fileDialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  fileDialog->setDirectory(StdPaths::desktopLocation());

  //  QString fileName = QFileDialog::getOpenFileName(0, tr("Scan File"));

  if (fileDialog->exec() == QDialog::Accepted) {
    QStringList list = fileDialog->selectedFiles();
    if(list.size() > 0) {
      scan(list[0]);
    }
  }
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

#if 0
void WatchThread::run()
{
  QByteArray ba = _path.toLocal8Bit();
  const char *c_path = ba.data();
  struct uhuru_watch_event watch_event;

  uhuru_watch(UHURU::instance(), c_path);

  while (!uhuru_watch_next_event(UHURU::instance(), &watch_event)) {
    switch(watch_event.event_type) {
    case UHURU_WATCH_DIRECTORY_CREATE:
      std::cerr << "watch: must scan " << watch_event.full_path << "\n";

      emit watched(QString(watch_event.full_path));
      break;
    default:
      break;
    }
  }
}
#endif

