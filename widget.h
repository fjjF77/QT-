#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QEvent>
#include"introduction.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

struct position{
    int point_x,point_y;
    int chess_x,chess_y;
    position() { chess_x=0; chess_y=0; point_x=0; point_y=0; }
    position(int x1,int y1)
    {
        chess_x=x1;
        chess_y=y1;
        point_x=0;
        point_y=0;
    }
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    introduction w1;

protected:
    void paintEvent(QPaintEvent *event);
    //void mouseMoveEvent(QMouseEvent *event);
    void paintmouse();//QPaintEvent *event
    void mousePressEvent(QMouseEvent *e);
    void change(int x,int y);
    void change_map();
private slots:
    void on_b1_pressed();
    void on_b3_clicked();
    void on_b4_clicked();
    void on_b5_clicked();
    void on_b2_clicked();
};
#endif // WIDGET_H
