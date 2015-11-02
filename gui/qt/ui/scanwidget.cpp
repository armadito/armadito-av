#include "scanwidget.h"
#include "ui_scanwidget.h"

#include <QLineEdit>
#include <QProgressBar>
#include <assert.h>
#include <iostream>

void ScanWidget::connectLineEdit(const char *lineEditName, Counter *counter)
{
  QLabel *ui_lineEdit = findChild<QLabel*>(lineEditName);
  assert(ui_lineEdit != NULL);

  QObject::connect(counter, SIGNAL(changed(const QString &)), ui_lineEdit, SLOT(setText(const QString &)));

  // We set the value now
  ui_lineEdit->setText(QString::number(counter->value()));
}

void ScanWidget::doConnect(ScanModel *model)
{
  // We create counters connexions
  connectLineEdit("scannedCount", model->scannedCount());
  connectLineEdit("malwareCount", model->malwareCount());
  connectLineEdit("suspiciousCount", model->suspiciousCount());
  connectLineEdit("unhandledCount", model->unhandledCount());
  connectLineEdit("cleanCount", model->cleanCount());

  // ProgressBar Maximum
  QProgressBar *ui_progressBar = findChild<QProgressBar*>("progressBar");
  assert(ui_progressBar != NULL);
  ui_progressBar->setMaximum(100);

  // ProgressBar Value
  ui_progressBar->setValue(model->scannedCount()->value());
  QObject::connect(model->scannedCount(), SIGNAL(changed(int)), ui_progressBar, SLOT(setValue(int)));

  // Current scanned file path 
  QLineEdit *ui_currentScannedPath = findChild<QLineEdit*>("currentScannedPath");
  assert(ui_currentScannedPath != NULL);
  ui_currentScannedPath->setText(model->path());
  QObject::connect(model, SIGNAL(scanning(QString)), ui_currentScannedPath, SLOT(setText(QString)));

  // Close button enabled at end of scan
  ui_closeButton = findChild<QPushButton*>("closeButton");
  assert(ui_closeButton != NULL);
  QObject::connect(model, SIGNAL(scanComplete()), this, SLOT(enableCloseButton()));

  // Signal to update Title 
  ui_labelTitle = findChild<QLabel*>("Title");
  assert(ui_labelTitle != NULL);

  if (model->completed())
    enableCloseButton();

}

ScanWidget::ScanWidget(ScanModel *model, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ScanWidget)
{
  ui->setupUi(this);

  // Init values and connexions
  doConnect(model);

  // Set report model 
  ui_reportView = findChild<QTableView*>("reportView");
  assert(ui_reportView != NULL);

  // We must use a proxyModel in order to sort
  QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel();
  ScanReportModel *report_model = model->report();
  QObject::connect(report_model, SIGNAL(endInsert()), this, SLOT(afterEndInsert()));

  proxyModel->setSourceModel(report_model);
  ui_reportView->setModel(proxyModel); 

#if QT_VERSION < 0x050000
  ui_reportView->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#else
  ui_reportView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#endif

}

void ScanWidget::afterEndInsert()
{  
  // do something on ui_reportView
  	
}

void ScanWidget::enableCloseButton() 
{
  ui_labelTitle->setText(tr("Scan has finished"));
  ui_closeButton->setEnabled(true);
}

void ScanWidget::on_closeButton_clicked() 
{
  window()->close();
}

ScanWidget::~ScanWidget()
{
    delete ui;
}
