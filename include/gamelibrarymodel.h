
#ifndef GAMELIBRARYMODEL_H
#define GAMELIBRARYMODEL_H

#include <QSqlTableModel>

#include "thegamesdb.h"
#include "librarydbmanager.h"

class GameLibraryModel: public QSqlTableModel
{
    Q_OBJECT

public:
    GameLibraryModel(QObject *parent = 0);
    virtual ~GameLibraryModel();

    enum GameRoles {
        TitleRole = Qt::UserRole + 1,
        ConsoleRole,
        TimePlayedRole,
        ArtworkRole,
    };

    LibraryDbManager &manager()
    {
        return dbm;
    }

    void update()
    {
        updateQuery();
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

public slots:
    void setFilter(QString search_terms_, QString new_category);
    virtual void sort(int column, Qt::SortOrder order) Q_DECL_OVERRIDE;

signals:
    void progressChanged(qreal);
    void labelChanged(QString);

private:

    LibraryDbManager dbm;
    QString base_query;
    QString search_terms;
    QString category;
    QString m_label;
    int sort_column;
    Qt::SortOrder sort_order;
    QHash<int, QByteArray> role_names;

    int m_file_count;
    qreal m_progress;

    void updateQuery();

};



#endif
