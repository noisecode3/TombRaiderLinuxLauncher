#include "tombraiderpwmodder.h"
#include "ui_tombraiderpwmodder.h"
#include <iostream>
#include <filesystem>
#include <dirent.h>
#include <unistd.h>
#include <string>

TombRaiderPWModder::TombRaiderPWModder(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TombRaiderPWModder)
{
    ui->setupUi(this);
    connect(ui->pushButtonLink, SIGNAL (clicked()), this, SLOT (linkClicked()));
    DIR *dpdf;
    struct dirent *epdf;
    const int  prefixSize = 18;
    char prefix[prefixSize] ="TombRaider (III).";
    char path[256] = "/home/";
    char gamedir[256] = "/.local/share/TombRaider3ModderGames";
    char* readnext = getlogin();
    int i=6;
    readNextCstring(i, readnext, path);
    readnext = gamedir;
    readNextCstring(i, readnext, path);

    char game[256] ="TombRaider (III)";
    char path2[256] = "/home/";
    char steamdir[256] = "/.steam/root/steamapps/common/";
    readnext = getlogin();
    i=6;

    readNextCstring(i, readnext,path2);
    readnext = steamdir;
    readNextCstring(i, readnext,path2);
    readnext = game;
    readNextCstring(i, readnext,path2);

    std::filesystem::path p(path2);
    std::filesystem::exists(p) ?
        std::filesystem::is_symlink(p) ?
            std::cout << std::filesystem::read_symlink(p) << '\n' :
        std::cout << " exists but it is not a symlink\n" :
    std::cout << " does not exist\n";

    //dpdf = opendir(ui->tableWidgetSetup->item(1, 0)->text().toStdString().data());
    dpdf = opendir(path);
    if (dpdf != NULL)
    {
        while ( (epdf = readdir(dpdf)) )
        {
            
            bool keep = 1;
            for (int i=0; i<(prefixSize-1); i++)
            {
                if (epdf->d_name[i] != prefix[i] && epdf->d_name[i] == 0)
                {
                    keep = 0;
                    break;
                }
            }
            if(keep)
               ui->listWidgetModds->insertItem(0, QString(epdf->d_name));
        }
        closedir(dpdf);
    }

}

void TombRaiderPWModder::readNextCstring(int &index, char* cString, char* path)
{
    for (; *cString !=0; cString++)
    {
        path[index] = *cString;
        index+=1;
    }
}

void TombRaiderPWModder::linkClicked()
{
    QList<QListWidgetItem *> list = ui->listWidgetModds->selectedItems();
    //std::cout << list.at(0)->text().toStdString() << "\n";

    char path1[256] = "/home/";
    char gamedir[256] = "/.local/share/TombRaider3ModderGames/";
    char* readnext = getlogin();
    int i=6;
    readNextCstring(i, readnext,path1);
    readnext = gamedir;
    readNextCstring(i, readnext,path1);

    char game[256] ="TombRaider (III)";
    char path2[256] = "/home/";
    char steamdir[256] = "/.steam/root/steamapps/common/";
    readnext = getlogin();
    i=6;

    readNextCstring(i, readnext,path2);
    readnext = steamdir;
    readNextCstring(i, readnext,path2);
    readnext = game;
    readNextCstring(i, readnext,path2);

    //the symlink
    std::filesystem::remove(path2);

    std::filesystem::create_symlink(path1 + list.at(0)->text().toStdString(), path2);
    QApplication::quit();
}
TombRaiderPWModder::~TombRaiderPWModder()
{
    delete ui;
}

