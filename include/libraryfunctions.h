#ifndef LIBRARYFUNCTIONS_H
#define LIBRARYFUNCTIONS_H

#include <QObject>
#include <QDirIterator>
#include <QStringList>
#include <QSqlQuery>

#include "gamelibrarymodel.h"
#include "thegamesdb.h"
#include "logging.h"

class LibraryFunctions : public QObject {
    Q_OBJECT

    Q_PROPERTY(qreal progress READ progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(GameLibraryModel model READ model WRITE setModel NOTIFY modelChanged)

public:
    LibraryFunctions();
    ~LibraryFunctions();

    void setProgress(qreal progress);
    void setLabel(QString label);

    QString label() const
    {
        return m_label;
    }

    qreal progress() const
    {
        return m_progress;
    }

    GameLibraryModel &model()
    {
        return m_model;
    }

    void setModel(GameLibraryModel &model);

public slots:
    void scanFolder(QString path);
    void setFilter(QString search_terms_, QString new_category);

signals:
    void progressChanged(qreal);
    void labelChanged(QString);
    void modelChanged();

private:
    TheGamesDB *scraper;
    GameLibraryModel m_model;
    int m_file_count;
    qreal m_progress;
    QString m_label;

    QString getSystem(QString suffix);
    void addFilters(QStringList &filter_list);

};

#endif // LIBRARYFUNCTIONS_H
