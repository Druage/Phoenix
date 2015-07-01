#ifndef LIBRARYINTERNALDATABASE_H
#define LIBRARYINTERNALDATABASE_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>

namespace Library {

    class LibraryInternalDatabase {

        public:

            static const QString tableVersion;
            static const QString databaseName;
            static const QString tableName;

            LibraryInternalDatabase();

            ~LibraryInternalDatabase();

            QSqlDatabase &database();

            void open();

            int version() const;

            QString filePath() const;

        private:
            QSqlDatabase db;

            bool createSchema();
            bool loadFixtures();

            QString mFilePath;
    };
}

#endif // LIBRARYINTERNALDATABASE_H
