#include "scanwindow.h"
#include "scanwidget.h"

#include <QApplication>
#include <QDesktopWidget>

QIcon *ScanWindow::getIcon()
{
#if 0
  QSvgRenderer renderer(QString(":/icons/uhuru_grey.svg"));

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
  return new QIcon(":/icons/uhuru_grey.svg");
#endif
}


void ScanWindow::construct(ScanModel *model)
{
  _model = model;

  ScanWidget *s = new ScanWidget(_model);

  setWindowTitle(QString("Analyse antivirale"));
  setCentralWidget(s);
  setWindowIcon(*getIcon());

  resize(800, 500);

  move(QApplication::desktop()->screen()->rect().center() - rect().center());

  _model->scan();
}

ScanWindow::ScanWindow(ScanModel *model, QWidget *parent) :
    QMainWindow(parent)
{
  construct(model);
}

ScanWindow::ScanWindow(const QString &path, QWidget *parent) :
    QMainWindow(parent)
{
  construct(new ScanModel(path));
}

