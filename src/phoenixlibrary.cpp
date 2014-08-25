#include "phoenixlibrary.h"

PhoenixLibrary::PhoenixLibrary()

{
    import_thread = new QThread(this);
    import_thread->setObjectName("phoenix-scraper");

    m_model = new GameLibraryModel();
    m_model->moveToThread(import_thread);

    scraper = nullptr;

    connect(import_thread, SIGNAL(started()), this, SLOT(scanFolder()));
    connect(this, SIGNAL(queryStaged()), m_model, SLOT(updateQuery()));
    connect(import_thread, SIGNAL(finished()), import_thread, SLOT(deleteLater()));
    //connect(this, SIGNAL(scanComplete()), m_model, SLOT(updateQuery()));
}

PhoenixLibrary::~PhoenixLibrary()
{
    m_model->deleteLater();

}
void PhoenixLibrary::startImport(bool start)
{
    if (start) {
        import_thread->start(QThread::NormalPriority);
    }
}

void PhoenixLibrary::addFilters(QStringList &filter_list)
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

QString PhoenixLibrary::getSystem(QString suffix)
{
    QString system;
    if (suffix == "nes")
        system = "Nintendo (NES)";
    else if (suffix == "sfc" || suffix == "smc")
        system = "Super Nintendo (SNES)";
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

void PhoenixLibrary::setLabel(QString label)
{
    m_label = label;
    emit labelChanged();
}

void PhoenixLibrary::setProgress(qreal progress)
{
    m_progress = progress;
    emit progressChanged();
}

void PhoenixLibrary::setFolderPath(QString path)
{
    m_folder_path = path;
    emit folderPathChanged(path);
}

void PhoenixLibrary::loadXml(QString file_path)
{

    QResource resource(file_path);
    QFile in_file(resource.absoluteFilePath());
    if (in_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QXmlStreamReader reader;
        reader.setDevice(&in_file);
        while (!reader.isEndDocument()) {
            reader.readNext();
            QString element = reader.name().toString();
            qCDebug(phxLibrary) << element;
            if (element == "name")

                qCDebug(phxLibrary) << reader.readElementText();
        }

        if (reader.hasError()) {
            qCDebug(phxLibrary) << reader.errorString();
        }
        in_file.close();
    }
    else
        qCDebug(phxLibrary) << file_path << " was not opened";
}

void PhoenixLibrary::scanFolder()
{
    QDirIterator dir_iter(m_folder_path, QDirIterator::Subdirectories);

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

    int m_file_count = files.size();
    qreal count = static_cast<qreal>(m_file_count);

    if (m_file_count == 0)
        return;

    QSqlDatabase database = m_model->manager().handle();

    database.transaction();

    QSqlQuery query;

    for (qreal i=0; i < count; ++i) {

        QFileInfo file_info = files.at(i);
        QString system = getSystem(file_info.suffix());

        qCDebug(phxLibrary) << file_info.baseName();


        if (system != "") {
            GameData game_data;

            scraper = new TheGamesDB();
            scraper->setData(&game_data);
            scraper->setGameName(file_info.baseName());
            scraper->setGamePlatform(system);
            scraper->start();

            query.prepare("INSERT INTO games (title, console, time_played, artwork)"
            " VALUES (?, ?, ?, ?)");


             QString system = getSystem(file_info.suffix());


             if (game_data.title != "")
                 query.bindValue(0, game_data.title);
             else
                 query.bindValue(0, file_info.baseName());
             if (game_data.front_boxart != "")
                 query.bindValue(3, game_data.front_boxart);
             else
                 query.bindValue(3, "qrc:/assets/No-Art.png");

             //qCDebug(phxLibrary) << game_data.front_boxart;

             query.bindValue(1, system);
             query.bindValue(2, "0h 0m 0s");

            /*
            QCryptographicHash sha1_gen(QCryptographicHash::Md5);
            QString xml_file =  QString((":/databases/%1.xml")).arg(system);
            QResource resource(xml_file);
            QFile in_file(file_info.absoluteFilePath());

            if (in_file.open(QIODevice::ReadOnly)) {
                QByteArray file_data = in_file.readAll();
                sha1_gen.addData(file_data.data(), file_data.length());
                QString md5;
                md5 = sha1_gen.result().toHex();

                if (md5 != "") {
                    QString console = QString(":/databases/%1.xml").arg(system);
                    //loadXml(console);
                }
                qCDebug(phxLibrary) << md5;

                in_file.close();
            }
            else
                qCDebug(phxLibrary) <<  "db not opened";
            */

            emit queryStaged();
            //setProgress((((i+1) / count) * 100.0));
            //qCDebug(phxLibrary) << m_progress;

        }

        query.exec();

    }

    database.commit();

    emit scanComplete();

}


//have query be all loaded up
