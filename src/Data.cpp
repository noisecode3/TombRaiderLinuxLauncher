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

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Data.h"

QVector<ListItemData> Data::getListItems()
{
    QVector<ListItemData> items;
    QSqlQuery index(db);
    int rowCount = 0;
    index.prepare("SELECT COUNT(*) FROM Level");

    if (!index.exec())
    {
        qDebug() << "Error executing query:" << index.lastError().text();
        return items;
    }

    // Move to the first (and only) result row
    if (index.next())
    {
        // Retrieve the count value (assuming it's in the first column, index 0)
        rowCount = index.value(0).toInt();

        // Now, 'rowCount' contains the count of rows in the 'Level' table
        qDebug() << "Number of rows in 'Level' table:" << rowCount;
    }
    else
    {
        // Handle the case where no rows are returned
        qDebug() << "No rows returned from the query";
    }
    for (int i=0; i < rowCount; i++)
    {
        QSqlQuery query(db);
        query.prepare("SELECT Level.*, Info.*, Picture.* "
                      "FROM Level "
                      "JOIN Info ON Level.infoID = Info.InfoID "
                      "JOIN Screens ON Level.LevelID = Screens.levelID "
                      "JOIN Picture ON Screens.pictureID = Picture.PictureID "
                      "WHERE Level.LevelID = :id "
                      "GROUP BY Level.LevelID "
                      "ORDER BY MIN(Picture.PictureID) ASC");
        query.bindValue(":id", i+1);  // Set the ID autoincrament starts at 1

        if (query.exec())
        {
            while (query.next())
            {
                items.append(ListItemData(
                    query.value("Info.title").toString(),
                    query.value("Info.author").toString(),
                    query.value("Info.type").toInt(),
                    query.value("Info.class").toInt(),
                    query.value("Info.release").toString(),
                    query.value("Info.difficulty").toInt(),
                    query.value("Info.duration").toInt(),
                    query.value("Picture.data").toByteArray()));
            }
        }
        else
        {
            qDebug() << "Error executing query:" << query.lastError().text();
        }
    }
    return items;
}

InfoData Data::getInfo(const int id)
{
    QVector<QByteArray> imageList;
    QSqlQuery query(db);
    query.prepare("SELECT Level.body, Picture.data "
                  "FROM Level "
                  "JOIN Screens ON Level.LevelID = Screens.levelID "
                  "JOIN Picture ON Screens.pictureID = Picture.PictureID "
                  "WHERE Level.LevelID = :id");
    query.bindValue(":id", id);

    if (query.exec())
    {
        if (query.next())
        {
            QString body = query.value("body").toString();
            while (query.next())
            {
                imageList.push_back(query.value("Picture.data").toByteArray());
            }
            return InfoData(body, imageList);
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return InfoData();
}

QString Data::getWalkthrough(const int id)
{
    QSqlQuery query(db);
    query.prepare("SELECT Level.walkthrough "
                  "FROM Level "
                  "WHERE Level.LevelID = :id");
    query.bindValue(":id", id);
    if (query.exec())
    {
        if (query.next())
        {
            QString body = query.value("Level.walkthrough").toString();
            return body;
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return "";
}

int Data::getType(const int id)
{
    QSqlQuery query(db);
    query.prepare("SELECT Info.type "
                  "FROM Level "
                  "JOIN Info ON Level.infoID = Info.InfoID "
                  "WHERE Level.LevelID = :id");
    query.bindValue(":id", id);
    if (query.exec())
    {
        if (query.next())
        {
            return query.value("Info.type").toInt();
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return 0;
}

ZipData Data::getDownload(const int id)
{
    QSqlQuery query(db);
    query.prepare("SELECT Zip.name, Zip.size, Zip.md5sum, Zip.url "
            "FROM Level "
            "JOIN Zip ON Level.zipID = Zip.ZipID "
            "WHERE Level.LevelID = :id");
    query.bindValue(":id", id);

    if (query.exec())
    {
        if (query.next())
        {
            return ZipData(
                    query.value("Zip.name").toString(),
                    query.value("Zip.size").toFloat(),
                    query.value("Zip.md5sum").toString(),
                    query.value("Zip.url").toString());
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return ZipData();
}

std::array<QVector<QString>, 2> Data::getFileList(const int id, bool trleList)
{
    std::array<QVector<QString>, 2> list;
    QSqlQuery query(db);
    if (trleList)
    {
        query.prepare("SELECT Files.path, Files.md5sum "
                "FROM Files "
                "JOIN LevelFileList ON Files.FileID = LevelFileList.fileID "
                "JOIN Level ON LevelFileList.levelID = Level.LevelID "
                "WHERE Level.LevelID = :id");
    }
    else
    {
        query.prepare("SELECT Files.path, Files.md5sum "
                "FROM Files "
                "JOIN GameFileList ON Files.FileID = GameFileList.fileID "
                "JOIN Game ON GameFileList.gameID = Game.GameID "
                "WHERE Game.GameID = :id");
    }
    query.bindValue(":id", id);
    if (query.exec())
    {
        while (query.next())
        {
            list[0] << query.value("Files.path").toString();
            list[1] << query.value("Files.md5sum").toString();
        }
    }
    else
    {
        qDebug() << "Error executing query:" << query.lastError().text();
    }
    return list;
}

