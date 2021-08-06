#ifndef INTRODUCTION_H
#define INTRODUCTION_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class introduction; }
QT_END_NAMESPACE

class introduction : public QWidget
{
    Q_OBJECT
public:
    explicit introduction(QWidget *parent = nullptr);

private:
    Ui::introduction *ui1;
signals:

};

#endif // INTRODUCTION_H
