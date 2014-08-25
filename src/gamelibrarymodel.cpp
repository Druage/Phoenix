

#include <QSqlQuery>

#include "gamelibrarymodel.h"
#include "logging.h"


GameLibraryModel::GameLibraryModel(QObject *parent)
    : QSqlTableModel(parent)
{
    role_names = QSqlTableModel::roleNames();
    role_names.insert(TitleRole, "title");
    role_names.insert(ConsoleRole, "console");
    role_names.insert(TimePlayedRole, "timePlayed");
    role_names.insert(ArtworkRole, "artwork");

    base_query = "SELECT title, console, time_played, artwork FROM games";
    sort_column = 0;
    sort_order = static_cast<Qt::SortOrder>(-1); // default = no sort
    updateQuery();

    m_file_count = 0;
    m_progress = 0;

    scraper = new TheGamesDB(this);

}

GameLibraryModel::~GameLibraryModel()
{
    scraper->deleteLater();
}

QVariant GameLibraryModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlTableModel::data(index, role);
    if (role < Qt::UserRole) {
        return value;
    } else {
        int columnIdx = (role - Qt::UserRole - 1);
        return QSqlTableModel::data(this->index(index.row(), columnIdx), Qt::DisplayRole);
    }
}

QHash<int, QByteArray> GameLibraryModel::roleNames() const
{
    return role_names;
}

void GameLibraryModel::updateQuery() {
    QString q_str(base_query);
    QSqlQuery q(dbm.handle());;
    if (!search_terms.isEmpty())
        q_str.append(" WHERE " + category + " LIKE ?");

    if (sort_order != -1) {
        q_str.append(" ORDER BY ");
        q_str.append(role_names[Qt::UserRole + sort_column + 1]);
        q_str.append(sort_order == 0 ? " ASC" : " DESC");
    }

    q.prepare(q_str);
    if (!search_terms.isEmpty())
        q.bindValue(0, "%" + search_terms + "%");

    q.exec();
    setQuery(q);
}

void GameLibraryModel::sort(int column, Qt::SortOrder order)
{
    if (sort_column == column && sort_order == order)
        return;

    sort_column = column;
    sort_order = order;
    updateQuery();
}

void GameLibraryModel::setFilter(QString new_terms, QString new_category)
{
    if (search_terms == new_terms)
        return;
    category = new_category;
    search_terms = new_terms;
    updateQuery();
}

void GameLibraryModel::addFilters(QStringList &filter_list)
{
    filter_list << "n64"
                << "z64"
                << "nes"
                << "gba"
                << "gb"
                << "gbc"
                << "cue"
                << "sfc"
                << "smc";
}

QString GameLibraryModel::getSystem(QString suffix)
{
    QString system;
    if (suffix == "nes")
        system = "Nintendo Entertainment System";
    else if (suffix == "sfc" || suffix == "smc")
        system = "Super Nintendo";
    else if (suffix == "n64" || suffix == "z64")
        system = "Nintendo 64";
    else if (suffix == "gb" || suffix == "gbc")
        system = "Game Boy";
    else if (suffix == "gba")
        system = "Game Boy Advance";
    else {
        system = "Unknown";
        qCDebug(phxLibrary) << suffix << " was not handled";
    }
    return system;
}

void GameLibraryModel::setLabel(QString label)
{
    m_label = label;
    emit labelChanged(label);
}

void GameLibraryModel::setProgress(qreal progress)
{
    m_progress = progress;
    emit progressChanged(progress);
}

void GameLibraryModel::scanFolder(QString path)
{
    QDirIterator dir_iter(path, QDirIterator::Subdirectories);

    QVector<QFileInfo> files;

    QStringList filter;
    addFilters(filter);

    // FileInfo is added to a vector so the user can see how far along on the import progress
    // the library is.

    setLabel("Import Games");
    while (dir_iter.hasNext()) {
        dir_iter.next();
        QFileInfo info(dir_iter.fileInfo());
        if (info.isFile()) {
            for (int i=0; i < filter.size(); ++i) {
                if (info.suffix() == filter.at(i)) {
                    files.append(info);
                    break;
                }
            }
        }
    }

    dbm.handle().transaction();

    QSqlQuery query(dbm.handle());

    bool data_changed = false;

    m_file_count = files.size();
    qreal count = static_cast<qreal>(m_file_count);

    setLabel("Finding Artwork");

    for (qreal i=0; i < count; ++i) {

        QFileInfo file_info = files.at(i);
        //qCDebug(phxLibrary) << file_info.absoluteFilePath();


        query.prepare("INSERT INTO games (title, console, time_played, artwork)"
                      " VALUES (?, ?, ?, ?)");

        QString system = getSystem(file_info.suffix());

        if (system != "") {
            GameData data = scraper->getAllData(file_info.baseName(), system);
            //qCDebug(phxLibrary) << "gamedata: " << data.back_boxart << " " << data.front_boxart;
            if (data.title != "")
                query.bindValue(0, data.title);
            else
                query.bindValue(0, file_info.baseName());
            if (data.front_boxart != "")
                query.bindValue(3, data.front_boxart);
            else
                query.bindValue(3, "qrc:/assets/missing_artwork.png");

            query.bindValue(1, system);
            query.bindValue(2, "0h 0m 0s");

            updateQuery();


            setProgress((((i+1) / m_file_count) * 100.0));

        }

        if (query.exec())
            data_changed = true;

    }

    if (data_changed) {
        dbm.handle().commit();
        submit();
    }

}

