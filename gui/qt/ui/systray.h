#ifndef SYSTRAY_H
#define SYSTRAY_H

#include "model/scanmodel.h"

#include <QObject>
#include <QThread>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

class Systray : public QObject {
    Q_OBJECT

public:
  Systray();
  ~Systray() {}

  void addRecentScan(ScanModel *model);

  void notify(const QString & title, const QString & message, QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);

private slots:
  void scan();
  void update();
  void about();

private:
  void createActions();
  void createTrayIcon();

  QIcon *getIcon();

  QAction *scanAction;
  QAction *updateAction;
  QAction *aboutAction;
  QMenu *recentScanMenu;

  QSystemTrayIcon *_trayIcon;
};

class RecentScanAction: public QAction {
    Q_OBJECT

 public:
  RecentScanAction(QObject *parent, ScanModel *model);
  RecentScanAction(const QString &text, QObject *parent, ScanModel *model);
  RecentScanAction(const QIcon &icon, const QString &text, QObject *parent, ScanModel *model);

 public slots:
  void showScan();

 private:
  ScanModel *_model;
};

#endif
