#ifndef MUSICWIDGET_H
#define MUSICWIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include"csqlite.h"
#include"customwidget.h"
#include"QCloseEvent"
#include"downloadwidget.h"

//快捷键  F1 帮助 F2 以及shift+F2 切换函数 调试 F 9 10 11  保存所有 ctrl + shift + S
//搜索 ctrl + F   注释 ctrl + /
#define MAX_MUSIC_COUNT  (200)//歌单最大容量
#define MAX_LYRIC_COUNT  (1000)//歌词缓存的最大容量

namespace Ui {
class MusicWidget;
}

class MusicWidget : public CustomMoveWidget
{
    Q_OBJECT

public:
    explicit MusicWidget(QWidget *parent = 0);
    ~MusicWidget();

private slots:
    void on_pb_play_clicked();

    void on_pb_addMusic_clicked();

    void on_lw_musicList_doubleClicked(const QModelIndex &index);

    void on_pb_prev_clicked();

    void on_pb_next_clicked();
 //自定义槽函数
    void slot_musicPositionChanged(qint64 val) ; //音乐 位置变换

    void on_slider_musicProcess_valueChanged(int value);
    void slot_musicBackgroundImage();

    void loadSqlAndSetMusicList(); // 加载sql 并设置歌单 音量
    void on_pb_min_clicked();

    void on_pb_close_clicked();


    virtual void closeEvent(QCloseEvent *event);

    void on_slider_volumn_valueChanged(int value);

    void on_pb_volumn_clicked();

    void on_pb_deleteMusic_clicked();

    void on_pb_show_clicked();

    void on_pb_search_clicked();

    void slot_PlayOnline(QString strUrl , QString musicName);

    void slot_musicDownloaded(QString FilePath );

    void slot_setMedia(QString url);
private:
    Ui::MusicWidget *ui;

    QString m_musicList[MAX_MUSIC_COUNT]; //缓存歌曲路径
    QString m_musicLyricLsit[MAX_LYRIC_COUNT];//歌词的缓冲数组
    quint32 m_musicCount;  //歌曲个数  uint
    quint32 m_musicLyricCount;//歌词行数 uint
    bool m_musicStartFlag;//播放的标志
    bool m_musicPositionChangedFlag;//音乐位置改变标志

    QString m_currentMusicName;//当前歌曲的名字
    QMediaPlayer * m_Player;

    quint32 m_volumnNow     ;
    quint32 m_volumnLast    ; //用于静音恢复
    bool    m_voiceOpenFlag ; //是否静音
    CSqlite *m_sql;//sqlite 数据库

    DownloadWidget *m_downloadWidget;
};

#endif // MUSICWIDGET_H
