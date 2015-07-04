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
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QMutexLocker>
#include <QSharedPointer>
#include <QCryptographicHash>

using namespace Library;

LibraryModel::LibraryModel( LibraryInternalDatabase &db, QObject *parent )
    : QSqlTableModel( parent, db.database() ),
      fileFilter {
          "*.sfc", "*.smc", // Super Nintendo
          //"*.z64", "*.n64", // Nintendo 64;
          "*.nes", // Nintendo Entertainment System
          "*.gba",  // Game Boy Advance
          "*.gb", "*.gbc", "Game Boy / Color"
      },
      mFindFilesWatcher( nullptr ),
      qmlScanRunning( false ),
      mCancelScan( false ),
      qmlCount( 0 ),
      qmlRecursiveScan( true ),
      qmlProgress( 0.0 )
{

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

}

LibraryModel::~LibraryModel()
{
    qDebug() << "scan running: " << scanRunning();
    if ( scanRunning() ) {
        setCancelScan( true );
        mFindFilesWatcher->waitForFinished();
        if ( mFindFilesWatcher )
            delete mFindFilesWatcher;
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

bool LibraryModel::cancelScan()
{
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

void LibraryModel::cancel()
{
    if ( !scanRunning() )
        return;

    setCancelScan( true );

}

void LibraryModel::handleScanFinished()
{
    delete mFindFilesWatcher;
    mFindFilesWatcher = nullptr;
    setScanRunning( false );
    setCancelScan( false );
}

void LibraryModel::handleFilesFound( const GameImportData importData )
{

    static const QString statement = "INSERT INTO " + LibraryInternalDatabase::tableName
            + " (title, system, time_played) " + "VALUES (?,?,?)";


    if ( cancelScan() ) {
        return;
    }

    static int count = 0;
    if ( count == 0  ) {
        //beginInsertRows( QModelIndex(), count(), count() );

        database().transaction();
        count = 1;
    }

    QSqlQuery query( database() );

    query.prepare( statement );
    query.addBindValue( importData.title );
    query.addBindValue( importData.system );
    query.addBindValue( importData.timePlayed );

    if( !query.exec() ) {
        qDebug() << query.lastError().text();
    }

    setProgress( importData.importProgress );

    qDebug() << progress() << importData.title;


    if ( static_cast<int>( progress() ) == 100 ) {

        if( submitAll() ) {
            database().commit();
        }

        else {
            database().rollback();
        }

        updateCount();
        //endInsertRows();

    }
}

QByteArray LibraryModel::hash( const QFileInfo &fileInfo )
{
    QByteArray hash;
    QFile file( fileInfo.canonicalFilePath() );
    if ( file.open( QIODevice::ReadOnly ) ) {
        hash = QCryptographicHash::hash( file.readAll(), QCryptographicHash::Sha1 ).toHex();
        file.close();
    }
    return std::move( hash );
}

bool LibraryModel::findFiles( const QUrl &url )
{


    auto localUrl = url.toLocalFile();

    QDir urlDirectory( localUrl );

    if ( !urlDirectory.exists() ) {
        qCWarning( phxLibrary ) << localUrl << " does not exist!";
        return false;
    }

    auto fileInfoList = urlDirectory.entryInfoList( fileFilter, QDir::Files, QDir::NoSort );

    if ( fileInfoList.size() == 0 )
        qCWarning( phxLibrary ) << "No files were found";

    int i = 0;
    for ( auto &fileInfo : fileInfoList ) {

        if ( cancelScan() ) {
            qCDebug( phxLibrary ) << "Canceled import";
            return false;
        }

        auto extension = fileInfo.suffix();
        auto sha1 = hash( fileInfo );

        auto system = QString( "Super Nintendo" );

        GameImportData importData;
        importData.timePlayed = "00:00";
        importData.title = fileInfo.baseName();
        importData.filePath = fileInfo.canonicalFilePath();
        importData.importProgress =  ( ( i + 1 )
                                       / static_cast<qreal>( fileInfoList.size() ) ) * 100.0;
        importData.system = system;

        emit fileFound( std::move( importData ) );
        //qDebug() << sha1;
        Q_UNUSED( extension );

        ++i;

    }

    return true;

}

void LibraryModel::setScanRunning( const bool running )
{
    qmlScanRunning = running;
}

void LibraryModel::setProgress( const qreal progress )
{
    if ( progress == qmlProgress )
        return;
    qmlProgress = progress;
    emit progressChanged();
}

void LibraryModel::setCancelScan(const bool scan)
{
    scanMutex.lock();
    mCancelScan = scan;
    scanMutex.unlock();
    emit cancelScanChanged();
}

int LibraryModel::count() const {
    return qmlCount;
}

qreal LibraryModel::progress() const
{
    return qmlProgress;
}

bool LibraryModel::scanRunning() const
{
    return qmlScanRunning;
}

QVariantMap LibraryModel::get( int inx ) {
    QVariantMap map;

    foreach( int i, roleNames().keys() ) {
        map[roleNames().value( i )] = data( index( inx, 0 ), i );
    }

    return map;
}

bool LibraryModel::recursiveScan() const
{
    return qmlRecursiveScan;
}

void LibraryModel::setRecursiveScan(const bool scan)
{
    qmlRecursiveScan = scan;
    emit recursiveScanChanged();
}


bool LibraryModel::remove( int row, int count ) {
    Q_UNUSED( count )

    if ( scanRunning() ) {
        qCWarning( phxLibrary) << "Cannot remove entries when scan is running.";
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


    if ( scanRunning() ) {
        qDebug() << "Scan in already running. returning...";
        return;
    }

    if ( cancelScan() ) {
        handleScanFinished();
    }

    setScanRunning( true );
    QFuture<bool> future = QtConcurrent::run( this, &LibraryModel::findFiles, url );

    mFindFilesWatcher = new QFutureWatcher<bool>;

    connect( mFindFilesWatcher, &QFutureWatcher<QFileInfoList>::finished, this, &LibraryModel::handleScanFinished );
    connect( mFindFilesWatcher, &QFutureWatcher<QFileInfoList>::canceled, this, [ ] {
        qDebug() << "Should be quitting";
    } );

    mFindFilesWatcher->setFuture( future );

}

void LibraryModel::clear() {
    if ( scanRunning() ) {
        qCWarning( phxLibrary) << "Cannot remove entries when scan is running.";
        return;
    }

    database().transaction();

    QSqlQuery query( database() );

    if( !query.exec( "DELETE FROM " + LibraryInternalDatabase::tableName ) ) {
        qDebug() << "SQLITE Deletion Error: " << query.lastError().text();
        return;
    }

    qDebug() << "clear";
    if( submitAll() ) {
        database().commit();
    }

    else {
        database().rollback();
    }

    updateCount();
}
