#include "widget.h"
#include "ui_widget.h"
#include <QPainter>
#include<QBrush>
#include<QEvent>
#include<QMouseEvent>
#include<iostream>

/*问题
    1、布局时一开始会自己填充两个
    2、悔棋没用
    3、甲乙布局完后再点甲（乙）方布局就可以继续填充
    4、其他*/

int x_0=0;
int y_0=0;
int l=40;
int num_control_max=1;
int num_control=0;//在某一步中下了几颗棋子
int flag_begin=1;//判断是否处于最开始的布局阶段,1为甲方（红），2为乙方（黑） 3为甲生，4为乙生，5为甲死，6为乙死，为中间状态 //，0为后续阶段，-1为开始游戏, 3为游戏进行时
bool moved[100][100];//判断是否已进行过移动。
bool flag_move=0; //判断是否进行棋子移动，0为不动，1为动。
int Chess_x_origin,Chess_y_origin;//移动时棋子原位置
int flag_regret=0;//判断是否悔棋
int draw_backgroud;//是否画了背景
int num_g=14;//一行或一列格子的数量
int purple;//初始落子的个数
int state[100][100];//状态数组   1为甲方，2为乙方    3为甲生，4为乙生，5为甲死，6为乙死
int game_win;//判断本局游戏是否胜利
//int order;//order为此时下子的角色，1为甲(红），2为乙（黑）
int Chess_x,Chess_y;//此时鼠标落子位置位于状态数组的何处
int Chess_x_old=-1,Chess_y_old=-1;//记录上一步的棋子的位置
int originX,originY;//鼠标原先位置
position mouse_pos;//鼠标位置

int dx[5]={0,0,0,-1,1},dy[5]={0,-1,1,0,0};  //格点周围的方向向量
static position s[10000];   //用栈来存本轮状态改变的点
static int top=0;   //栈顶

position find_pos(int i,int j)
{
    position pos;
    pos.chess_x=i;
    pos.chess_y=j;
    pos.point_x=x_0+(i-1)*l;
    pos.point_y=y_0+(j-1)*l;
    return pos;
}
position determine_pos(int x,int y)
{
    position pos;
    pos.chess_x=(x-x_0)/l+1;
    pos.chess_y=(y-y_0)/l+1;
    pos.point_x=x_0+(pos.chess_x-1)*l;
    pos.point_y=y_0+(pos.chess_y-1)*l;
    return pos;
}
bool judge(int x,int y)
{
    if(x&&y&&x<=num_g&&y<=num_g)   //判断落子是否在棋盘内
    {
        return 1;
    }
    return 0;
}
bool judge_new(int x,int y)      //判断落子是否合法
{
    if(x&&y&&x<=num_g&&y<=num_g)   //判断落子是否在棋盘内
    {
        if(flag_begin==1)          //判断棋子是否落入自己领地
        {
            if(y>num_g/2)  return 1;
        }
        else
        {
            if(y<=num_g/2)  return 1;
        }
    }
    return 0;
}
int check_game()       //判断本局游戏是否结束
{
    if(state[1][num_g/2]==1||state[1][num_g/2+1]==1||state[2][num_g/2]==1||state[2][num_g/2+1]==1) return 1;
    if(state[num_g][num_g/2]==2||state[num_g][num_g/2+1]==2||state[num_g-1][num_g/2]==2||state[num_g-1][num_g/2+1]==2) return 2;
    return 0;
}
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::paintEvent(QPaintEvent *event)
{
    //if(draw_backgroud==0)
    {                                          //绘制地图
     QPainter painter(this);
     painter.setRenderHint(QPainter::Antialiasing,true);

     QPen pen=painter.pen();
     pen.setColor(Qt::black);
     pen.setWidth(1);
     painter.setPen(pen);
     for(int i=1;i<=num_g+1;i++)
     {
         painter.drawLine(x_0+(i-1)*l,y_0,x_0+(i-1)*l,y_0+num_g*l);
         painter.drawLine(x_0,y_0+(i-1)*l,x_0+num_g*l,y_0+(i-1)*l);
     }
     //draw_backgroud=1;
     pen.setWidth(2);
     painter.setPen(pen);
     painter.drawLine(x_0,y_0+(num_g/2)*l,x_0+num_g*l,y_0+(num_g/2)*l);     //画分界线

     painter.drawLine(x_0+(num_g/2-1)*l,y_0,x_0+(num_g/2+1)*l,y_0+2*l);     //画大本营
     painter.drawLine(x_0+(num_g/2-1)*l,y_0+2*l,x_0+(num_g/2+1)*l,y_0);
     pen.setColor(Qt::red);
     painter.setPen(pen);
     painter.drawLine(x_0+(num_g/2-1)*l,y_0+(num_g-2)*l,x_0+(num_g/2+1)*l,y_0+num_g*l);
     painter.drawLine(x_0+(num_g/2-1)*l,y_0+num_g*l,x_0+(num_g/2+1)*l,y_0+(num_g-2)*l);

     //画所选中的棋子  为什么不行？
     position temp=find_pos(Chess_x,Chess_y);
     int temp_x=temp.point_x,temp_y=temp.point_y;
     pen.setColor(Qt::green);
     painter.setPen(pen);
     painter.drawLine(temp_x+l/2,temp_y+l/4,temp_x+l/2,temp_y+l*0.75);
     painter.drawLine(temp_x+l/4,temp_y+l/2,temp_x+l*0.75,temp_y+l/2);
    }

    game_win=check_game();       //判断游戏是否结束

    QPainter painter(this);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    position p1;
    if(!flag_regret)
    {
        /*if(flag_begin==0)
        {
            if(num_control<num_control_max&&judge(Chess_x,Chess_y))
            {
                if(!state[Chess_x][Chess_y])   //判断所下棋子所属阵营
                {
                    num_control++;
                    if(num_control%2)
                    {
                        state[Chess_x][Chess_y]=num_control%2;
                    }
                    else
                    {
                        state[Chess_x][Chess_y]=2;
                    }
                    Chess_x_old=Chess_x;
                    Chess_y_old=Chess_y;
                 }
                 for(int i=1;i<=num_g;i++)   //绘制棋子
                 {
                     for(int j=1;j<=num_g;j++)
                     {
                         if(state[i][j]==1)
                         {
                             p1=find_pos(i,j);
                             brush.setColor(Qt::red);
                             painter.setBrush(brush);
                             painter.drawRect(p1.point_x,p1.point_y,l,l);
                         }
                         else if(state[i][j]==2)
                         {
                             p1=find_pos(i,j);
                             brush.setColor(Qt::black);
                             painter.setBrush(brush);
                             painter.drawRect(p1.point_x,p1.point_y,l,l);
                         }
                     }
                 }
            }
        }
        else if(flag_begin==1||flag_begin==2)
        {
            if(num_control<num_control_max/2&&judge(Chess_x,Chess_y))
            {
                if(!state[Chess_x][Chess_y])
                {
                    num_control++;
                    state[Chess_x][Chess_y]=flag_begin;
                    Chess_x_old=Chess_x;
                    Chess_y_old=Chess_y;
                }
                for(int i=1;i<=num_g;i++)
                {
                    for(int j=1;j<=num_g;j++)
                    {
                        if(state[i][j]==1)
                        {
                            p1=find_pos(i,j);
                            brush.setColor(Qt::red);
                            painter.setBrush(brush);
                            painter.drawRect(p1.point_x,p1.point_y,l,l);
                        }
                        else if(state[i][j]==2)
                        {
                            p1=find_pos(i,j);
                            brush.setColor(Qt::black);
                            painter.setBrush(brush);
                            painter.drawRect(p1.point_x,p1.point_y,l,l);
                        }
                    }
                }
            }
        }*/
        //std::cout<<num_control<<std::endl;
        if(judge_new(Chess_x,Chess_y)&&!flag_move&&num_control<num_control_max&&!state[Chess_x][Chess_y])  //新生成棋子
        {
            if(!num_control)   //防止上一步下的棋子被程序误判成这一步下的
            {
                state[Chess_x][Chess_y]=flag_begin;
                Chess_x_old=Chess_x;
                Chess_y_old=Chess_y;
                num_control++;
            }
        }
        else if(flag_move&&(abs(Chess_x-Chess_x_origin)+abs(Chess_y-Chess_y_origin))==1) //移动棋子,并判断移动位置是否合法
        {
            //std::cout<<"state["<<Chess_x_origin<<"]["<<Chess_y_origin<<"]="<<state[Chess_x_origin][Chess_y_origin];
           //std::cout<<"   state["<<Chess_x<<"]["<<Chess_y<<"]="<<state[Chess_x][Chess_y]<<std::endl;
            state[Chess_x][Chess_y]=flag_begin;
            moved[Chess_x][Chess_y]=1;
            state[Chess_x_origin][Chess_y_origin]=0;
            Chess_x_origin=-1; Chess_y_origin=-1;
            flag_move=0;
        }
    }
    else   //悔棋,只能悔一步
    {
        //num_control--;
        if(flag_begin==1) flag_begin=2;
        else flag_begin=1;
        state[Chess_x][Chess_y]=0;
    }
    for(int i=1;i<=num_g;i++)   //绘制棋子
    {
        for(int j=1;j<=num_g;j++)
        {
            if(state[i][j]==1)
            {
                p1=find_pos(i,j);
                brush.setColor(Qt::red);
                painter.setBrush(brush);
                painter.drawRect(p1.point_x,p1.point_y,l,l);
            }
            else if(state[i][j]==2)
            {
                p1=find_pos(i,j);
                brush.setColor(Qt::black);
                painter.setBrush(brush);
                painter.drawRect(p1.point_x,p1.point_y,l,l);
            }
        }
    }
}

void Widget::paintmouse()//QPaintEvent *event
{
    //绘制鼠标周围的四个红杠杠 ^_^
    {
        int d=5;  //红杠杠 线的间距；
        QPainter painter(this);
        QPen pen1=painter.pen();
        pen1.setColor(Qt::red);
        pen1.setWidth(2);
        painter.setPen(pen1);
        painter.drawLine(mouse_pos.point_x,mouse_pos.point_y,mouse_pos.point_x+d,mouse_pos.point_y);
        painter.drawLine(mouse_pos.point_x,mouse_pos.point_y,mouse_pos.point_x,mouse_pos.point_y+d);
        painter.drawLine(mouse_pos.point_x+l,mouse_pos.point_y,mouse_pos.point_x+l-d,mouse_pos.point_y);
        painter.drawLine(mouse_pos.point_x+l,mouse_pos.point_y,mouse_pos.point_x+l,mouse_pos.point_y+d);
        painter.drawLine(mouse_pos.point_x,mouse_pos.point_y+l,mouse_pos.point_x+d,mouse_pos.point_y+l);
        painter.drawLine(mouse_pos.point_x,mouse_pos.point_y+l,mouse_pos.point_x,mouse_pos.point_y+l-d);
        painter.drawLine(mouse_pos.point_x+l,mouse_pos.point_y+l,mouse_pos.point_x+l-d,mouse_pos.point_y+l);
        painter.drawLine(mouse_pos.point_x+l,mouse_pos.point_y+l,mouse_pos.point_x+l,mouse_pos.point_y+l-d);
    }
}

/*void Widget::mouseMoveEvent(QMouseEvent *event)
{
    position origin_pos=determine_pos(originX,originY);
    originX=event->globalX();
    originY=event->globalY();
    position pos=determine_pos(originX,originY);
    if(judge(pos.chess_x,pos.chess_y))
    {
        if(origin_pos.chess_x!=pos.chess_x||origin_pos.chess_y!=pos.chess_y)
        {
            mouse_pos=pos;
            paintmouse();
        }
    }
}*/

void Widget::mousePressEvent(QMouseEvent *e)
{
    //if(flag_begin!=3)
    int x=e->x();
    int y=e->y();
    {
        if(x>=x_0&&x<=x_0+num_g*l&&y>=y_0&&y<=y_0+num_g*l)   //转换成棋子位置
        {
            Chess_x=(x-x_0)/l+1;
            Chess_y=(y-y_0)/l+1;
            if(judge(Chess_x,Chess_y))
            {
                update();


                if(!flag_move&&!state[Chess_x][Chess_y]) update();  //没有子移动，而且下的位置没有棋子————生成新棋子
                else if(!flag_move&&state[Chess_x][Chess_y]==flag_begin&&!moved[Chess_x][Chess_y]) {Chess_x_origin=Chess_x; Chess_y_origin=Chess_y; flag_move=1;}
                       //没有子移动，但是下的位置有自家棋子，并且该棋子没有移动————下一步移动该棋子
                else if(flag_move&&!state[Chess_x][Chess_y]) update();  //有子移动，且下的位置没有棋子————移动到这里
                else if(flag_move&&state[Chess_x][Chess_y]==flag_begin&&!moved[Chess_x][Chess_y])   {Chess_x_origin=Chess_x; Chess_y_origin=Chess_y; flag_move=1;}
                       //有子移动，但是下的位置还有自家的子，并且该子未移动————移动对象转换到下的位置的子身上
                else if(flag_move&&((state[Chess_x][Chess_y]==flag_begin&&moved[Chess_x][Chess_y])||state[Chess_x][Chess_y]==3-flag_begin))
                       {Chess_x_origin=0;  Chess_y_origin=0; Chess_x=0;  Chess_y=0;  flag_move=0;}
                       //有子移动，但是下的位置有子：敌方子/自家移动完的子————移动取消
            }

        }
    }
   /* else                //鼠标点击，进行棋子移动
    {
        int x=e->x(),y=e->y();
        if(x>=x_0&&x<=x_0+num_g*l&&y>=y_0&&y<=y_0+num_g*l)
        {
            Chess_x=(x-x_0)/l+1;
            Chess_y=(y-y_0)/l+1;
            update();
        }
        if(state[Chess_x][Chess_y]==1||state[Chess_x][Chess_y]==2)
        {
            Chess_x_old=Chess_x;
            Chess_y_old=Chess_y;

        }
    }*/
}

void Widget::on_b1_pressed()  //新游戏 重绘背景
{
    for(int i=1;i<=num_g;i++)
    {
        for(int j=1;j<=num_g;j++)
        {
            state[i][j]=0;
        }
    }
    num_control=0;
    Chess_x=Chess_y=0;
    Chess_x_old=Chess_y_old=-1;
    flag_begin=1;
    update();
}

void Widget::on_b3_clicked()  //游戏说明
{
   w1.show();
}

void Widget::on_b4_clicked()
{

}

void Widget::on_b5_clicked()  //悔棋
{
    flag_regret=1;
    update();
    flag_regret=0;
}

void Widget::change(int x,int y)
{
    if(flag_begin==1||flag_begin==2)    //红黑双方操作完后各进行一轮判断
    {
        int num[2]={0,0};//统计p1，p2的个数
        int newx,newy;
        for(int i=1;i<=4;i++)
        {
            newx=x+dx[i],newy=y+dy[i];
            if(judge(newx,newy))
            {
                if(state[newx][newy]==1||state[newx][newy]==2||state[newx][newy]==5||state[newx][newy]==6)
               {
                    num[state[newx][newy]%2]++;
               }
            }
        }
        if(!state[x][y])
        {
            if(num[0]==4) {state[x][y]=4; s[++top]=position(x,y);}
            if(num[1]==4) {state[x][y]=3; s[++top]=position(x,y);}
        }
        else if(state[x][y]==1||state[x][y]==2)
        {
            if(num[0]==4||num[1]==4) {state[x][y]+=4; s[++top]=position(newx,newy);}

            int state_temp=3-state[x][y];
            if(judge(x+2,y)&&(state[x+1][y]==state_temp||state[x+1][y]==(state_temp+4))&&(state[x+2][y]==state_temp||state[x+2][y]==(state_temp+4)))
               { state[x][y]+=4; s[++top]=position(x,y); }
            if(judge(x-2,y)&&(state[x-1][y]==state_temp||state[x-1][y]==(state_temp+4))&&(state[x-2][y]==state_temp||state[x-2][y]==(state_temp+4)))
               { state[x][y]+=4; s[++top]=position(x,y); }
            if(judge(x,y+2)&&(state[x][y+1]==state_temp||state[x][y+1]==(state_temp+4))&&(state[x][y+2]==state_temp||state[x][y+2]==(state_temp+4)))
               { state[x][y]+=4; s[++top]=position(x,y); }
            if(judge(x,y-2)&&(state[x][y-1]==state_temp||state[x][y-1]==(state_temp+4))&&(state[x][y-2]==state_temp||state[x][y-2]==(state_temp+4)))
               { state[x][y]+=4; s[++top]=position(x,y); }

        }
    }
}

void Widget::change_map()
{
    while(top)
    {
        int x=s[top].chess_x,y=s[top].chess_y;
        top--;
        if(state[x][y]==3) state[x][y]=1;
        if(state[x][y]==4) state[x][y]=2;
        if(state[x][y]==5||state[x][y]==6) {state[x][y]=0;}//std::cout<<"state["<<x<<"]["<<y<<"]="<<state[x][y]<<std::endl;
    }
}

void Widget::on_b2_clicked()   //下一步
{
    for(int i=1;i<=num_g;i++)
        for(int j=1;j<=num_g;j++)
        {
            change(i,j);
        }
    change_map();
    if(flag_begin==1) flag_begin=2;
    else flag_begin=1;
    num_control=0;
    flag_move=0;
    memset(moved,0,sizeof(moved));
    Chess_x=Chess_y=0;
    update();
}
