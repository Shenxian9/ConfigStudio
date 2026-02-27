#ifndef DATASRC_H
#define DATASRC_H

#include <QWidget>

namespace Ui {
class DataSrc;
}

class DataSrc : public QWidget
{
    Q_OBJECT

public:
    explicit DataSrc(QWidget *parent = nullptr);
    ~DataSrc();

private:
    Ui::DataSrc *ui;
};

#endif // DATASRC_H
