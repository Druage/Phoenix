#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QSqlTableModel>
#include <QVariant>
#include <QHash>

#include "libraryinternaldatabase.h"

namespace Library {

    class LibraryModel : public QSqlTableModel {
            Q_OBJECT
            Q_PROPERTY( int count READ count NOTIFY countChanged )

        public:

            using QSqlTableModel::setFilter;

            explicit LibraryModel( LibraryInternalDatabase &db, QObject *parent = 0 );

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

            int count() const;

            QVariantMap get( int inx );

        public slots:
            bool remove( int row, int count = 1 );
            bool append( QVariantMap dict );
            void clear();
            void updateCount();
            void setFilter( QString filter
                            , QVariantList params
                            , bool preserveCurrentFilter );

        signals:
            void countChanged();

        private:

            QHash<int, QByteArray> mRoleNames;
            QVariantList params;

            // QML Variables
            int qmlCount;


    };

}

#endif // LIBRARYMODEL_H
