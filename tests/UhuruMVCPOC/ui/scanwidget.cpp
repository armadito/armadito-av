#include "scanwidget.h"
#include "ui_scanwidget.h"

#include <QLineEdit>
#include <QProgressBar>
#include <assert.h>

ScanWidget::ScanWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScanWidget)
{
    ui->setupUi(this);

    QLineEdit *ui_pathToScan = findChild<QLineEdit*>("pathToScan");
    ui_pathToScan->setText(QCoreApplication::arguments().at(1));

    model = new ScanModel(ui_pathToScan->text());

    QLineEdit *ui_fileCount = findChild<QLineEdit*>("fileCount");
    assert(ui_fileCount != NULL);

    QObject::connect(model->fileCount(), SIGNAL(changed(const QString &)), ui_fileCount, SLOT(setText(const QString &)));

    QProgressBar *ui_progressBar = findChild<QProgressBar*>("progressBar");
    assert(ui_progressBar != NULL);

    QObject::connect(model->fileCount(), SIGNAL(changed(int)), ui_progressBar, SLOT(setMaximum(int)));

    QLineEdit *ui_scannedCount = findChild<QLineEdit*>("scannedCount");
    assert(ui_scannedCount != NULL);
    QObject::connect(model->scannedCount(), SIGNAL(changed(const QString &)), ui_scannedCount, SLOT(setText(const QString &)));
    QObject::connect(model->scannedCount(), SIGNAL(changed(int)), ui_progressBar, SLOT(setValue(int)));

    QLineEdit *ui_malwareCount = findChild<QLineEdit*>("malwareCount");
    assert(ui_malwareCount != NULL);
    QObject::connect(model->malwareCount(), SIGNAL(changed(const QString &)), ui_malwareCount, SLOT(setText(const QString &)));

    QLineEdit *ui_suspiciousCount = findChild<QLineEdit*>("suspiciousCount");
    assert(ui_suspiciousCount != NULL);
    QObject::connect(model->suspiciousCount(), SIGNAL(changed(const QString &)), ui_suspiciousCount, SLOT(setText(const QString &)));
}

ScanWidget::~ScanWidget()
{
    delete ui;
}
