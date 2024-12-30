/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2024
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "Data.hpp"

QVector<ListItemData> Data::getListItems() {
    QVector<ListItemData> items;
    QSqlQuery index(db);
    int rowCount = 0;
    index.prepare("SELECT COUNT(*) FROM Level");

    if (!index.exec()) {
        qDebug() << "Error executing query:" << index.lastError().text();
        return items;
    }

    // Move to the first (and only) result row
    if (index.next()) {
        // Retrieve the count value (assuming it's in the first column, index 0)
        rowCount = index.value(0).toInt();

        // Now, 'rowCount' contains the count of rows in the 'Level' table
        qDebug() << "Number of rows in 'Level' table:" << rowCount;
    } else {
        // Handle the case where no rows are returned
        qDebug() << "No rows returned from the query";
    }
    for (int i=0; i < rowCount; i++) {
        QSqlQuery query(db);
        query.prepare("SELECT Level.*, Info.*, Picture.*, Author.* "
                      "FROM Level "
                      "JOIN Info ON Level.infoID = Info.InfoID "
                      "JOIN Screens ON Level.LevelID = Screens.levelID "
                      "JOIN Picture ON Screens.pictureID = Picture.PictureID "
                      "JOIN AuthorList ON Level.LevelID = AuthorList.levelID "
                      "JOIN Author ON AuthorList.authorID = Author.AuthorID "
                      "WHERE Level.LevelID = :id "
                      "GROUP BY Level.LevelID "
                      "ORDER BY MIN(Picture.PictureID) ASC");
        query.bindValue(":id", i+1);  // Set the ID autoincrament starts at 1

        if (query.exec()) {
            while (query.next()) {
                items.append(ListItemData(
                    query.value("Info.title").toString(),
                    query.value("Author.value").toString(),
                    query.value("Info.type").toInt(),
                    query.value("Info.class").toInt(),
                    query.value("Info.release").toString(),
                    query.value("Info.difficulty").toInt(),
                    query.value("Info.duration").toInt(),
                    query.value("Picture.data").toByteArray()));
            }
        } else {
            qDebug() << "Error executing query:" << query.lastError().text();
        }
    }
    return items;
}

InfoData Data::getInfo(const int id) {
    QVector<QByteArray> imageList;
    QSqlQuery query(db);
    query.prepare("SELECT Level.body, Picture.data "
                  "FROM Level "
                  "JOIN Screens ON Level.LevelID = Screens.levelID "
                  "JOIN Picture ON Screens.pictureID = Picture.PictureID "
                  "WHERE Level.LevelID = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        if (query.next()) {
            QString body = query.value("body").toString();
            while (query.next()) {
                imageList.push_back(query.value("Picture.data").toByteArray());
            }
            return InfoData(body, imageList);
        }
    } else {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return InfoData();
}

QString Data::getWalkthrough(const int id) {
    QSqlQuery query(db);
    query.prepare("SELECT Level.walkthrough "
                  "FROM Level "
                  "WHERE Level.LevelID = :id");
    query.bindValue(":id", id);
    if (query.exec()) {
        if (query.next()) {
            QString body = query.value("Level.walkthrough").toString();
            return body;
        }
    } else {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return "";
}

int Data::getType(const int id) {
    QSqlQuery query(db);
    query.prepare("SELECT Info.type "
                  "FROM Level "
                  "JOIN Info ON Level.infoID = Info.InfoID "
                  "WHERE Level.LevelID = :id");
    query.bindValue(":id", id);
    if (query.exec()) {
        if (query.next()) {
            return query.value("Info.type").toInt();
        }
    } else {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return 0;
}

ZipData Data::getDownload(const int id) {
    QSqlQuery query(db);
    query.prepare("SELECT Zip.* "
            "FROM Level "
            "JOIN ZipList ON Level.LevelID = ZipList.levelID "
            "JOIN Zip ON ZipList.zipID = Zip.ZipID "
            "WHERE Level.LevelID = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        if (query.next()) {
            return ZipData(
                    query.value("Zip.name").toString(),
                    query.value("Zip.size").toFloat(),
                    query.value("Zip.md5sum").toString(),
                    query.value("Zip.url").toString(),
                    query.value("Zip.version").toInt(),
                    query.value("Zip.release").toString());
        }
    } else {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return ZipData();
}

void Data::setDownloadMd5(const int id, const QString& newMd5sum) {
    QSqlQuery query(db);
    bool status = false;
    status = query.prepare(
        "UPDATE Zip "
        "SET md5sum = :newMd5sum "
        "WHERE Zip.ZipID IN ("
        "    SELECT ZipList.zipID"
        "    FROM Level"
        "    JOIN ZipList ON Level.LevelID = ZipList.levelID"
        "    WHERE Level.LevelID = :id)");

    if (status) {
        query.bindValue(":newMd5sum", newMd5sum);
        query.bindValue(":id", id);

        if (!query.exec()) {
            qDebug() << "Error executing query:" << query.lastError().text();
        } else {
            qDebug() << "md5sum updated successfully.";
        }
    } else {
        qDebug() << "Error preparing query:" << query.lastError().text();
    }
}

QVector<FileList> Data::getFileList(const int id) {
    QVector<FileList> list;
    QSqlQuery query(db);
    if (!query.prepare("SELECT File.path, File.md5sum "
            "FROM File "
            "JOIN GameFileList ON File.FileID = GameFileList.fileID "
            "WHERE GameFileList.gameID = :id")) {
        qDebug() << "Error preparing query:" << query.lastError().text();
    }
    query.bindValue(":id", id);
    if (query.exec()) {
        while (query.next()) {
            list.append({
                query.value("path").toString(),
                query.value("md5sum").toString()});
        }
    } else {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return list;
}
