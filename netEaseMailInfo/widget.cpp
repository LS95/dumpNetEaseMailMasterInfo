#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //设置按钮不可见 自动执行查找路径等功能
    ui->getInstallPathBtn->setVisible(false);
    ui->getDataPathBtn->setVisible(false);
    ui->getAllUsersBtn->setVisible(false);

    Widget::on_getInstallPathBtn_clicked();

    Widget::on_getDataPathBtn_clicked();

    Widget::on_getAllUsersBtn_clicked();


    //关联用户复选框的信号和槽
   connect(ui->usersComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCombBoxChanged(int)));  //连接当前选中的用户的槽函数
    //关联显示详细信息 的信号和槽
    connect(ui->tableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

   //开启实时检测的定时器
   onTimeCheckNewMail = new QTimer(this);
   onTimeCheckNewMail->start(5000);
   connect(onTimeCheckNewMail, SIGNAL(timeout()), this, SLOT(on_time_out()));

   //设置treeview表头
   mainModel= new QStandardItemModel();
   mainModel->setColumnCount(5);
   mainModel->setHeaderData(0,Qt::Horizontal,QString("邮件序号"));
   mainModel->setHeaderData(1,Qt::Horizontal,QString("主题"));
   mainModel->setHeaderData(2,Qt::Horizontal,QString("发送者"));
   mainModel->setHeaderData(3,Qt::Horizontal,QString("接收者"));
   mainModel->setHeaderData(4,Qt::Horizontal,QString("附件列表"));
   ui->tableView->setModel(mainModel);

   //设置选中时为整行选中
   ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

   //设置表格的单元为只读属性，即不能编辑
   ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
   //如果你用在QTableView中使用右键菜单，需启用该属性
   ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
   //设置不可多行选中
   ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
}

Widget::~Widget()
{
    ui->dataPathlineEdit->clear();
    ui->installPathlineEdit->clear();
    ui->textBrowser->clear();
    //删除解析的文件
    QProcess p(0);
    p.start("cmd", QStringList()<<"/c"<<"del *.txt /F /Q");
    p.waitForStarted();
    p.waitForFinished(-1);

    delete ui;
}

//获得所有的用户文件夹
 QStringList Widget::findAllUserWorkDirs(void)
{

    //Widget::allUserList.clear();//清空所有的用户名列表
    QDir *dir = new QDir(Widget::RootWorkDir);//如果不加地址，则就在当前目录下遍历
    QFileInfoList list = dir->entryInfoList();
    if(list.length()!=0)
    {
        for (int i = 0; i < list.size(); ++i)
           {
               QString tmpFileName  =  list.at(i).fileName();
               //找到所有的用户名
               if(tmpFileName.contains("@"))
               {
                   qDebug() << tmpFileName;
                   QString UserName = tmpFileName.split("_").at(0);
                   QString UserFilePath =  Widget::RootWorkDir  + tmpFileName + "\\";
                   qDebug() << UserName << ">>>>>>>>>>>>>>>>>"   <<  UserFilePath;
                   //不包含重复的名字 则添加
                   if(!Widget::allUserList.contains(UserName))
                   {
                       userNameAndPathM_map.insert(UserName,UserFilePath);
                       Widget::allUserList <<UserName;
                   }
                 }
           }

    }else{
        qDebug()<<"no file";
    }
    //测试显示代码
    qDebug() << "Widget::allUserList==" << Widget::allUserList;
    ui->textBrowser->setText("所有用户列表:\n"+allUserList.join("\n"));
    return Widget::allUserList;
}


 //获得mailmaster.exe安装路径
int  Widget::on_getInstallPathBtn_clicked()
{
    // 遍历MuiCache下所有的键值 这个位置是管理员身份运行的exe集合 不一定有路径 获得邮箱大师的安装路径还要再找注册表
    QSettings *settings=new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\mailmaster.exe",QSettings::NativeFormat);
    QStringList keyList=settings->childKeys();
    foreach(QString key,keyList)
     {
            QString location=settings->value(key).toString();
             if(location.contains("mailmaster.exe"))
            {
                 ui->installPathlineEdit->setText(location);
                 return 1;
            }
       }
    ui->installPathlineEdit->setText("not found");
    return 0;
}



//获得数据文件夹的路径
int Widget::on_getDataPathBtn_clicked()
{
        QString LOCALAPPDATA = QProcessEnvironment::systemEnvironment().value("LOCALAPPDATA");
        QString workDir = LOCALAPPDATA + "\\Netease\\MailMaster\\data\\";
        Widget::RootWorkDir = workDir;
        qDebug() << LOCALAPPDATA;
        qDebug() << Widget::RootWorkDir;
        ui->dataPathlineEdit->setText(Widget::RootWorkDir);
        //遍历环境变量
        //        QStringList environmentList = QProcess::systemEnvironment();
        //        foreach (QString environment, environmentList )
        //        {
        //            qDebug() << environment;
        //        }
        return 0;
}



//获得所有的用户名 并显示在CombBox中
void Widget::on_getAllUsersBtn_clicked()
{

        Widget::findAllUserWorkDirs();
        qDebug("size of this map is : %d", userNameAndPathM_map.count()); //获取map包含的总数
        QMap<QString,QString>::iterator it; //遍历map
        for ( it = userNameAndPathM_map.begin(); it != userNameAndPathM_map.end(); ++it ) {
                qDebug() <<"key:"<<  it.key()  <<"value:" <<  it.value(); //用key()和data()分别获取“键”和“值”
        }
    //userNameAndPathM_map.clear(); //清空map

    disconnect(ui->usersComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCombBoxChanged(int)));  //取消连接当前选中的用户的槽函数
    ui->usersComboBox->clear();
    connect(ui->usersComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCombBoxChanged(int)));  //连接当前选中的用户的槽函数
    ui->usersComboBox->addItems(Widget::allUserList);

    //切换当前用户为选中的用户
    qDebug() << ui->usersComboBox->currentIndex();
    Widget::CurrentWorkUser = allUserList[ui->usersComboBox->currentIndex()];
     /*
    向comboBox部件里添加元素
    QStringList string;
        string<<"高"<<"低";
        ui->comboBox->addItems(string);
    个人理解这也是Qt的MVC模型的体现，QStringList是装载数据的model,当然comboBox就是view

    怎么获取comboBox里的数据呢，可以这样

    ui->textEdit->setText(string[ui->comboBox->currentIndex()]);

    */
}


//下拉框改变的函数 影响当前选中的用户
void Widget::onCombBoxChanged(int index)
{
    //QMessageBox::warning(this, "Message:当前用户", ui->usersComboBox->itemText(index), QMessageBox::Ok);
    Widget::CurrentWorkUser = allUserList[ui->usersComboBox->currentIndex()];
    qDebug() << "index=" << index <<  "Widget::CurrentWorkUser=" << ui->usersComboBox->itemText(index);

    //无需点击 直接运行解析
    //Widget::on_mainBtn_clicked();
}

//运行py脚本 提取邮件内容
void Widget::on_mainBtn_clicked()
{

    onTimeCheckFinish = new QTimer(this);
    onTimeCheckFinish->start(1000);
    connect(onTimeCheckFinish, SIGNAL(timeout()), this, SLOT(check_finish_time_out()));

    qDebug() << "python begin to getMailInfo";
    QProcess p(0);
    p.startDetached("cmd", QStringList()<<"/c"<<"python NetEase.py" << userNameAndPathM_map[Widget::CurrentWorkUser]);
    qDebug() << "begin python NetEase.py ";

}

//实时检测的定时器槽函数
void Widget::on_time_out()
{
        HWND hq = FindWindow(L"MailNotificationWindow", NULL);
        if (hq)
        {
            qDebug("find notify window id=%x\n", hq);
            QMessageBox::warning(this,
                        "请注意",
                        "新邮件到达了哦",
                         QMessageBox::Ok);

            //新邮件到达 后自动更新
            Widget::on_mainBtn_clicked();
        }
        else {
            qDebug("\nwaiting........\n");
        }


        //暂时停止倒计时的功能
        //Widget::onTimeCheckNewMail->stop();
}

//检测脚本执行结束完成的定时器
void Widget::check_finish_time_out()
{
    QString mailInfoFileName =  QString("%1-MailInfo.txt").arg(Widget::CurrentWorkUser); ;
    QString contactInfoFileName = QString("%1-Contacts.txt").arg(Widget::CurrentWorkUser);
    if(QFile::exists(mailInfoFileName)  && QFile::exists(contactInfoFileName))
    {
          qDebug() <<  "检测完成";
          onTimeCheckFinish->stop();
          ui->textBrowser->setText("数据提取成功,请点击查看");
          Widget::globalMailInfoList   = Widget::parseMailInfo();
          Widget::globalContactsInfoList = Widget::parseContactsInfo();
          qDebug() << "globalMailInfoList size=" <<  globalMailInfoList.count();
          qDebug() << "globalContactsInfoList size=" <<  globalContactsInfoList.count();
    }
}



//解析文件 获取内容
QStringList Widget::parseMailInfo()
{
    QStringList result;
    QString fileName = QString("%1-MailInfo.txt").arg(Widget::CurrentWorkUser);
    QFile file(fileName);
    QTextCodec *codec=QTextCodec::codecForName("UTF-8");
     QTextStream in(&file);
    in.setCodec(codec);
    if(!file.open(QFile::ReadOnly|QFile::Text))
    {
        qDebug() <<"can not open file";
        result << "empty";
        return result;
    }
    //QString line = codec->fromUnicode(in.read(128));
    //qDebug()  << "line=" << line;

    QString MailInfo =  codec->fromUnicode(in.readAll());
    qDebug() << "MailInfo=" <<  MailInfo;
    //ui->textBrowser->setText(MailInfo);
    file.close();
    result = MailInfo.split(Widget::largeSplit);
    result.removeLast();
//    for(int i=0;i<result.count();i++)
//    {
//        QStringList eachMailInfo = result[i].split(Widget::smallSplit);
//         for(int ii=0;ii<eachMailInfo.count();ii++)
//        {
//            qDebug() << eachMailInfo[ii];
//            qDebug() << "\n--------------------------------------------------------\n";
//        }
//    }
    return result;
}

QStringList Widget::parseContactsInfo()//解析联系人内容
{
    QStringList result;
    QString fileName = QString("%1-Contacts.txt").arg(Widget::CurrentWorkUser);
    QFile file(fileName);
    QTextCodec *codec=QTextCodec::codecForName("UTF-8");
     QTextStream in(&file);
    in.setCodec(codec);
    if(!file.open(QFile::ReadOnly|QFile::Text))
    {
        qDebug() <<"can not open file";
        result << "empty";
        return result;
    }
    QString ContactsInfo =  codec->fromUnicode(in.readAll());
    qDebug() << "ContactInfo=" <<  ContactsInfo;
//    ui->textBrowser->setText(ContactsInfo);
    file.close();
    result = ContactsInfo.split(Widget::largeSplit);
    result.removeLast();
//    for(int i=0;i<result.count();i++)
//    {
//        QStringList eachMailInfo = result[i].split(Widget::smallSplit);
//         for(int ii=0;ii<eachMailInfo.count();ii++)
//        {
//            qDebug() << eachMailInfo[ii];
//            qDebug() << "\n--------------------------------------------------------\n";
//        }
//    }
    return result;

}

//显示邮件的内容按钮点击 在tableView中填充内容
void Widget::on_showMailInfoBtn_clicked()
{
    //清空原始内容
    mainModel->removeRows(0,mainModel->rowCount());

    int totalMailSize = Widget::globalMailInfoList.count();
    for(int i=0;i<totalMailSize;i++)
    {
        QStringList eachContact = globalMailInfoList[i].split(Widget::smallSplit);
        mainModel->setItem(i,0,new QStandardItem(QString(eachContact.at(0)).simplified()));
        mainModel->setItem(i,1,new QStandardItem(QString(eachContact.at(1)).simplified()));
        mainModel->setItem(i,2,new QStandardItem(QString(eachContact.at(2))));
        mainModel->setItem(i,3,new QStandardItem(QString(eachContact.at(3))));
        //注意 eachContact.at(4)是正文    .at(5)才是附件列表
        mainModel->setItem(i,4,new QStandardItem(QString(eachContact.at(5))));
    }
    //根据内容设置宽度
    ui->tableView->resizeColumnsToContents();

//调试 在textBrowser中输出所有邮件的内容
/*
   QString finalContactString;
   for(int i=0;i<Widget::globalMailInfoList.count();i++)
   {
       //CID,NameIndex,Name,Email
       //.at(4)为正文
            QStringList eachContact = globalMailInfoList[i].split(Widget::smallSplit);
            //eachContact.replaceInStrings("\n","");
            QString tmpContact  = QString("Num: %1\nID: %2\n主题: %3\n发件人: %4\n收件人: %5\n正文: %6\n附件文件列表: %7\n-----------------------------------------------\n").arg(i+1).arg(QString(eachContact.at(0)).simplified()).arg(QString(eachContact.at(1)).simplified()).arg(QString(eachContact.at(2)).replace("\n","  ")).arg(QString(eachContact.at(3)).replace("\n","  ")).arg(eachContact.at(4)).arg(QString(eachContact.at(5)).simplified());
            finalContactString += tmpContact;
   }
   ui->textBrowser->setText(finalContactString);


*/

}


//双击显示正文的槽
void Widget::slotRowDoubleClicked(const QModelIndex index)
{
     int row  = index.row();
     qDebug() << "row======" << row;
    qDebug() <<  "index========"  << index;
    QString data = index.data().toString();
    qDebug() << "data=============" << data;
    {
            QStringList eachMailInfo = Widget::globalMailInfoList[row].split(Widget::smallSplit);
            qDebug() << "eachContact=" << eachMailInfo;
            QString mainContent =   QString(eachMailInfo.at(4));
            qDebug() << "mainContent = " << mainContent;
            ui->textBrowser->clear();
            ui->textBrowser->setText(mainContent);
    }
}




void Widget::on_showContactsInfoBtn_clicked()
{

    QString finalContactString = "联系人信息:\n";
    for(int i=0;i<Widget::globalContactsInfoList.count();i++)
    {
            //CID,NameIndex,Name,Email
             QStringList eachContact = globalContactsInfoList[i].split(Widget::smallSplit);
             eachContact.replaceInStrings("\n","");
             QString tmpContact  = QString("Num: %1\nNameIndex: %2\nName: %3\nEmail: %4\n-----------------------------------------------\n").arg(i+1).arg(eachContact.at(1)).arg(eachContact.at(2)).arg(eachContact.at(3));
             finalContactString += tmpContact;
    }
    ui->textBrowser->setText(finalContactString);

}
