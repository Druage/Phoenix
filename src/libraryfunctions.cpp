#include "libraryfunctions.h"
#include "qdebug.h"

LibraryFunctions::LibraryFunctions()
    //: QObject(parent)
{
    m_file_count = 0;
    m_progress = 0;
    scraper = new TheGamesDB(this);

}

LibraryFunctions::~LibraryFunctions()
{
    scraper->deleteLater();
}

void LibraryFunctions::setModel(GameLibraryModel &model)
{
    m_model = model;
    emit modelChanged();
}

void LibraryFunctions::addFilters(QStringList &filter_list)
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

QString LibraryFunctions::getSystem(QString suffix)
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

void LibraryFunctions::setLabel(QString label)
{
    m_label = label;
    emit labelChanged(label);
}

void LibraryFunctions::setProgress(qreal progress)
{
    m_progress = progress;
    emit progressChanged(progress);
}

void LibraryFunctions::scanFolder(QString path)
{
    QDirIterator dir_iter(path, QDirIterator::Subdirectories);

    QVector<QFileInfo> files;

    QStringList filter;
    addFilters(filter);

    // FileInfo is added to a vector so the user can see how far along on the import progress
    // the library is.

    setLabel("Analyzing Games");
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

    m_model.dbm.handle().transaction();
    QSqlQuery query(m_model.dbm.handle());

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

            m_model.updateQuery();


            setProgress((((i+1) / m_file_count) * 100.0));

        }

        if (query.exec())
            data_changed = true;

    }

    if (data_changed) {
        m_model.dbm.handle().commit();
        m_model.submit();
    }
}
