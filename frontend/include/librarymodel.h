#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QSqlTableModel>
#include <QVariant>
#include <QHash>
#include <QUrl>
#include <QMutex>
#include <QFileInfo>
#include <QThread>

#include "libraryinternaldatabase.h"
#include "libretro_cores_info_map.h"
#include "platforms.h"

namespace Library {

    class LibraryModel : public QSqlTableModel {
            Q_OBJECT
            Q_PROPERTY( int count READ count NOTIFY countChanged )
            Q_PROPERTY( bool recursiveScan READ recursiveScan WRITE setRecursiveScan
                        NOTIFY recursiveScanChanged )


            Q_PROPERTY( qreal progress READ progress NOTIFY progressChanged )

        public:

            // Simple data groupings to simplify signals and slots

            // GameMetaData is used to set in metadata for any game during;
            // this usually used after the append process.
            struct GameMetaData {
                QByteArray hash;
                QString artworkUrl;
                QString goodToolsCode;
                QString region;
            };

            // GameImportData is used to import game files into the SQL database.
            struct GameImportData {
                qreal importProgress;
                QString system;
                QString timePlayed;
                QString title;
                QString filePath;
            };

            using QSqlTableModel::setFilter;

            explicit LibraryModel( LibraryInternalDatabase &db, QObject *parent = 0 );

            ~LibraryModel();

            // Model Roles
            enum GameRoles {
                TitleRole = Qt::UserRole + 1,
                SystemRole,
                TimePlayedRole,
                ArtworkRole,
                FileNameRole,
                SystemPathRole,
            };

            // Getters
            bool select();
            bool cancelScan();

            //  QML Getters
            int count() const;
            qreal progress() const;
            bool recursiveScan() const;

            // QML Setters
            void setRecursiveScan( const bool scan );

            // Subclass Setters.
            QVariantMap get( int inx );
            QVariant data( const QModelIndex &index, int role ) const;

            // Subclass Getters
            QHash<int, QByteArray> roleNames() const;



        public slots:
            // Removes 1 row from the SQL model.
            bool remove( int row, int count = 1 );

            // Starts the import progress. The url is a folder or
            // file that the user wants to import.
            void append( const QUrl url );

            // clear removes every SQL row  in the database permanently.
            void clear();

            //  updateCount() reads the SQL database and updates the count()
            // function with how many entries are actually in the databaes.
            // This is important for creating our own scrollbar.
            void updateCount();

            // Filters the SQL model based on a SQL query.
            // This is used to filter games in the BoxartGrid
            void setFilter( QString filter
                            , QVariantList params
                            , bool preserveCurrentFilter );

            // Cancels the import progress if the mScanFilesThread is running.
            void cancel();

            void setMetadata();

        private slots:


            // handleFilesFound runs on the main QML thread, and is
            // where the SQL query statement is created and executed.
            void handleFilesFound( const GameImportData importData );

            // This function is connected to the mScanFilesThread.
            // This iterates through the mImportUrl, if the url is
            // a folder, and emits the filesFound signal.

            // findFiles is needed to run directly on the mScanFilesThread
            // so make sure that this signal is connected with a Qt::DirectionConnection;
            // this is because the LibraryModel is not moved into the mScanFilesThread,
            // nor should it be.
            void findFiles();

        signals:
            void countChanged();
            void recursiveScanChanged();
            void progressChanged();
            void fileFound( const GameImportData importData );
            void cancelScanChanged();

        private:
            // Normal Variables
            QStringList mFileFilter;
            QHash<int, QByteArray> mRoleNames;
            QVariantList params;
            QMutex scanMutex;

            // This thread is started when a user wants to import
            // a games folder. Currently, the thread quits whenever the
            // user cancels and import, or the import finishes.
            QThread mScanFilesThread;

            // mImportUrl is the url of the folder or file that is
            // being scanned and imported.
            QUrl mImportUrl;
            bool mCancelScan;

            // QML Variables
            int qmlCount;
            bool qmlRecursiveScan;
            qreal qmlProgress;

            // QML Setters
            void setProgress( const qreal progress );

            // Normal Setters
            void setCancelScan( const bool scan );

            // Helper Functions
            QByteArray getCheckSum( const QString &absoluteFilePath );
            void checkHeaderOffsets( const QFileInfo &fileInfo, QString &platform );
            bool getCueFileInfo( QFileInfo &fileInfo );

            QString getPlatformFromHeader( const QByteArray &rawFileData, const QString &suffix );

    };


}

#endif // LIBRARYMODEL_H
