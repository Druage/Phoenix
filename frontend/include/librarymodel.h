#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QSqlTableModel>
#include <QVariant>
#include <QHash>
#include <QUrl>
#include <QMutex>
#include <QFutureWatcher>
#include <QFileInfo>

#include "libraryinternaldatabase.h"

namespace Library {

    class LibraryModel : public QSqlTableModel {
            Q_OBJECT
            Q_PROPERTY( int count READ count NOTIFY countChanged )
            Q_PROPERTY( bool recursiveScan READ recursiveScan WRITE setRecursiveScan
                        NOTIFY recursiveScanChanged )


            Q_PROPERTY( qreal progress READ progress NOTIFY progressChanged )

        public:

            struct GameMetaData {
                QByteArray hash;
                QString artworkUrl;
                QString goodToolsCode;
                QString region;
            };

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

            enum GameRoles {
                TitleRole = Qt::UserRole + 1,
                SystemRole,
                TimePlayedRole,
                ArtworkRole,
                FileNameRole,
                SystemPathRole,
            };

            QVariant data( const QModelIndex &index, int role ) const;

            QHash<int, QByteArray> roleNames() const;

            bool select();
            bool cancelScan();

            int count() const;
            qreal progress() const;
            bool scanRunning() const;

            QVariantMap get( int inx );

            bool recursiveScan() const;

            void setRecursiveScan( const bool scan );

        public slots:
            bool remove( int row, int count = 1 );
            void append( const QUrl url );
            void clear();
            void updateCount();
            void setFilter( QString filter
                            , QVariantList params
                            , bool preserveCurrentFilter );
            void cancel();


        private slots:
            void handleScanFinished();
            //void getMetadata();
            void handleFilesFound( const GameImportData importData );

        signals:
            void countChanged();
            void recursiveScanChanged();
            void progressChanged();
            void fileFound( const GameImportData importData );
            void importThreadSynced();
            void cancelScanChanged();

        private:
            // Normal Variables
            QStringList fileFilter;
            QHash<int, QByteArray> mRoleNames;
            QVariantList params;
            QMutex scanMutex;
            QFutureWatcher<bool> *mFindFilesWatcher;
            bool qmlScanRunning;
            bool mCancelScan;

            // QML Variables
            int qmlCount;
            bool qmlRecursiveScan;
            qreal qmlProgress;

            bool findFiles( const QUrl &url );
            void setScanRunning( const bool running );
            void setProgress( const qreal progress );
            void setCancelScan( const bool scan );
            QByteArray hash( const QFileInfo &fileInfo );

    };


}

#endif // LIBRARYMODEL_H
