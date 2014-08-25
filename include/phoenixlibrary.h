#ifndef PHOENIXLIBRARY_H
#define PHOENIXLIBRARY_H

#include <QObject>
#include <QDirIterator>
#include <QStringList>
#include <QCryptographicHash>
#include <QFile>
#include <QThread>
#include <QSqlQuery>
#include <QResource>
#include <QXmlStreamReader>

#include "thegamesdb.h"
#include "gamelibrarymodel.h"
#include "logging.h"

class PhoenixLibrary: public QObject {
    Q_OBJECT
    Q_PROPERTY(QString folderPath READ folderPath WRITE setFolderPath NOTIFY folderPathChanged)
    Q_PROPERTY(qreal progress READ progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)

public:
    PhoenixLibrary();

    ~PhoenixLibrary();

    void setFolderPath(QString path);
    void setProgress(qreal progress);
    void setLabel(QString label);
    void setStart(bool start);


    QString folderPath()
    {
        return m_folder_path;
    }

    QString label() const
    {
        return m_label;
    }

    qreal progress() const
    {
        return m_progress;
    }

public slots:
    void scanFolder();
    void setModel(GameLibraryModel *model)
    {
        m_model = model;
        emit modelChanged();
    }

    void startImport(bool start);
    GameLibraryModel *model()
    {
        return m_model;
    }

private slots:

signals:
    void folderPathChanged(QString);
    void queryStaged();
    void labelChanged();
    void progressChanged();
    void startChanged();
    void modelChanged();
    void scanComplete();

private:
    QThread *import_thread;
    TheGamesDB *scraper;
    GameLibraryModel *m_model;
    QString m_folder_path;
    QString m_label;
    int m_progress;
    bool m_start;


    QString getSystem(QString suffix);
    void loadXml(QString file_path);
    void addFilters(QStringList &filter_list);

};

#endif // PHOENIXLIBRARY_H
