#include "librarymodel.h"
#include "logging.h"

#include <QSqlRecord>
#include <QSqlField>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlResult>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QMutexLocker>
#include <QCryptographicHash>

using namespace Library;

LibraryModel::LibraryModel( LibraryInternalDatabase &db, QObject *parent )
    : QSqlTableModel( parent, db.database() ),
      mScanFilesThread( this ),
      mCancelScan( false ),
      qmlCount( 0 ),
      qmlRecursiveScan( true ),
      qmlProgress( 0.0 ) {

    for( auto &extension : platformMap.keys() ) {
        mFileFilter.append( "*." + extension );
    }

    mRoleNames = QSqlTableModel::roleNames();
    mRoleNames.insert( TitleRole, "title" );
    mRoleNames.insert( SystemRole, "system" );
    mRoleNames.insert( TimePlayedRole, "time_played" );
    mRoleNames.insert( ArtworkRole, "artwork" );
    mRoleNames.insert( FileNameRole, "filename" );
    mRoleNames.insert( SystemPathRole, "system_path" );

    setEditStrategy( QSqlTableModel::OnManualSubmit );
    setTable( LibraryInternalDatabase::tableName );
    select();

    connect( this, &LibraryModel::fileFound, this, &LibraryModel::handleFilesFound );
    connect( &mScanFilesThread, &QThread::started, this, &LibraryModel::findFiles, Qt::DirectConnection );
    connect( &mScanFilesThread, &QThread::finished, this, [ this ] {
        setCancelScan( false );
        mImportUrl.clear();
        setProgress( 0.0 );
    } );

}

LibraryModel::~LibraryModel() {
    if( mScanFilesThread.isRunning() ) {
        setCancelScan( true );
        mScanFilesThread.exit();
        mScanFilesThread.wait();
    }
}
QVariant LibraryModel::data( const QModelIndex &index, int role ) const {
    QVariant value = QSqlTableModel::data( index, role );

    if( role < Qt::UserRole ) {
        return value;
    } else {
        if( !mRoleNames.contains( role ) ) {
            return value;
        }

        // role name need to be the same as column name.
        int columnIdx = record().indexOf( mRoleNames.value( role ) );
        return QSqlTableModel::data( this->index( index.row(), columnIdx ), Qt::DisplayRole );
    }
}

QHash<int, QByteArray> LibraryModel::roleNames() const {
    return mRoleNames;
}

bool LibraryModel::select() {
    const QString query = selectStatement();

    if( query.isEmpty() ) {
        return false;
    }

    beginResetModel();

    //    d->clearCache();

    QSqlQuery qu( database() );
    qu.prepare( query );

    for( auto &val : params ) {
        qu.addBindValue( val );
    }

    qu.exec();

    setQuery( qu );

    if( !qu.isActive() || lastError().isValid() ) {
        setTable( tableName() ); // resets the record & index
        endResetModel();
        return false;
    }

    endResetModel();
    return true;
}

bool LibraryModel::cancelScan() {
    scanMutex.lock();
    auto cancel = mCancelScan;
    scanMutex.unlock();
    return cancel;
}

void LibraryModel::updateCount() {
    database().transaction();

    QSqlQuery query( database() );

    query.exec( "SELECT Count(*) FROM " + LibraryInternalDatabase::tableName );

    while( query.next() ) {
        qmlCount = query.value( 0 ).toInt();
    }

    emit countChanged();
}

void LibraryModel::setFilter( QString filter, QVariantList params, bool preserveCurrentFilter ) {
    Q_UNUSED( preserveCurrentFilter );

    /*
        if( preserveCurrentFilter && !this->filter().isEmpty() ) {
            filter = this->filter() + " AND " + filter;
            this->params.append( params );
        }*/

    //else {
    this->params = params;
    //}

    QSqlTableModel::setFilter( filter );
}

void LibraryModel::cancel() {
    if( !mScanFilesThread.isRunning() ) {
        return;
    }

    setCancelScan( true );

}

void LibraryModel::setMetadata() {
    database().transaction();
}

void LibraryModel::handleFilesFound( const GameImportData importData ) {

    static const QString statement = "INSERT INTO " + LibraryInternalDatabase::tableName
                                     + " (title, system, filename, time_played) " + "VALUES (?,?,?,?)";

    if( cancelScan() ) {
        return;
    }

    static int count = 0;

    if( count == 0 ) {
        //beginInsertRows( QModelIndex(), count(), count() );

        database().transaction();
        count = 1;
    }

    QSqlQuery query( database() );

    query.prepare( statement );
    query.addBindValue( importData.title );
    query.addBindValue( importData.system );
    query.addBindValue( importData.filePath );
    query.addBindValue( importData.timePlayed );

    if( !query.exec() ) {
        qDebug() << query.lastError().text();
    }

    // Limit how many times the progress is updated, to reduce strain on the render thread.
    auto importProgress = static_cast<int>( importData.importProgress );

    if( importProgress  != static_cast<int>( progress() ) ) {
        setProgress( importProgress );
    }

    if( static_cast<int>( progress() ) == 100 ) {

        if( submitAll() ) {
            //database().commit();
        }

        else {
            database().rollback();
        }

        updateCount();
        //endInsertRows();

    }
}

QByteArray LibraryModel::getCheckSum( const QString &absoluteFilePath ) {
    QByteArray hash;
    QFile file( absoluteFilePath );

    if( file.open( QIODevice::ReadOnly ) ) {

        QCryptographicHash checkSum( QCryptographicHash::Sha1 );
        checkSum.addData( &file );
        hash = checkSum.result();

        file.close();
    }

    return std::move( hash );
}

bool LibraryModel::getCueFileInfo( QFileInfo &fileInfo ) {
    QFile file( fileInfo.canonicalFilePath() );

    if( !file.open( QIODevice::ReadOnly ) ) {
        return false;
    }

    while( !file.atEnd() ) {
        auto line = file.readLine();
        QList<QByteArray> splitLine = line.split( ' ' );

        if( splitLine.first().toUpper() == QByteArrayLiteral( "FILE" ) ) {
            QString baseName;

            for( int i = 1; i < splitLine.size() - 1; ++i ) {

                auto bytes = splitLine.at( i );

                bytes = bytes.replace( QByteArrayLiteral( "\"" ), QByteArrayLiteral( "" ) );

                if( i == splitLine.size() - 2 ) {
                    baseName += bytes;
                } else {
                    baseName += bytes + ' ';
                }
            }

            if( baseName.isEmpty() ) {
                return false;
            }

            fileInfo.setFile( fileInfo.canonicalPath() + QDir::separator() + baseName );
            break;

        }
    }

    file.close();

    return true;
}

void LibraryModel::checkHeaderOffsets( const QFileInfo &fileInfo, QString &platform ) {
    QFile file( fileInfo.canonicalFilePath() );

    if( file.open( QIODevice::ReadOnly ) ) {

        auto headers = headerOffsets.value( fileInfo.suffix() );

        for( auto &header : headers ) {
            if( !file.seek( header.offset ) ) {
                continue;
            }

            auto bytes = file.read( header.length );

            platform = platformForHeaderString( bytes.simplified().toHex() );

            if( !platform.isEmpty() ) {
                break;
            }

        }

        file.close();
    }
}

void LibraryModel::findFiles() {
    auto localUrl = mImportUrl.toLocalFile();

    QDir urlDirectory( localUrl );

    if( !urlDirectory.exists() ) {
        qCWarning( phxLibrary ) << localUrl << " does not exist!";
        return;
    }

    auto fileInfoList = urlDirectory.entryInfoList( mFileFilter, QDir::Files, QDir::NoSort );

    if( fileInfoList.size() == 0 ) {
        qCWarning( phxLibrary ) << "No files were found";
    }

    int i = 0;

    for( auto &fileInfo : fileInfoList ) {

        if( cancelScan() ) {
            mScanFilesThread.exit();
            qCDebug( phxLibrary ) << "Canceled import";
            return;
        }

        auto extension = fileInfo.suffix();

        // QFileInfo::baseName() seems to split the absolutePath based on periods '.'
        // This causes issue with some game names that use periods.
        auto title =  fileInfo.absoluteFilePath().remove(
                          fileInfo.canonicalPath() ).remove( 0, 1 ).remove( "." + extension );
        auto absoluteFilePath = fileInfo.canonicalFilePath();

        // We need to check for .cue files specifically, since these files are text files.
        if( extension == "cue" ) {
            if( !getCueFileInfo( fileInfo ) ) {
                // Can't import a wonky cue file. Just skip it.
                // We should be telling the use that this cue file has an error.
                qCWarning( phxLibrary ) << fileInfo.canonicalFilePath()
                                        << " is isn't a valid cue file. Skipping...";
                continue;
            }
        }

        // ####################################################################
        //                             WARNING!!!
        // ####################################################################

        // Every call to fileInfo after this point isn't guaranteed to have the
        // same values.

        // Make copies of the fileInfo data before this point, unless you want the
        // cue file check to override them.

        auto system = platformMap.value( extension, "" );

        // System should only be empty on ambiguous files, such as ISO's and BINS.
        if( system.isEmpty() ) {

            checkHeaderOffsets( fileInfo, system );

            if( system.isEmpty() ) {
                qCWarning( phxLibrary ) << "The system is 'still' empty, for"
                                        << fileInfo.canonicalFilePath();

                qCWarning( phxLibrary ) << "this means we need to add"
                                        << "in better header and offset checking.";
            }
        }


        GameImportData importData;
        importData.timePlayed = "00:00";
        importData.title = title;
        importData.filePath = absoluteFilePath;
        importData.importProgress = ( ( i + 1 )
                                      / static_cast<qreal>( fileInfoList.size() ) ) * 100.0;
        importData.system = system;

        emit fileFound( std::move( importData ) );

        ++i;

    }

    mScanFilesThread.quit();

}

void LibraryModel::setProgress( const qreal progress ) {
    if( progress == qmlProgress ) {
        return;
    }

    qmlProgress = progress;
    emit progressChanged();
}

void LibraryModel::setCancelScan( const bool scan ) {
    scanMutex.lock();
    mCancelScan = scan;
    scanMutex.unlock();
    emit cancelScanChanged();
}

int LibraryModel::count() const {
    return qmlCount;
}

qreal LibraryModel::progress() const {
    return qmlProgress;
}

QVariantMap LibraryModel::get( int inx ) {
    QVariantMap map;

    foreach( int i, roleNames().keys() ) {
        map[roleNames().value( i )] = data( index( inx, 0 ), i );
    }

    return map;
}

bool LibraryModel::recursiveScan() const {
    return qmlRecursiveScan;
}

void LibraryModel::setRecursiveScan( const bool scan ) {
    qmlRecursiveScan = scan;
    emit recursiveScanChanged();
}


bool LibraryModel::remove( int row, int count ) {
    Q_UNUSED( count )

    if( mScanFilesThread.isRunning() ) {
        qCWarning( phxLibrary ) << "Cannot remove entries when scan is running.";
        return false;
    }

    //beginRemoveRows( QModelIndex(), row, row + count );

    database().transaction();
    QSqlQuery query( database() );
    query.prepare( "DELETE FROM " + LibraryInternalDatabase::tableName + " WHERE systemID = ?" );
    query.addBindValue( row );

    if( !query.exec() ) {
        qDebug() << query.lastError().text();
        return false;
    }

    // If you want to cache changed, don't commit this.
    //

    if( submitAll() ) {
        //database().commit();
    }

    else {
        database().rollback();
    }


    //endRemoveRows();

    updateCount();

    return true;

}

void LibraryModel::append( const QUrl url ) {

    if( mScanFilesThread.isRunning() ) {
        qDebug() << "Scan in already running. returning...";
        return;
    }

    mImportUrl = url;
    mScanFilesThread.start( QThread::HighPriority );

}

void LibraryModel::clear() {
    if( mScanFilesThread.isRunning() ) {
        qCWarning( phxLibrary ) << "Cannot remove entries when scan is running.";
        return;
    }

    database().transaction();

    QSqlQuery query( database() );

    if( !query.exec( "DELETE FROM " + LibraryInternalDatabase::tableName ) ) {
        qDebug() << "SQLITE Deletion Error: " << query.lastError().text();
        return;
    }

    if( submitAll() ) {
        database().commit();
    }

    else {
        database().rollback();
    }

    updateCount();
}
