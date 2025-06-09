/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2025
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "../src/Data.hpp"

qint64 Data::getListRowCount() {
    QSqlQuery query(db);
    qint64 result = 0;

    if (query.prepare("SELECT COUNT(*) FROM Level") == true) {
        if (query.exec() == true) {
            // Move to the first (and only) result row
            if (query.next() == true) {
                // Assign the count value to result
                result = query.value(0).toInt();
                qWarning() << "Number of rows in 'Level' table:" << result;
            } else {
                qDebug() << "No rows returned from the query";
            }
        } else {
            qDebug() << "Error executing query:" << query.lastError().text();
        }
    } else {
        qDebug() << "Error preparing query:" << query.lastError().text();
    }
    return result;
}

QVector<ListItemData> Data::getListItems() {
    bool status = true;
    QSqlQuery query(db);
    QVector<ListItemData> items;

    status = query.prepare(
            "SELECT Info.trleID, "
            "Info.title, "
            "GROUP_CONCAT(Author.value, ', ') AS authors, "
            "Info.type, "
            "Info.class, "
            "Info.release, "
            "Info.difficulty, "
            "Info.duration "
            "FROM Level "
            "JOIN Info ON Level.infoID = Info.InfoID "
            "JOIN AuthorList ON Level.LevelID = AuthorList.levelID "
            "JOIN Author ON AuthorList.authorID = Author.AuthorID "
            "GROUP BY Info.trleID "
            "ORDER BY Info.release DESC");
    if (!status) {
        qDebug() << "Error preparing query getListItems:"
            << query.lastError().text();
    }

    if (status) {
        if (query.exec()) {
            while (query.next()) {
                items.append(ListItemData(
                    query.value("Info.trleID").toInt(),
                    query.value("Info.title").toString(),
                    query.value("authors").toString().split(", "),
                    query.value("Info.type").toInt(),
                    query.value("Info.class").toInt(),
                    query.value("Info.release").toString(),
                    query.value("Info.difficulty").toInt(),
                    query.value("Info.duration").toInt()));
            }
        } else {
            qDebug() << "Error executing query getListItems:"
                << query.lastError().text();
        }
    }

    return items;
}

void Data::getCoverPictures(QVector<ListItemData*>* items) {
    QSqlQuery query(db);

    bool status = query.prepare(
        "SELECT Picture.data "
        "FROM Level "
        "JOIN Info ON Level.infoID = Info.InfoID "
        "JOIN Screens ON Level.LevelID = Screens.levelID "
        "JOIN Picture ON Screens.pictureID = Picture.PictureID "
        "WHERE Info.trleID = :id AND Screens.position = 0 ");
    if (!status) {
        qDebug() << "Error preparing query getPictures:"
            << query.lastError().text();
    }

    if (status) {
        for (ListItemData* item : *items) {
            if (item->m_cover.isNull()) {
                query.bindValue(":id", item->m_trle_id);
                if (query.exec()) {
                    if (query.next() == true) {
                        item->addPicture(
                                query.value("Picture.data").toByteArray());
                    }
                } else {
                    qDebug() << "Error executing query getPictures:"
                        << query.lastError().text();
                }
            }
        }
    }
}

InfoData Data::getInfo(const int id) {
    QSqlQuery query(db);
    bool status = false;
    InfoData result;

    status = query.prepare(
        "SELECT Level.body, Picture.data "
        "FROM Level "
        "JOIN Info ON Level.infoID = Info.InfoID "
        "JOIN Screens ON Level.LevelID = Screens.levelID "
        "JOIN Picture ON Screens.pictureID = Picture.PictureID "
        "WHERE Info.trleID = :id AND Screens.position > 0 "
        "ORDER BY Screens.position ASC");
    query.bindValue(":id", id);

    if (status) {
        if (query.exec() == true) {
            if (query.next() == true) {
                QVector<QByteArray> imageList;
                QString body = query.value("body").toString();
                imageList.push_back(query.value("Picture.data").toByteArray());
                while (query.next() == true) {
                    imageList.push_back(
                        query.value("Picture.data").toByteArray());
                }
                result = InfoData(body, imageList);
            }
        } else {
            qDebug() << "Error executing query:" << query.lastError().text();
        }
    }
    return result;
}

QString Data::getWalkthrough(const int id) {
    QSqlQuery query(db);
    bool status = false;
    QString result = "";

    status = query.prepare(
        "SELECT Level.walkthrough "
        "FROM Level "
        "JOIN Info ON Level.infoID = Info.InfoID "
        "WHERE Info.trleID = :id");
    query.bindValue(":id", id);

    if (status) {
        if (query.exec() == true) {
            if (query.next() == true) {
                result = query.value("Level.walkthrough").toString();
            } else {
                qDebug() << "No results found for Level ID:" << id;
            }
        } else {
            qDebug() << "Error executing query:" << query.lastError().text();
        }
    }
    return result;
}

int Data::getType(const int id) {
    QSqlQuery query(db);
    bool status = false;
    int result = 0;

    status = query.prepare(
        "SELECT Info.type "
        "FROM Info "
        "WHERE Info.trleID = :id");
    query.bindValue(":id", id);

    if (status) {
        if (query.exec() == true) {
            if (query.next() == true) {
                result = query.value("Info.type").toInt();
            } else {
                qDebug() << "No results found for Level ID:" << id;
            }
        } else {
            qDebug() << "Error executing query:" << query.lastError().text();
        }
    }
    return result;
}

ZipData Data::getDownload(const int id) {
    QSqlQuery query(db);
    bool status = false;
    ZipData result;

    status = query.prepare(
        "SELECT Zip.*, Info.type "
        "FROM Level "
        "JOIN Info ON Level.infoID = Info.InfoID "
        "JOIN ZipList ON Level.LevelID = ZipList.levelID "
        "JOIN Zip ON ZipList.zipID = Zip.ZipID "
        "WHERE Info.trleID = :id");
    query.bindValue(":id", id);

    if (status) {
        if (query.exec() == true) {
            if (query.next() == true) {
                result = ZipData(
                    query.value("Zip.name").toString(),
                    query.value("Zip.size").toFloat(),
                    query.value("Zip.md5sum").toString(),
                    query.value("Zip.url").toString(),
                    query.value("Zip.version").toInt(),
                    query.value("Info.type").toInt(),
                    query.value("Zip.release").toString());
            } else {
                qDebug() << "No results found for Level ID:" << id;
            }
        } else {
            qDebug() << "Error executing query:" << query.lastError().text();
        }
    }
    return result;
}

void Data::setDownloadMd5(const int id, const QString& newMd5sum) {
    bool status = false;
    QSqlQuery query(db);

    status = query.prepare(
        "UPDATE Zip "
        "SET md5sum = :newMd5sum "
        "WHERE Zip.ZipID IN ("
        "    SELECT ZipList.zipID"
        "    FROM Level"
        "    JOIN Info ON Level.infoID = Info.InfoID"
        "    JOIN ZipList ON Level.LevelID = ZipList.levelID"
        "    WHERE Info.trleID = :id)");

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

QVector<File> Data::getFileList(const int id) {
    QSqlQuery query(db);
    QVector<File> list;

    if (!query.prepare(
            "SELECT File.path, File.md5sum "
            "FROM File "
            "JOIN GameFileList ON File.FileID = GameFileList.fileID "
            "WHERE GameFileList.gameID = :id")) {
        qDebug() << "Error preparing query:" << query.lastError().text();
    }
    query.bindValue(":id", id);

    if (query.exec() == true) {
        while (query.next() == true) {
            list.append({
                query.value("path").toString(),
                query.value("md5sum").toString()});
        }
    } else {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return list;
}
