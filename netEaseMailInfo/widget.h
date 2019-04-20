#ifndef WIDGET_H
#define WIDGET_H

#include <windows.h>
#include <QWidget>
#include<QSettings>
#include<QDir>
#include<QDebug>
#include<QProcess>
#include<QMap>
#include<QMessageBox>
#include<QProcess>
#include<QTimer>
#include<QTextCodec>
#include<QStandardItemModel>
#include<QProcessEnvironment>
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);

    QStringList findAllUserWorkDirs(void);
    ~Widget();
    //解析文件的分隔符
    QString smallSplit = "******";
    QString largeSplit = "******************";

    QStandardItemModel *mainModel; //tableview列表模式
    QTimer *onTimeCheckNewMail; //实时检测新邮件到达的定时器
    QTimer *onTimeCheckFinish; //检测提取邮件结束的定时器
    QStringList globalMailInfoList;//全局保存邮件的列表
    QStringList globalContactsInfoList;//全局保存联系人的列表
    QStringList parseMailInfo();//解析邮件内容
    QStringList parseContactsInfo();//解析联系人内容

private slots:
    void slotRowDoubleClicked(const QModelIndex index);//双击单元格某行触发的槽

    int  on_getInstallPathBtn_clicked();

    int on_getDataPathBtn_clicked();

    void on_getAllUsersBtn_clicked();

    void on_mainBtn_clicked();

    void onCombBoxChanged(int);//下拉框改变的函数 影响当前选中的用户

    void  on_time_out();//实时检测新邮件的定时器
    void   check_finish_time_out();//运行python结束的定时器
    void on_showMailInfoBtn_clicked();

    void on_showContactsInfoBtn_clicked();

private:
    Ui::Widget *ui;

    QString RootWorkDir;//邮件保存的根路径

    QMap<QString,QString> userNameAndPathM_map; //定义一个QMap对象 保存用户名以及对应的物理路径

    QStringList allUserList;//保存所有的用户名列表 "q463511278@163.com"

    QString CurrentWorkUser;//保存当前要获取的用户名

};

#endif // WIDGET_H
