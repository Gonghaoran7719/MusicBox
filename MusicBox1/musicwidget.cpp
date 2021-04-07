#include "musicwidget.h"
#include "ui_musicwidget.h"
#include<QFileDialog>
#include<QTime>
//#include<QFile>
#include<qDebug>
#include<QMessageBox>
#include<QDir>

MusicWidget::MusicWidget(QWidget *parent) :
    CustomMoveWidget(parent),
    ui(new Ui::MusicWidget)
{
    ui->setupUi(this);
    m_musicCount = 0;  //歌曲个数  uint
    m_musicLyricCount = 0;
    m_musicStartFlag = false;//播放的标志
    m_musicPositionChangedFlag = false;
    m_volumnNow     = 0 ;
    m_volumnLast    = 0 ;
    m_voiceOpenFlag = true;
    m_Player = new QMediaPlayer;

    m_sql = new CSqlite;
    //加载数据库
    loadSqlAndSetMusicList();
 //   m_Player->setVolume( 30 );  //设置音量
    //进度条
    QObject::connect( m_Player ,SIGNAL(positionChanged(qint64)) ,
                      this , SLOT( slot_musicPositionChanged(qint64)) );

    //设置默认窗口背景
        QPixmap pixmap = QPixmap(":/images/start.png");
        pixmap = pixmap.scaled(this->width(),this->height()); // scaled 等比例缩放
        QPalette pal(this->palette()); // QPalette画板 控件的背景
        pal.setBrush(QPalette::Background , QBrush(pixmap));
        this->setPalette( pal );

        m_downloadWidget = new DownloadWidget();
        connect( m_downloadWidget , SIGNAL(SIG_PlayOnline(QString,QString)) ,
                 this, SLOT(slot_PlayOnline(QString,QString))) ;
        connect( m_downloadWidget, SIGNAL(SIG_musicDownloaded(QString)) ,
                 this, SLOT(slot_musicDownloaded(QString)) );
        m_downloadWidget->hide();
}

MusicWidget::~MusicWidget()
{
    if( m_downloadWidget )
        delete m_downloadWidget;

    if( m_sql )
        delete m_sql;
    delete ui;
}


//点击播放按钮  播放(未开始到播放 暂停到播放 ) -- 暂停(从播放到暂停)
void MusicWidget::on_pb_play_clicked()
{
//    m_Player->setMedia( QUrl::fromLocalFile("./music/周杰伦 - 稻香 (Demo).mp3") );
//    m_Player->play();
    int inx = ui->lw_musicList->currentRow();
    qDebug()<< "inx" <<inx ;
    //异常处理  就是没有歌
    if(  m_musicCount  == 0 )
    {
        //回到初始状态
        m_Player->pause();
        m_musicStartFlag = false;
        ui->pb_play->setIcon( QIcon(":/images/bfzn_play.png"));
        return;
    }
    if( inx < 0 )
    {
        ui->lw_musicList->setCurrentRow(0);
    }

    if(  m_musicStartFlag == false )  // !m_musicStartFlag--未播放到播放 播放(未开始到播放 暂停到播放 )
    {
        //设置媒体文件 -- 如果是暂停的 不需要设置  如果当前歌曲 和你列表中选中的不一致, 要设置
        if( m_Player->state() == QMediaPlayer::StoppedState
                || m_currentMusicName != ui->lw_musicList->currentItem()->text() )//未开始QMediaPlayer::StoppedState
        {
            //m_Player->setMedia( QUrl::fromLocalFile( m_musicList[ ui->lw_musicList->currentIndex().row() ]) ); //当前列表中选中项对应的绝对地址
            this->slot_setMedia( m_musicList[ ui->lw_musicList->currentIndex().row() ] );
            //设置背景
            slot_musicBackgroundImage();
        }
        m_Player->play();

        //UI切换  ":/images/bfzn_pause.png" 使用导入资源 :/   QIcon图标类
        ui->pb_play->setIcon( QIcon(":/images/bfzn_pause.png") );

        //存储当前播放的歌曲名  存缓存  和设置控件  ui->lw_musicList->currentItem() 当前选中的控件
        m_currentMusicName = ui->lw_musicList->currentItem()->text();
        ui->lb_currentMusicName->setText( m_currentMusicName );

        //需要切换 播放状态
        m_musicStartFlag = true;


    }else //暂停(从播放到暂停)
    {
        //回到初始状态
        m_Player->pause();
        m_musicStartFlag = false;
        ui->pb_play->setIcon( QIcon(":/images/bfzn_play.png"));
    }
}

//点击添加音乐
void MusicWidget::on_pb_addMusic_clicked()
{
    //弹窗 选择音乐文件  获取文件的绝对路径   存路径 需要字符串的数组
    QStringList path = QFileDialog::getOpenFileNames( this,"选择歌曲","./music");  //1.父类 2.标题 3.默认路径 4.文件过滤
    //去重
    bool hasSame = false;
    for( int i = 0 ; i < path.count() ; ++i)
    {
        hasSame = false;
        for( int j = 0 ; j < m_musicCount ; ++j) //遍历歌单看有没有重复的
        {
            if( path[i] == m_musicList[j])
            {
                hasSame = true;
                break;
            }
        }
        if( !hasSame)
        {
            m_musicList[ m_musicCount++ ]  = path[i];
            //同时添加到控件  -- 添加歌曲名
            QFileInfo  info(path[i]);
            //info.baseName(); // 文件名   /music/稻香.mp3  -->baseName 是 稻香
            ui->lw_musicList->addItem( info.baseName() );

            // 插入sql
            QString sqlStr =
                    QString("insert into t_musicList (musicName , musicPath) values ('%1','%2')")
                    .arg(info.baseName()).arg(path[i]); //格式化字符串
            m_sql->UpdateSql( sqlStr );
        }
    }
}

//双击歌单 , 播放
void MusicWidget::on_lw_musicList_doubleClicked(const QModelIndex &index)
{//ui->lw_musicList->currentIndex().row() 当前列表选中项
    //设置媒体文件
    //m_Player->setMedia( QUrl::fromLocalFile( ) ); //当前列表中选中项对应的绝对地址
    this->slot_setMedia( m_musicList[ ui->lw_musicList->currentIndex().row() ]  );
    m_Player->play();

    //UI切换  ":/images/bfzn_pause.png" 使用导入资源 :/   QIcon图标类
    ui->pb_play->setIcon( QIcon(":/images/bfzn_pause.png") );

    //存储当前播放的歌曲名  存缓存  和设置控件  ui->lw_musicList->currentItem() 当前选中的控件
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    ui->lb_currentMusicName->setText( m_currentMusicName );

    //需要切换 播放状态
    m_musicStartFlag = true;

    //设置背景
    slot_musicBackgroundImage();
}

//上一曲 按钮
void MusicWidget::on_pb_prev_clicked()
{
    //异常处理  就是没有歌
    if(  m_musicCount  == 0 )
    {
        //回到初始状态
        m_Player->pause();
        m_musicStartFlag = false;
        ui->pb_play->setIcon( QIcon(":/images/bfzn_play.png"));
        return;
    }
    //切换到上一曲  注意 : 第一手会切换到最后一首

    //设置媒体文件
    if( ui->lw_musicList->currentIndex().row() - 1 >= 0 )  //当前是从第二首开始 >=0 row >=1  下标从0开始
    {
        //m_Player->setMedia( QUrl::fromLocalFile( ) ); //当前列表中选中项对应的绝对地址
        this->slot_setMedia( m_musicList[ ui->lw_musicList->currentIndex().row()-1 ]  );
        //ui 控件要切换到上一个 设置焦点
        ui->lw_musicList->setCurrentRow( ui->lw_musicList->currentIndex().row()-1 );
    }else
    {
       // m_Player->setMedia( QUrl::fromLocalFile( ) );
        this->slot_setMedia( m_musicList[ m_musicCount -1 ]  );
        ui->lw_musicList->setCurrentRow( m_musicCount -1 );
    }

    m_Player->play();

    //UI切换  ":/images/bfzn_pause.png" 使用导入资源 :/   QIcon图标类
    ui->pb_play->setIcon( QIcon(":/images/bfzn_pause.png") );

    //存储当前播放的歌曲名  存缓存  和设置控件  ui->lw_musicList->currentItem() 当前选中的控件
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    ui->lb_currentMusicName->setText( m_currentMusicName );

    //需要切换 播放状态
    m_musicStartFlag = true;
    //设置背景
    slot_musicBackgroundImage();
}

//点击下一曲按钮
void MusicWidget::on_pb_next_clicked()
{
    //异常处理  就是没有歌
    if(  m_musicCount  == 0 )
    {
        //回到初始状态
        m_Player->pause();
        m_musicStartFlag = false;
        ui->pb_play->setIcon( QIcon(":/images/bfzn_play.png"));
        return;
    }
    //切换到下一曲  注意:最后一个会切换到第一首   -->  %  循环队列

    //设置媒体文件
//    m_Player->setMedia( QUrl::fromLocalFile(
//                            m_musicList[ (ui->lw_musicList->currentIndex().row()+1)%m_musicCount ]
//                        ) ); //当前列表中选中项对应的绝对地址
    this->slot_setMedia( m_musicList[ (ui->lw_musicList->currentIndex().row()+1)%m_musicCount ]  );
    //ui 控件要切换到下一个 设置焦点
    ui->lw_musicList->setCurrentRow( (ui->lw_musicList->currentIndex().row()+1)%m_musicCount );

    m_Player->play();

    //UI切换  ":/images/bfzn_pause.png" 使用导入资源 :/   QIcon图标类
    ui->pb_play->setIcon( QIcon(":/images/bfzn_pause.png") );

    //存储当前播放的歌曲名  存缓存  和设置控件  ui->lw_musicList->currentItem() 当前选中的控件
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    ui->lb_currentMusicName->setText( m_currentMusicName );

    //需要切换 播放状态
    m_musicStartFlag = true;
    //设置背景
    slot_musicBackgroundImage();
}

//音乐 位置变化
void MusicWidget::slot_musicPositionChanged(qint64 val)
{
    static bool lrcBeginFlag = false;
    if( m_Player->state() == QMediaPlayer::PlayingState )
    {
        if( m_Player->duration() ) //m_Player->duration() 歌曲持续多久 ms数
        {
            //1. 计算当前歌曲时间 总时间   QTime
            //m_Player->duration() 歌曲持续多久  m_Player->position() 歌曲现在的位置 ms数
            //保存当前时间 ( 小时, 分钟 , 秒 , 毫秒);//qRound 四舍五入 传参是一个小数
            QTime duration1( 0,m_Player->position()/60000 , qRound(m_Player->position()%60000/1000.0) ,0) ;
            //总时间
            QTime duration2( 0,m_Player->duration()/60000 , qRound(m_Player->duration()%60000/1000.0) ,0) ;
            //1.1 更新 进度条  更新 音乐时间
            m_musicPositionChangedFlag = true;
            ui->slider_musicProcess ->setValue( m_Player->position()*100/m_Player->duration() );
            //更新 音乐时间
            QString tm1 = duration1.toString("mm:ss");
            QString tm2 = duration2.toString("mm:ss");
            tm1 = tm1 + "/" + tm2;
            ui->lb_musicTime->setText( tm1 );
            //2. 当前 == 总 切歌 --> 下一首 播放模式是下一曲
            if( duration1 == duration2 )
            {
                // 下一曲
                this->on_pb_next_clicked();
            }
            //关于歌词显示的逻辑
            if( m_musicLyricLsit[0] != m_currentMusicName )//相等意味着已经加载歌词,要去显示, 不等就是要去加载
            {//不等就是要去加载
                QFile lrcFile("./lrc/"+m_currentMusicName+".txt"); // ./ 的位置 debug文件夹
//如果点exe运行, ./ 与exe同级.
                ui->lw_lyric->clear(); //清空歌词
                if( lrcFile.open( QIODevice::ReadOnly ))
                {//打开成功, 加载歌词
                    //读每一行到数组
                    QTextStream lrcFileStream(&lrcFile);
                    lrcFileStream.setCodec( "UTF-8");//设置编码utf-8
                    lrcBeginFlag = true;
                    m_musicLyricCount = 0;
                    m_musicLyricLsit[m_musicLyricCount++] = m_currentMusicName;

                    while( !lrcFileStream.atEnd() ) //文件没有到结束就循环
                    {
                        QString line = lrcFile.readLine();//从文件中读取一行
                        m_musicLyricLsit[m_musicLyricCount++] = line;
                    }
                    qDebug()<<"lrc read success!line:"<< m_musicLyricCount;
                    lrcFile.close();

                }else
                {//打开失败, 没有歌词
                    ui->lw_lyric->clear(); //清空歌词
                    m_musicLyricCount = 0;
                    //控件显示无歌词
                    ui->lw_lyric->addItem("");
                    ui->lw_lyric->addItem("当前歌曲无歌词");  // 需要高亮标记
                    ui->lw_lyric->addItem("");

                    ui->lw_lyric->setCurrentRow(1);
                    //字体变大
                    QFont font;
                    font.setPointSize(18);
                    ui->lw_lyric->currentItem()->setFont(font);//设置字体
                    ui->lw_lyric->currentItem()->setTextAlignment( Qt::AlignCenter);//中间居中对齐 上下左右
                    ui->lw_lyric->currentItem()->setTextColor( Qt::yellow ); //设置颜色
                }
            }else
            {//相等意味着已经加载歌词,要去显示
                //初次加载歌词 避免前几行时间没有匹配上,暂时没有歌词的情况
                if( lrcBeginFlag )
                {
                    lrcBeginFlag = false;
                    ui->lw_lyric->clear(); //清空歌词
                    //初次加载歌词前9行
                    int lrcAddLine = 0;
                    for( int i = 0 ; i < m_musicLyricCount && i < 9 ; i++)
                    {
                        QStringList lrc = m_musicLyricLsit[i].split(']');//根据] 分割字符串
                        if( lrc.count() == 2)
                        {
                            ui->lw_lyric->addItem( lrc.at(1));
                        }else
                        {
                            ui->lw_lyric->addItem(m_musicLyricLsit[i]);
                        }
                        lrcAddLine++;
                        ui->lw_lyric->setCurrentRow( lrcAddLine -1 );
                        ui->lw_lyric->currentItem()->setTextAlignment(Qt::AlignCenter);
                    }
                }
                //匹配时间
                int currentMusicRow = 0;
                for( currentMusicRow = 0; currentMusicRow < m_musicLyricCount ; ++currentMusicRow )
                {
                    QString mm ,ss ;
                    //QString 下面的mid函数就是取字符串子串的函数
                    mm = m_musicLyricLsit[currentMusicRow].mid(1,2);//[00:29.640]我是浪花的泡沫 从下标1的位置取2个长度子串
                    ss = m_musicLyricLsit[currentMusicRow].mid(4,2);
                    QTime duration3 (0,mm.toInt() , ss.toInt() , 0); // 构造函数 参数 ( 小时, 分钟 , 秒 , 毫秒)
                    if( duration1 == duration3)  //当前时间等于歌词的时间 --> 匹配到要刷新歌词了
                    {
                        break;
                    }
                }
                // 读取 currentMusicRow 上面4行 以及下面4行 的9行歌词 显示到UI 然后中间那行高亮
                //向上和向下可能越界
                if( currentMusicRow < m_musicLyricCount )
                {//匹配到
                    ui->lw_lyric->clear(); //清空歌词
                    int lrcAddLineCount = 0;// 用来做控件显示用的临时变量, 不断设置居中
                    for( int i = currentMusicRow - 4 ; i <= currentMusicRow + 4 ;++i)
                    {
                        if( i >=0 && i < m_musicLyricCount ) //不可越界
                        {
                            //提取歌词
                            QStringList lrc = m_musicLyricLsit[i].split(']');//根据] 分割字符串
                            if( lrc.count() == 2)
                            {
                                ui->lw_lyric->addItem( lrc.at(1));
                            }else
                            {
                                ui->lw_lyric->addItem(m_musicLyricLsit[i]);
                            }
                            lrcAddLineCount++;
                            ui->lw_lyric->setCurrentRow( lrcAddLineCount -1 );
                            ui->lw_lyric->currentItem()->setTextAlignment(Qt::AlignCenter); //设置居中
                        }else{ //越界也要补齐9行  补空字符串
                            ui->lw_lyric->addItem("");
                            lrcAddLineCount++;
                            ui->lw_lyric->setCurrentRow( lrcAddLineCount -1 );
                            ui->lw_lyric->currentItem()->setTextAlignment(Qt::AlignCenter);//设置居中
                        }
                        //高亮
                        if( i == currentMusicRow )
                        {
                            //字体变大
                            QFont font;
                            font.setPointSize(18);
                            ui->lw_lyric->setCurrentRow(4); // 控件上一定 9行中间 也就是4
                            ui->lw_lyric->currentItem()->setFont(font);//设置字体
                            ui->lw_lyric->currentItem()->setTextAlignment( Qt::AlignCenter);//中间居中对齐 上下左右
                            ui->lw_lyric->currentItem()->setTextColor( Qt::yellow ); //设置颜色
                        }
                    }
                }
            }
            ui->lw_lyric->setCurrentRow(-1);
        }
    }
}

//进度条 进度改变  //1. setValue来的  2.手动拖动进度
void MusicWidget::on_slider_musicProcess_valueChanged(int value)
{
    if( !m_musicPositionChangedFlag) //手动的
    {
        if( m_musicStartFlag )
        {
            m_Player->pause();
            m_Player->setPosition( value* m_Player->duration() /100 );
            m_Player->play();
        }else
        {
            m_Player->setPosition( value* m_Player->duration() /100 );
        }
    }else//setvalue来的
    {
        m_musicPositionChangedFlag = false;
    }
}

//切换音乐背景图片
void MusicWidget::slot_musicBackgroundImage()
{
    //根据歌曲来切换 先找png的图片 没有然后找jpg的, 然后都没有 就是默认的.
    QPixmap pixmap = QPixmap("./images/"+ m_currentMusicName +".png");
    if( pixmap .isNull() == false )
    {
        pixmap = pixmap.scaled(this->width(),this->height()); // scaled 等比例缩放
        QPalette pal(this->palette()); // QPalette画板 控件的背景
        pal.setBrush(QPalette::Background , QBrush(pixmap));
        this->setPalette( pal );
    }else
    {
        QPixmap pixmap = QPixmap("./images/"+ m_currentMusicName +".jpg");
        if( pixmap .isNull() == false )
        {
            pixmap = pixmap.scaled(this->width(),this->height()); // scaled 等比例缩放
            QPalette pal(this->palette()); // QPalette画板 控件的背景
            pal.setBrush(QPalette::Background , QBrush(pixmap));
            this->setPalette( pal );
        }else
        {
            //默认背景
            QPixmap pixmap = QPixmap(":/images/start.png");
            pixmap = pixmap.scaled(this->width(),this->height()); // scaled 等比例缩放
            QPalette pal(this->palette()); // QPalette画板 控件的背景
            pal.setBrush(QPalette::Background , QBrush(pixmap));
            this->setPalette( pal );
        }
    }
}

void MusicWidget::loadSqlAndSetMusicList() // 加载sql 并设置歌单 音量
{
    //首先要获取路径  设置sql
    QString DBDir = QDir::currentPath() + "/sql/";
    QString FileName = "music.db";
    QDir tempDir;
    tempDir.setPath( DBDir );
    //看路径是否存在 //路径是否存在 没有就创建
    if( !tempDir.exists( DBDir  ))
    {
        qDebug()<< "不存在该路径";
        tempDir.mkdir(DBDir );
    }
    // 需要 images  music  lrc
    if( !tempDir.exists( QDir::currentPath() + "/music/"  ))
    {
        qDebug()<< "不存在该路径";
        tempDir.mkdir( QDir::currentPath() + "/music/"  );
    }
    if( !tempDir.exists( QDir::currentPath() + "/images/"  ))
    {
        qDebug()<< "不存在该路径";
        tempDir.mkdir( QDir::currentPath() + "/images/" );
    }
    if( !tempDir.exists( QDir::currentPath() + "/lrc/"  ))
    {
        qDebug()<< "不存在该路径";
        tempDir.mkdir( QDir::currentPath() + "/lrc/"  );
    }

    //看有没有数据库
//    其中 t_musicList 包含字段为musicName varchar(260) , musicPath varchar(260)
//    sql语句:
//    create table t_musicList (musicName varchar(260) , musicPath varchar(260));
//    t_volumn 包含的字段为 volumn int
//    sql 语句:
//    create table t_volumn ( volumn int);
//    insert into t_volumn (volumn ) values( 30 );

    QFile tempFile ;
    if( tempFile.exists( DBDir + FileName) )
    {//有数据库 加载
        qDebug()<< "有数据库";
        m_sql->ConnectSql( DBDir + FileName  ); //传入数据库绝对路径
        QStringList resList;
        QString sqlStr = "select musicName , musicPath from t_musicList;";
        bool res = m_sql->SelectSql( sqlStr , 2 , resList );

        if( !res )
        {
            return;
        }
        //把 查到的结果放到 数组和控件
        for( int i  =0 ; i < resList.count() ; i+=2 )
        {
            ui->lw_musicList->addItem( resList[i] );//名字
            m_musicList[ m_musicCount++ ] = resList[i + 1];//全路径
        }
        //音量
        resList.clear();
        sqlStr = "select volumn from t_volumn;";
        res = m_sql->SelectSql( sqlStr , 1 , resList );
        if( !res ) return;
        if( !resList.isEmpty() )
        {
            m_volumnLast = m_volumnNow = ((QString)resList[0]).toInt();

            qDebug()<< "m_volumnNow" << m_volumnNow;
            if( m_volumnNow == 0)
            {
                m_voiceOpenFlag = true;
                this->on_slider_volumn_valueChanged(0);
                m_volumnLast = 30;
            }else
            {
                ui->slider_volumn->setValue( m_volumnNow );
            }
        }
    }else
    {//没有数据库 要创建
        qDebug()<< "没有数据库";
        tempFile.setFileName( DBDir + FileName );
        if( !tempFile.open(QIODevice::WriteOnly | QIODevice::Text )) //创建文件
        {
            qDebug()<< "数据库创建失败";
            QMessageBox::information(this , "提示","数据库创建失败,会导致无法写入歌曲");

        }else
        {
            qDebug()<< "数据库创建";
            tempFile.close();
            m_sql->ConnectSql( DBDir + FileName  ); //传入数据库绝对路径
            //创建表
            QString sqlStr = "create table t_musicList (musicName varchar(260) , musicPath varchar(260));";
            m_sql->UpdateSql( sqlStr );

            sqlStr = "create table t_volumn ( volumn int);";
            m_sql->UpdateSql( sqlStr );

            sqlStr ="insert into t_volumn (volumn ) values( 30 );";
            m_sql->UpdateSql( sqlStr );

            ui->slider_volumn->setValue(30);//所有相关的ui会联动
        }
    }

}

//最小化窗口
void MusicWidget::on_pb_min_clicked()
{
    this->showMinimized();
}


//关闭窗口
void MusicWidget::on_pb_close_clicked()
{
    //关闭子窗口

    //关闭主窗口
    this->close();
}

void MusicWidget::closeEvent(QCloseEvent *event)
{
    //close() 触发 关闭事件
    if( QMessageBox::question(this,"关闭程序","是否关闭程序?" ,QMessageBox::Yes|QMessageBox::No , QMessageBox::No)
            == QMessageBox::Yes )
    {
        QString  sqlStr = QString("update t_volumn set volumn = %1;").arg( m_volumnNow);
        m_sql->UpdateSql( sqlStr );
        event->accept(); //执行关闭
    }else
    {
        event->ignore(); //不执行
    }
}

//音量改变处理
void MusicWidget::on_slider_volumn_valueChanged(int value)
{
    qDebug()<<__FUNCTION__;
    m_Player->setVolume( value );

    if( m_volumnNow != value )
    {
        m_volumnLast = m_volumnNow;
        m_volumnNow = value;
    }
    //更新控件
    ui->lb_volumn->setText( QString::number(value)+"%" );
    //是否需要改变喇叭 -- 静音
    if( value == 0)
    {
        if( m_voiceOpenFlag )
        {
            ui->pb_volumn->setIcon(QIcon (":/images/voice_close.png"));
        }
        m_voiceOpenFlag = false;
    }else
    {
        if( !m_voiceOpenFlag )
        {
            ui->pb_volumn->setIcon(QIcon (":/images/voice_open.png"));
        }
        m_voiceOpenFlag = true;
    }
}

//切换 静音 和 非静音
void MusicWidget::on_pb_volumn_clicked()
{
    if( m_voiceOpenFlag )//当前是非静音 --> 静音
    {
        m_voiceOpenFlag = false;
        ui->pb_volumn->setIcon( QIcon (":/images/voice_close.png"));
        ui->slider_volumn->setValue( 0 );
    }else
    {
        m_voiceOpenFlag = true;
        ui->pb_volumn->setIcon( QIcon(":/images/voice_open.png"));
        ui->slider_volumn->setValue( m_volumnLast );
    }
}

//删除歌曲
void MusicWidget::on_pb_deleteMusic_clicked()
{
    //异常
    if( m_musicCount <= 0 ) return;
    //删除数据库内容
    int inx = ui->lw_musicList->currentRow();
    if( inx < 0 ) return;
    QString sqlStr = QString("delete from t_musicList where musicPath ='%1';")
            .arg( m_musicList[inx] );
    m_sql->UpdateSql( sqlStr );

    //删除数组 count--
    for( int i= inx ; i < m_musicCount -1 ; ++i)
    {
        m_musicList[i] = m_musicList[i+1];
    }
    m_musicCount--;

    //删除控件 takeitem
    ui->lw_musicList->takeItem(inx);

    ui->lw_musicList->setCurrentRow(-1);
}

//显示网络模块
void MusicWidget::on_pb_show_clicked()
{
    if( m_downloadWidget ->isHidden() )
    {
        m_downloadWidget->showNormal();
    }else
    {
        m_downloadWidget->hide();
    }
}

//搜索网络歌曲
void MusicWidget::on_pb_search_clicked()
{
    //搜索
    if( !ui->le_search->text().isEmpty() )  //"" --> empty
    {
        m_downloadWidget->search( ui->le_search->text() );
    }
    m_downloadWidget->showNormal();
}

//处理播放网络歌曲
void MusicWidget::slot_PlayOnline(QString strUrl , QString musicName)
{
    bool hasSame = false;
    for( int j = 0 ; j < m_musicCount ; ++j) //遍历歌单看有没有重复的
    {
        if( strUrl == m_musicList[j])
        {
            hasSame = true;
            ui->lw_musicList->setCurrentRow( j );
            break;
        }
    }
    if( !hasSame )
    {
        m_musicList[ m_musicCount++ ]  = strUrl;
        ui->lw_musicList->addItem( musicName );
        ui->lw_musicList->setCurrentRow( m_musicCount - 1);
    }

    //设置媒体文件
    m_Player->setMedia( QUrl(strUrl) ); //当前列表中选中项对应的绝对地址
    m_Player->play();

    //UI切换  ":/images/bfzn_pause.png" 使用导入资源 :/   QIcon图标类
    ui->pb_play->setIcon( QIcon(":/images/bfzn_pause.png") );

    //存储当前播放的歌曲名  存缓存  和设置控件  ui->lw_musicList->currentItem() 当前选中的控件
    m_currentMusicName = musicName;
    ui->lb_currentMusicName->setText( m_currentMusicName );

    //需要切换 播放状态
    m_musicStartFlag = true;

    //设置背景
    slot_musicBackgroundImage();
}
//处理下载完成
void MusicWidget::slot_musicDownloaded(QString FilePath )
{

    //添加到控件
    //写表
    //弹窗告诉下载完成

    bool hasSame = false;
    for( int j = 0 ; j < m_musicCount ; ++j) //遍历歌单看有没有重复的
    {
        if( FilePath == m_musicList[j])
        {
            hasSame = true;
            break;
        }
    }
    if( !hasSame)
    {
        m_musicList[ m_musicCount++ ]  = FilePath;
        //同时添加到控件  -- 添加歌曲名
        QFileInfo  info(FilePath);
        //info.baseName(); // 文件名   /music/稻香.mp3  -->baseName 是 稻香
        ui->lw_musicList->addItem( info.baseName() );

        // 插入sql
        QString sqlStr =
                QString("insert into t_musicList (musicName , musicPath) values ('%1','%2')")
                .arg(info.baseName()).arg(FilePath); //格式化字符串
        m_sql->UpdateSql( sqlStr );
        //弹窗 告诉下载完成
        QMessageBox::information( this , "提示", QString("歌曲%1 下载完成, 已添加到歌单").arg(info.baseName()));
    }

}

//加载媒体文件或链接
void MusicWidget::slot_setMedia(QString url)
{
    if( url.contains("http")||url.contains("https"))
    {
        m_Player->setMedia( QUrl(url) );
    }else
    {
        m_Player->setMedia( QUrl::fromLocalFile(url) );
    }
}
