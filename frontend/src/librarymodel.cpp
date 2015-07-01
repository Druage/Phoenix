#include "librarymodel.h"
#include <QSqlRecord>
#include <QSqlField>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlResult>
#include <QVariant>

using namespace Library;

LibraryModel::LibraryModel( LibraryInternalDatabase &db, QObject *parent )
    : QSqlTableModel( parent, db.database() ),
      qmlCount( 0 ) {

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
    if( this->filter() == filter ) {
        return;
    }


    if( preserveCurrentFilter && !this->filter().isEmpty() ) {
        filter = this->filter() + " AND " + filter;
        this->params.append( params );
    }

    else {
        this->params = params;
    }

    qDebug() << filter;

    QSqlTableModel::setFilter( filter );
}

int LibraryModel::count() const {
    return qmlCount;
}

QVariantMap LibraryModel::get( int inx ) {
    QVariantMap map;

    foreach( int i, roleNames().keys() ) {
        map[roleNames().value( i )] = data( index( inx, 0 ), i );
    }

    return map;
}


bool LibraryModel::remove( int row, int count ) {
    Q_UNUSED( count )
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

bool LibraryModel::append( QVariantMap dict ) {
    //beginInsertRows( QModelIndex(), count(), count() );

    database().transaction();
    QSqlQuery query( database() );


    QString statement = "INSERT INTO " + LibraryInternalDatabase::tableName + " (romFileName) VALUES (?)";

    query.prepare( statement );

    qDebug() << "dict value: " << dict.value( "romFileName" ).toString();
    query.addBindValue( dict.value( "romFileName" ) );


    if( !query.exec() ) {
        qDebug() << query.lastError().text();
        return false;
    }


    if( submitAll() ) {
        //database().commit();
    }

    else {
        database().rollback();
    }


    //endInsertRows();

    updateCount();
    return true;
}

void LibraryModel::clear() {

    database().transaction();

    QSqlQuery query( database() );

    if( !query.exec( "DELETE FROM " + LibraryInternalDatabase::tableName ) ) {
        qDebug() << "SQLITE Deletion Error: " << query.lastError().text();
        return;
    }

    if( submitAll() ) {
        //database().commit();
    }

    else {
        database().rollback();
    }


    updateCount();
}
