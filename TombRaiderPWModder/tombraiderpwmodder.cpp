#include "tombraiderpwmodder.h"
#include "ui_tombraiderpwmodder.h"
#include <iostream>
#include <filesystem>
#include <dirent.h>

TombRaiderPWModder::TombRaiderPWModder(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TombRaiderPWModder)
{
    ui->setupUi(this);
    connect(ui->pushButtonLink, SIGNAL (clicked()), this, SLOT (linkClicked()));
    DIR *dpdf;
    struct dirent *epdf;
    char prefix[18] ="TombRaider (III).";

    //dpdf = opendir(ui->tableWidgetSetup->item(1, 0)->text().toStdString().data());
    dpdf = opendir("/home/marbangens/.local/share/TombRaider3ModderGames");
    if (dpdf != NULL)
    {
        while ( (epdf = readdir(dpdf)) )
        {
            
            bool keep = 1;
            for (int i=0; i<17;i++)
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

void TombRaiderPWModder::linkClicked()
{
    QList<QListWidgetItem *> list = ui->listWidgetModds->selectedItems();
    //std::cout << list.at(0)->text().toStdString() << "\n";
    std::filesystem::remove("/home/marbangens/.steam/root/steamapps/common/TombRaider (III)");

    std::filesystem::create_symlink("/home/marbangens/.local/share/TombRaider3ModderGames/" + list.at(0)->text().toStdString(), "/home/marbangens/.steam/root/steamapps/common/TombRaider (III)");
    QApplication::quit();
}
TombRaiderPWModder::~TombRaiderPWModder()
{
    delete ui;
}

