#include "datasrc.h"
#include "ui_datasrc.h"

DataSrc::DataSrc(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DataSrc)
{
    ui->setupUi(this);
}

DataSrc::~DataSrc()
{
    delete ui;
}
