#include "libraryinternaldatabase.h"
#include "logging.h"

#include <QStandardPaths>
#include <QApplication>
#include <QDir>


using namespace Library;

const QString LibraryInternalDatabase::tableVersion = QStringLiteral( "schema_version" );
const QString LibraryInternalDatabase::databaseName = QStringLiteral( "gamelibrary.sqlite" );
const QString LibraryInternalDatabase::tableName = QStringLiteral( "games" );


LibraryInternalDatabase::LibraryInternalDatabase() {
    open();
}

LibraryInternalDatabase::~LibraryInternalDatabase() {
    db.close();
}

QSqlDatabase &LibraryInternalDatabase::database() {
    return db;
}

void LibraryInternalDatabase::open() {
    db = QSqlDatabase::addDatabase( "QSQLITE", "library" );

    QString dataPathStr = QStandardPaths::writableLocation( QStandardPaths::GenericDataLocation );
    Q_ASSERT( !dataPathStr.isEmpty() );

    QDir dataPath( dataPathStr );
    auto appname = QApplication::applicationName();

    if( !dataPath.exists( appname ) ) {
        dataPath.mkdir( appname ); // race...
    }

    Q_ASSERT( dataPath.cd( appname ) );

    mFilePath = dataPath.filePath( databaseName );

    db.setDatabaseName( mFilePath );
    qCDebug( phxLibrary, "Opening library database %s", qPrintable( db.databaseName() ) );

    if( !db.open() ) {
        qFatal( "Could not open database %s: %s", qPrintable( mFilePath ),
                qPrintable( db.lastError().driverText() ) );
    }


    if( !db.tables().contains( tableVersion ) ) {
        // must initialize db
        createSchema();
        loadFixtures();
    }
}

bool LibraryInternalDatabase::createSchema() {
    qCDebug( phxLibrary, "Initializing database schema" );
    db.transaction();
    QSqlQuery q( db );
    q.exec( "CREATE TABLE " + tableVersion + " (version INTEGER NOT NULL)" );
    q.exec( QStringLiteral( "INSERT INTO " ) + tableVersion + QStringLiteral( " (version) VALUES (0)" ) );
    q.exec( QStringLiteral( "CREATE TABLE " ) + tableName + QStringLiteral( " (\n" ) +
            QStringLiteral( "   id INTEGER PRIMARY KEY AUTOINCREMENT,\n" ) +

            QStringLiteral( "   /* game info */" ) +
            QStringLiteral( "   title TEXT NOT NULL,\n" ) +
            QStringLiteral( "   is_favorite BOOLEAN,\n" ) +
            QStringLiteral( "   system TEXT NOT NULL,\n" ) +
            QStringLiteral( "   region TEXT,\n" ) +
            QStringLiteral( "   goodtools_code TEXT,\n" ) +
            QStringLiteral( "   time_played DATETIME,\n" ) +
            QStringLiteral( "   artwork TEXT,\n" ) +

            QStringLiteral( "   /* file info */" ) +
            QStringLiteral( "   directory TEXT,\n" ) +
            QStringLiteral( "   filename TEXT UNIQUE,\n" ) +
            QStringLiteral( "   sha1 BLOB,\n" ) +
            QStringLiteral( "   crc32 BLOB,\n" ) +
            QStringLiteral( "   rom_count BOOLEAN\n" ) +
            QStringLiteral( ")" ) );

    q.exec( QStringLiteral( "CREATE INDEX title_index ON " ) + tableName + QStringLiteral( " (title)" ) );
    q.exec( QStringLiteral( "CREATE INDEX favorite_index ON " ) + tableName + QStringLiteral( " (is_favorite)" ) );
    db.commit();
    return true;
}

bool LibraryInternalDatabase::loadFixtures() {
    qCDebug( phxLibrary, "Loading fixtures" );
    db.transaction();
    QSqlQuery q( db );
    const QString command = QStringLiteral( "INSERT INTO " ) + tableName + QStringLiteral( " (title, system, time_played, artwork)" ) +
                            QStringLiteral( " VALUES (\"somegame %2\", \"test\", \"0h 0m 0s\", \"qrc:/assets/No-Art.png\")" );

    for( int i = 0; i < 10000; i++ ) {
        q.exec( command.arg( i ) );
    }

    db.commit();
    return true;
}

int LibraryInternalDatabase::version() const {
    QSqlQuery q( QStringLiteral( "SELECT version FROM " ) + tableVersion + QStringLiteral( " LIMIT 0,1" ), db );

    if( !q.exec() ) {
        qCDebug( phxLibrary, "Error while retreiving db version: %s",
                 qPrintable( q.lastError().text() ) );
        q.lastError();
    }

    return q.value( 0 ).toInt();
}

QString LibraryInternalDatabase::filePath() const {
    return mFilePath;
}
