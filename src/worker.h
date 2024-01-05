#ifndef FILES_H
#define FILES_H
#include "network.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include <QCoreApplication>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QStandardPaths>
#include <QDirIterator>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QCryptographicHash>
#include <QStringList>
#include <QDebug>
#include <QString>
class GameFileList
{
public:
    /**
     * @class GameFileList
     * @brief Game List of file names and md5sum
     * @param name Name of game release or game level
     * @details 2 Qt lists of file names and md5sum, that share index number. \name
     * When a file is added to the collection of lists. It makes sure the attributes share the index.
     */
    GameFileList() {}
    GameFileList(const QString name) : name(name) {}
    
    /**
     * @brief get md5sum based on path
     * @param path File name relative to game directory
     */
    const QString getMd5sum(QString& path)
    {
        int index = fileList.indexOf(path);
        if (index != -1)
            return md5List.at(index);
        return "";
    }
    /**
     * @brief get index form path 
     * @param path File name relative to game directory
     */
    size_t getIndex(QString& path)
    {
        return fileList.indexOf(path);
    }
    /**
     * @brief get size of the index
     */
    size_t getSize()
    {
        return fileList.size();
    }
    /**
     * @brief Adds file attributes to the list
     * @param md5sum Recorded md5sum from the file
     * @param path File name relative to game directory
     * @details Adds file name and md5sum to 2 QList only return true if both gets added
     */
    bool addFile(const QString md5sum, const QString path)
    {
        int pathSize = fileList.size();
        fileList.append(path);
        int md5sumSize  = md5List.size(); 
        md5List.append(md5sum);
        if(pathSize+1 == path.size())
            if(md5sumSize+1 == path.size())
                return true;
        return false;
    }
    /**
     * @brief Get filename
     * @param index a number from 0 to size
     * @details The filename and path from the current game directory. \n
     * For example /tomb.exe or /pix/screen1.jpg without the dot prefix
     */
    const QString& getPath(int index) const
    {
        if (index != -1)
            return fileList.at(index);
        return emptySpace;
    }
    /**
     * @brief Get md5sum
     * @details The md5sum is used for file integrity monitoring
     */
    const QString& getMd5sum(int index) const
    {
        if (index != -1)
            return md5List.at(index);
        return emptySpace;
    }
private:
    const QString name;
    QList<QString> fileList;
    QList<QString> md5List;
    const QString emptySpace = "";
};
/**
 * @struct FolderNames
 * @brief Folder names game used on windows
 * @details These names are used by steam and installed in the common folder on linux.
 */
struct FolderNames
{
    const QString TR1 = "/Tomb Raider (I)";
    const QString TR2 = "/Tomb Raider (II)";
    const QString TR3 = "/TombRaider (III)";
    const QString TR4 = "/Tomb Raider (IV) The Last Revelation";
    const QString TR5 = "/Tomb Raider (V) Chronicles";
};
struct LevelData
{
public:
    /**
     * @struct GameFileList
     * @brief Game List of file names and md5sum
     * @param name Name of game release or game level
     * @details 2 Qt lists of file names and md5sum, that share index number. \name
     * When a file is added to the collection of lists. It makes sure the attributes share the index.
     */
    LevelData(int id, const QString md5sum)
    : id(id), zipMd5sum(md5sum)  {}
    const int id;
    const QString zipMd5sum;
    /**
     * 
     */
    const QString& getZipMd5sum() const { return zipMd5sum; }
    /**
     * 
     */
    const int& getId() const { return id; }
    
    /**
     * 
     */
    const QString& getTitle() const { return title; }
    /**
     * 
     */
    void setTitle(const QString& s) { title = s; }

    /**
     * 
     */
    const QString& getAuthor() const { return author; }
    /**
     * 
     */
    void setAuthor(const QString& s) { author = s; }

    /**
     * 
     */
    const QString& getType() const { return type; }
    /**
     * 
     */
    void setType(const QString& s) { type = s; }

    /**
     * 
     */
    const QString& getLevelclass() const { return levelclass; }
    /**
     * 
     */
    void setLevelclass(const QString& s) { levelclass = s; }

    /**
     * 
     */
    const QString& getReleasedate() const { return releasedate; }
    /**
     * 
     */
    void setReleasedate(const QString& s) { releasedate = s; }

    /**
     * 
     */
    const QString& getDifficulty() const { return difficulty; }
    /**
     * 
     */
    void setDifficulty(const QString& s) { difficulty = s; }

    /**
     * 
     */
    const QString& getDuration() const { return duration; }
    /**
     * 
     */
    void setDuration(const QString& s) { duration = s; }

    /**
     * 
     */
    const QString& getScreensLarge() const { return screensLarge; }
    /**
     * 
     */
    void setScreensLarge(const QString& s) { screensLarge = s; }

    /**
     * 
     */
    const QString& getScreen() const { return screen; }
    /**
     * 
     */
    void setScreen(const QString& s) { screen = s; }

    /**
     * 
     */
    const QString& getBody() const { return body; }
    /**
     * 
     */
    void setBody(const QString& s) { body = s; }

    /**
     * 
     */
    const QString& getWalkthrough() const { return walkthrough; }
    /**
     * 
     */
    void setWalkthrough(const QString& s) { walkthrough = s; }

    /**
     * 
     */
    const float& getZipSize() const { return zipSize; }
    /**
     * 
     */
    void setZipSize(const float& s) { zipSize = s; }

    /**
     * 
     */
    const QString& getZipName() const { return zipName; }
    /**
     * 
     */
    void setZipName(const QString& s) { zipName = s; }

private:
    QString title;
    QString author;
    QString type; // like TR4
    QString levelclass;
    QString releasedate;
    QString difficulty;
    QString duration;
    QString screen;
    QString screensLarge; // separated by ,
    QString body; //html
    QString walkthrough; //html
    float zipSize;
    QString zipName;
};

/**
 *
 *
 */
/*
class pool
{
public:
    const QString levelUrl = "https://www.trle.net/sc/levelfeatures.php?lid={id}";
    const QString walkthroughUrl = "https://www.trle.net/sc/Levelwalk.php?lid={id}";
    const QString screensUrl = "https://www.trle.net/screens/{id}.jpg";
    const QString screensLargeUrl = "https://www.trle.net/screens/large/{id}[a-z].jpg";
    const QString downloadUrl = "https://www.trle.net/levels/levels/{year}/{id}/{zipName}";

    /**
     * 
     */
    //void setName(const QString newName) { name.push_back(newName); }
    /**
     * 
     */
    //void setFileList(const gameFileList& newFileList) { fileList.push_back(newFileList); }
    /**
     * 
     */
    //void setData(const leveldata& newData) { data.push_back(newData); }
    /**
     * 
     */
    //int getSize() const { return name.size(); }
    /**
     * 
     */
    //const gameFileList& getFileList(int index) const { return fileList[index]; }
    /**
     * 
     */
    //const gameFileList& getFileList(const QString& gameName) const
    /*
    {
        int index = name.indexOf(gameName);
        return fileList[index];
    }
    */
    /**
     * 
     */
    //const leveldata& getLevelData(int index) const { return data[index]; }
    /**
     * 
     */
    /*
    const leveldata& getLevelData(const QString& gameName) const
    {
        int index = name.indexOf(gameName);
        return data[index];
    }

private:
    QStringList name;
    QList<gameFileList> fileList;
    QList<leveldata> data;


};

*/

   /* 
    QCoreApplication a(argc, argv);

    // Add SQLite database driver
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    // Set the database name and file mode (ReadOnly)
    db.setDatabaseName("your_database_file.sqlite3");
    db.setConnectOptions("QSQLITE_OPEN_READONLY");

    // Open the database
    if (db.open()) {
        qDebug() << "Database opened in read-only mode.";

        // Perform read-only operations here

        // Close the database when done
        db.close();
        qDebug() << "Database closed.";
    } else {
        // Handle the case when the database cannot be opened
        qDebug() << "Failed to open the database.";
    }
    */
// WorkerThread class for hard drive and database operations
    // Create an instance of the worker thread
    // WorkerThread workerThread;

    // Start the worker thread
    // workerThread.start();

    // Wait for the worker thread to finish
    // workerThread.wait();

/**
 */
class WorkerThread : public QThread
{
    Q_OBJECT
    uint id;
    bool original;
    QString folderPath;
    QString directoryPath;
    GameFileList TR3FileList;
    GameFileList TRLE3573FileList;
    LevelData TRLE3573;
public:
    /**
     * 
     */
    WorkerThread(uint id, bool original, QString folderPath="", QString directoryPath="")
    : id(id), original(original), folderPath(folderPath), directoryPath(directoryPath),
      TR3FileList("TR3.Original"), TRLE3573(3573, "152d33e5c28d7db6458975a5e53a3122")
    {
        TR3FileList.addFile("61f1377c2123f1c971cbfbf9c7ce8faf", "DATA.TAG");
        TR3FileList.addFile("d09677d10864caa509ab2ffbd7ad37e7", "DEC130.DLL");
        TR3FileList.addFile("4ee5d4026f15c967ed3ae599885018b0", "WINPLAY.DLL");
        TR3FileList.addFile("af1d8d9435cb10fe2f4b4215eaf6bec4", "os.dat");
        TR3FileList.addFile("3561c0dffdb90248fa1fc2d4fb86f08a", "data.bin");
        TR3FileList.addFile("4044dc2c58f02bfea2572e80dd8f2abb", "tomb3.exe");
        TR3FileList.addFile("d0f7f7eda9e692eac06b32813a86e0c3", "layout.bin");
        TR3FileList.addFile("26b1f5d031c67a0b4a1832d1b4e6422c", "EDEC.DLL");
        TR3FileList.addFile("bd9397ee53c1dbe34d1b4fc168d8025e", "WINSTR.DLL");
        TR3FileList.addFile("f220b7353e550c8e017ab2d90fd83ae3", "WINSDEC.DLL");
        TR3FileList.addFile("90e64689804b4f4b0197c07290965a3c", "lang.dat");
        TR3FileList.addFile("0e4643dc86f0228969c0e0c1b30c0711", "audio/cdaudio.wad");
        TR3FileList.addFile("7d88aa20642a41e9d68558456683f57b", "pix/TITLEUK.BMP");
        TR3FileList.addFile("ffb8d2d1cc6a4723e0c58a2fe548f94f", "pix/SOUTHPAC.BMP");
        TR3FileList.addFile("2e5453045748420e576806917e0a30e3", "pix/CREDIT07.BMP");
        TR3FileList.addFile("5511970cf790c81a351e19baab1b1068", "pix/CREDIT03.BMP");
        TR3FileList.addFile("17e2230f562c04d2f0c0afabcad60629", "pix/CREDIT05.BMP");
        TR3FileList.addFile("88332670006eb121a705c839288c0e36", "pix/CREDIT04.BMP");
        TR3FileList.addFile("05608fdd5355624192a2b95eeaf21bc5", "pix/CREDIT01.BMP");
        TR3FileList.addFile("8682420d6bdefcb87820065dd87e50f0", "pix/ANTARC.BMP");
        TR3FileList.addFile("d942b38b5ce68d4a96b8b484e38f486f", "pix/THEEND.BMP");
        TR3FileList.addFile("deb5c81fc134fb2207f16df45bf97352", "pix/CREDIT08.BMP");
        TR3FileList.addFile("f1928fd6d00e1aefff2338f681c5a229", "pix/CREDIT09.BMP");
        TR3FileList.addFile("7456ab850b9444fa622250ca70aac6d5", "pix/HOUSE.BMP");
        TR3FileList.addFile("7fa680415bbabe15b155632a722b2ea0", "pix/CREDIT02.BMP");
        TR3FileList.addFile("92756534d2ac4dc9e118c906c7c16098", "pix/INDIA.BMP");
        TR3FileList.addFile("abb63ed2a790b5795082324b14a1fea7", "pix/CREDIT06.BMP");
        TR3FileList.addFile("b59fc9ad5f0bd64000c73bed4c2c6e21", "pix/LEGAL.BMP");
        TR3FileList.addFile("db7ae096fced4eaa630c0bec83114ee4", "pix/NEVADA.BMP");
        TR3FileList.addFile("67b5cab900f04c913f6bab241c4638ff", "pix/LONDON.BMP");
        TR3FileList.addFile("f56331cf11bf04dfef47492626cc9073", "pix/THEEND2.BMP");
        TR3FileList.addFile("9befdc5075fdb84450d2ed0533719b12", "data/JUNGLE.TR2");
        TR3FileList.addFile("69daad41e8a9ac9fad5aab6d22908de7", "data/CITY.TR2");
        TR3FileList.addFile("508ba45acbe4317e23daaf54ba919d04", "data/MAIN.SFX");
        TR3FileList.addFile("80f7907ded8a372bb87b1bcea178f94e", "data/ANTARC.TR2");
        TR3FileList.addFile("7a46c92685674a95024a9886152f8c2c", "data/TOWER.TR2");
        TR3FileList.addFile("8dc8bdc53dc53e1ec7943fac3b680a7c", "data/NEVADA.TR2");
        TR3FileList.addFile("28395720a88971b6dc590489ff47d9e3", "data/TITLE.TR2");
        TR3FileList.addFile("070d4a7b486c234d3e84ebaba904d48a", "data/TONYBOSS.TR2");
        TR3FileList.addFile("01e6f703493807dfd513d4fcd7f29ffd", "data/VICT.TR2");
        TR3FileList.addFile("3ae21d4e98daf1692a1eaf0acd9d6958", "data/TOMBPC.DAT");
        TR3FileList.addFile("18af2d4384904bf48c6941fb51382672", "data/TEMPLE.TR2");
        TR3FileList.addFile("ee80c9522dffc40aef5576de09ad5ded", "data/QUADCHAS.TR2");
        TR3FileList.addFile("59dc31d9020943e5ef85942df0a88c58", "data/AREA51.TR2");
        TR3FileList.addFile("438cd76e0e7be12464c3bef35d0216f5", "data/CHAMBER.TR2");
        TR3FileList.addFile("d2f6ef3fbd87a86f9c2d561765a19d89", "data/SEWER.TR2");
        TR3FileList.addFile("7b064a9d5b7cb17bd4e16261242bc940", "data/SHORE.TR2");
        TR3FileList.addFile("538f602ac876cee837a07760a3dbe3aa", "data/MINES.TR2");
        TR3FileList.addFile("9b3f54902d526472008408949f23032b", "data/ROOFS.TR2");
        TR3FileList.addFile("1630b3f25a226d51d7f4875300133e8e", "data/COMPOUND.TR2");
        TR3FileList.addFile("ab8b5f6f568432666aaf5c4d83b9f6f2", "data/CRASH.TR2");
        TR3FileList.addFile("ba54a5782912a4ef83929f687009377e", "data/OFFICE.TR2");
        TR3FileList.addFile("0275cb33c94e840859a622763865a2e9", "data/STPAUL.TR2");
        TR3FileList.addFile("c9c011b71964426ecd269c314ad5f4c1", "data/TRIBOSS.TR2");
        TR3FileList.addFile("f080de24577654474fa1ebd6d07673e2", "data/RAPIDS.TR2");
        TR3FileList.addFile("5e11d251ddb12b98ebead1883dc12d2a", "data/HOUSE.TR2");
        TR3FileList.addFile("ee2d0aa76754fe0744c47f4ad9fcd607", "fmv/Crsh_Eng.rpl");
        TR3FileList.addFile("6947a8a13b70235f9fc1aa3ea4db1da9", "fmv/Sail_Eng.rpl");
        TR3FileList.addFile("20be937db48b25b57bc88e9d47517905", "fmv/logo.rpl");
        TR3FileList.addFile("3afa8b9903af0c8dabee5f6f1eb396d0", "fmv/Intr_Eng.rpl");
        TR3FileList.addFile("24b90bdd3219facfdde240c954164a59", "fmv/Endgame.rpl");
        TR3FileList.addFile("e54ce1ac0106a76f72432db8e02c8dbf", "cuts/CUT1.TR2");
        TR3FileList.addFile("81ff9f99044738510cccacc3646fc347", "cuts/CUT2.TR2");
        TR3FileList.addFile("e93435fb9577ed5da27b8cb95e6a85f0", "cuts/CUT6.TR2");
        TR3FileList.addFile("a75d14b398ffbbca13bee7bc3ff0c080", "cuts/CUT11.TR2");
        TR3FileList.addFile("28180b6e049b439413cd657870bf8474", "cuts/CUT9.TR2");
        TR3FileList.addFile("4a061d14750d36c236ae4e2c22e75aa4", "cuts/CUT4.TR2");
        TR3FileList.addFile("19b04538646d2603308f37cea64d8e66", "cuts/CUT7.TR2");
        TR3FileList.addFile("0bfe24996a41984434de13470e359b05", "cuts/CUT12.TR2");
        TR3FileList.addFile("86290b1ac08dfb0d500357d9e861c072", "cuts/CUT5.TR2");
        TR3FileList.addFile("3135f022bceccc129c43997c2e53320c", "cuts/CUT3.TR2");
        TR3FileList.addFile("ab459301b03aab6c35327284cacbd0bd", "cuts/CUT8.TR2");
        TR3FileList.addFile("72890403f686d0a199cfcfed5296fed0", "support/info/Readme.txt");
        TR3FileList.addFile("746c9a7ab8cd2bd375f6520b5cf4ae3a", "support/info/ninjahead.gif");
        TR3FileList.addFile("3f463cc01e32b8de7542b1f9a2aa9e91", "support/info/support.htm");
        TR3FileList.addFile("bb72ad0b84f5d6cad35b6a3a7cb28ab9", "support/info/slegal.gif");
        TR3FileList.addFile("cf6df27145b94333dffa488e52a905ff", "support/info/eidos.gif");
        TR3FileList.addFile("cfaa679cd484b0709c36c8324dbf0960", "support/info/Ninja.jpg");
        TR3FileList.addFile("a78b2f1ec41c1443d04592714b9ab36c", "support/info/core.jpg");
        TR3FileList.addFile("a1030488e87c0edbdfc5b8ebeba53235", "support/info/tr3logo.jpg");


        TRLE3573FileList.addFile("f220b7353e550c8e017ab2d90fd83ae3", "/WINSDEC.DLL");
        TRLE3573FileList.addFile("8b1ad46066f4db6a035b70845671e518", "/tomb3.exe");
        TRLE3573FileList.addFile("8cae30f072ddb6abe23d4b349145428f", "/ReadmeHost.txt");
        TRLE3573FileList.addFile("31269ef2475300687fd75c33e39c2062", "/WINSTR.DLL");
        TRLE3573FileList.addFile("b8f5730d4c67260c0c1060f6c0fe88e8", "/tomb3_ConfigTool.exe");
        TRLE3573FileList.addFile("d09677d10864caa509ab2ffbd7ad37e7", "/DEC130.DLL");
        TRLE3573FileList.addFile("24be28857d11447aa580d39b9c1d601d", "/tomb3_ConfigTool.json");
        TRLE3573FileList.addFile("e1cd821e799f27721edea04bc246c762", "/WINPLAY.DLL");
        TRLE3573FileList.addFile("2400496dc550471ffde509b0f9e53aa7", "/ReadMe.pdf");
        TRLE3573FileList.addFile("26b1f5d031c67a0b4a1832d1b4e6422c", "/EDEC.DLL");
        TRLE3573FileList.addFile("3e5bcf084a9e69a24a6b462736b11722", "/Uninst.isu");
        TRLE3573FileList.addFile("c89cb2b4c028b8099bef44f4ebb3fb94", "/ExtraOptions/reset all options.reg");
        TRLE3573FileList.addFile("91ef536ce11ceca31a8c5662276b7ede", "/ExtraOptions/Lara's moveset/slide_to_run_ON.reg");
        TRLE3573FileList.addFile("bbd248cecd6b833cf3907de747e47cb7", "/ExtraOptions/Lara's moveset/crawl_tilt_OFF.reg");
        TRLE3573FileList.addFile("805e5aae04b982fb60a20ee3327ac0c4", "/ExtraOptions/Lara's moveset/duck_roll_ON.reg");
        TRLE3573FileList.addFile("d0e157876fdba4d04b29b2b8847e4c14", "/ExtraOptions/Lara's moveset/crawl_tilt_ON.reg");
        TRLE3573FileList.addFile("8a9a664f801f9da2731a15409a68a3be", "/ExtraOptions/Lara's moveset/flexible_sprint_OFF.reg");
        TRLE3573FileList.addFile("09864d3abd224a24775204577ed08eb0", "/ExtraOptions/Lara's moveset/flexible_crawl_OFF.reg");
        TRLE3573FileList.addFile("50b6c2d958bac000d6faac5b5b5e0c13", "/ExtraOptions/Lara's moveset/flexible_sprint_ON.reg");
        TRLE3573FileList.addFile("ace28f2de47360fd2a3a8727d8a1e5b9", "/ExtraOptions/Lara's moveset/flexible_crawl_ON.reg");
        TRLE3573FileList.addFile("8d28bd23f83d2fdfb49a711bafa327a8", "/ExtraOptions/Lara's moveset/duck_roll_OFF.reg");
        TRLE3573FileList.addFile("9679b35677e5953a8ac361766818b98b", "/ExtraOptions/Lara's moveset/slide_to_run_OFF.reg");
        TRLE3573FileList.addFile("d12c18d2b38f064f278b4393f4b89f1f", "/ExtraOptions/View/psx_fov_OFF.reg");
        TRLE3573FileList.addFile("7a04a5952acc2b96a01f3fca0a6f1c08", "/ExtraOptions/View/psx_fov_ON.reg");
        TRLE3573FileList.addFile("0fec096967e1eeb3547e9c9d091bc93d", "/ExtraOptions/GUI Style/bar_positions_Improved.reg");
        TRLE3573FileList.addFile("fe1e86b1a01f237be71cdf50f96bb165", "/ExtraOptions/GUI Style/Bars_PC.reg");
        TRLE3573FileList.addFile("3b7003f51c05a402482d0ecb98cafdad", "/ExtraOptions/GUI Style/ammo_counter_PC.reg");
        TRLE3573FileList.addFile("5075bbdb1fa1a7a40ed6e6eceb44985f", "/ExtraOptions/GUI Style/psx_boxes_ON.reg");
        TRLE3573FileList.addFile("5da364fe2a5f286171769a7d54abde42", "/ExtraOptions/GUI Style/psx_text_colors_ON.reg");
        TRLE3573FileList.addFile("a99c3655a1e056fc2e18d0610a0e3cc5", "/ExtraOptions/GUI Style/bar_positions_PC.reg");
        TRLE3573FileList.addFile("cfc266afd1c25bf2dccc5d880abbda2c", "/ExtraOptions/GUI Style/improved_poison_bar_OFF.reg");
        TRLE3573FileList.addFile("0393df69447132c9b741c73b598c0c2a", "/ExtraOptions/GUI Style/improved_poison_bar_ON.reg");
        TRLE3573FileList.addFile("443ac350e3e08af839843901b20b3a1d", "/ExtraOptions/GUI Style/PickupDisplay_ON.reg");
        TRLE3573FileList.addFile("20b106bdb7c4a05f07b8b7ff49e751ce", "/ExtraOptions/GUI Style/PickupDisplay_OFF.reg");
        TRLE3573FileList.addFile("57ea0818cca0a9dce7b99590d5e4fde9", "/ExtraOptions/GUI Style/Bars_PSX.reg");
        TRLE3573FileList.addFile("f42c4fd8ce5bbe150c1f6bbea95f44ba", "/ExtraOptions/GUI Style/psx_boxes_OFF.reg");
        TRLE3573FileList.addFile("c63a5b9da0343b68447a2e0454c309e4", "/ExtraOptions/GUI Style/Shadow_PSX.reg");
        TRLE3573FileList.addFile("5b01ad5d4be935c0777d784bcbc8d278", "/ExtraOptions/GUI Style/bar_positions_PSX.reg");
        TRLE3573FileList.addFile("f19eda36f5e97626b649b2df9cc8bad0", "/ExtraOptions/GUI Style/ammo_counter_PSX.reg");
        TRLE3573FileList.addFile("75a4856a052357b038ec43de4f01f62a", "/ExtraOptions/GUI Style/Shadow_PC.reg");
        TRLE3573FileList.addFile("cca4085fa0de17c54454c103ec04e742", "/ExtraOptions/GUI Style/psx_text_colors_OFF.reg");
        TRLE3573FileList.addFile("7001c46cf9fe3f751b36e544f65c8109", "/ExtraOptions/Sound/UnderwaterMusicMute_Full.reg");
        TRLE3573FileList.addFile("48ae12d793d8dd741b1dda57562612ae", "/ExtraOptions/Sound/UnderwaterMusicMute_Quiet.reg");
        TRLE3573FileList.addFile("fb8af6d2ced17d3c4b1312fdf2e08e6c", "/ExtraOptions/Sound/UnderwaterMusicMute_None.reg");
        TRLE3573FileList.addFile("b0d466d8ef363721e39a656811a33a71", "/ExtraOptions/Sound/InventoryMusicMute_Quiet.reg");
        TRLE3573FileList.addFile("0de4c1702d041cd557c633fe86e133e0", "/ExtraOptions/Sound/InventoryMusicMute_None.reg");
        TRLE3573FileList.addFile("015c00e0a706a767579b9744cc715b70", "/ExtraOptions/Sound/InventoryMusicMute_Full.reg");
        TRLE3573FileList.addFile("ca2e4c3d02727f65db5ab51be6623e7b", "/ExtraOptions/Effects/improved lasers_ON.reg");
        TRLE3573FileList.addFile("80145a877b983c60dda924ad1a4b297f", "/ExtraOptions/Effects/Sophia Rings_PSX.reg");
        TRLE3573FileList.addFile("afabd81bfd654176497901e74c6577b7", "/ExtraOptions/Effects/improved rain_ON.reg");
        TRLE3573FileList.addFile("d2a4536647f82eb9ce61a33cc01d01c5", "/ExtraOptions/Effects/upv_wake_ON.reg");
        TRLE3573FileList.addFile("56ebe5afd886d41bdb4e6214377e0335", "/ExtraOptions/Effects/kayak_mist_OFF.reg");
        TRLE3573FileList.addFile("5d41904671ea84e5d95ae20bd439771e", "/ExtraOptions/Effects/improved rain_OFF.reg");
        TRLE3573FileList.addFile("d2312710675ad5be54675082b43dfb1b", "/ExtraOptions/Effects/Footprints_ON.reg");
        TRLE3573FileList.addFile("f92154d0ca24c85e067c38066a598c1a", "/ExtraOptions/Effects/improved lasers_OFF.reg");
        TRLE3573FileList.addFile("152c54fec741230680413c6fd229d82d", "/ExtraOptions/Effects/kayak_mist_ON.reg");
        TRLE3573FileList.addFile("ef9799a0d6fe9f037a4f7639e9a3b7ef", "/ExtraOptions/Effects/Sophia Rings_PC.reg");
        TRLE3573FileList.addFile("87be25a06c21ddd16534d9a6663901c9", "/ExtraOptions/Effects/Footprints_OFF.reg");
        TRLE3573FileList.addFile("82d51412712f3a0ab7cb729ec9c2abdc", "/ExtraOptions/Effects/underwater dust_OFF.reg");
        TRLE3573FileList.addFile("8e152bb8c09e3bcb9a662fa1ff347444", "/ExtraOptions/Effects/Sophia Rings_ImprovedPC.reg");
        TRLE3573FileList.addFile("5723ebf2a2badc0429a2652d9d94f597", "/ExtraOptions/Effects/upv_wake_OFF.reg");
        TRLE3573FileList.addFile("e8e59ead6119eb2af3746b77991f2f08", "/ExtraOptions/Effects/underwater dust_ON.reg");
        TRLE3573FileList.addFile("4ecdb91641adf3c16f005bd802a9e333", "/ExtraOptions/misc/dozy_OFF.reg");
        TRLE3573FileList.addFile("fc59ee21c06fd42e57fd623537eade87", "/ExtraOptions/misc/psx_crystal_sfx_ON.reg");
        TRLE3573FileList.addFile("b0bf896fa48764a85bcbe98af4db08e7", "/ExtraOptions/misc/disable_colorkey_OFF.reg");
        TRLE3573FileList.addFile("e68ccd528be17fc34c489630f8920c76", "/ExtraOptions/misc/disable_gamma_OFF.reg");
        TRLE3573FileList.addFile("ca345258ca3950c953cfd7c47ea6556a", "/ExtraOptions/misc/disable_gamma_ON.reg");
        TRLE3573FileList.addFile("6f265bcfe94f7ac609e376cfccf47108", "/ExtraOptions/misc/psx_water_color_OFF.reg");
        TRLE3573FileList.addFile("fe253133c575a0039fc0c874949bc0fe", "/ExtraOptions/misc/psx_water_color_ON.reg");
        TRLE3573FileList.addFile("3d0bf00136697ba33fcf3d5385bbb92b", "/ExtraOptions/misc/psx_crystal_sfx_OFF.reg");
        TRLE3573FileList.addFile("7d105006d834d997bcfb9f4fb2558a1e", "/ExtraOptions/misc/dozy_ON.reg");
        TRLE3573FileList.addFile("2bd25a166b921bc5cf0c608f169927a9", "/ExtraOptions/misc/disable_colorkey_ON.reg");
        TRLE3573FileList.addFile("db3c5c1378f2b6da5619d8c8e7cff0ec", "/ExtraOptions/GUI scale/InvGuiScale_0_5.reg");
        TRLE3573FileList.addFile("6a3bee2eabab2b916066f63f6d3278df", "/ExtraOptions/GUI scale/GameGuiScale_0_7.reg");
        TRLE3573FileList.addFile("253688d9e4847fe0d0dff7682802ff00", "/ExtraOptions/GUI scale/InvGuiScale_2_0.reg");
        TRLE3573FileList.addFile("62f0cecfd157cc7b7f9dfcdf7d06d990", "/ExtraOptions/GUI scale/GameGuiScale_1_5.reg");
        TRLE3573FileList.addFile("e99c1a6ebef83f000237b3138ea060f3", "/ExtraOptions/GUI scale/InvGuiScale_0_7.reg");
        TRLE3573FileList.addFile("f24622eff6a816d423efb9776fbc0905", "/ExtraOptions/GUI scale/InvGuiScale_1_5.reg");
        TRLE3573FileList.addFile("add7deba55bc8c473b75c7b11a103082", "/ExtraOptions/GUI scale/InvGuiScale_1_0.reg");
        TRLE3573FileList.addFile("b68e98ef2fea09531fa4b8a807a04dad", "/ExtraOptions/GUI scale/GameGuiScale_0_5.reg");
        TRLE3573FileList.addFile("7b1985a180a99a984e553f8373f5e553", "/ExtraOptions/GUI scale/GameGuiScale_1_0.reg");
        TRLE3573FileList.addFile("eeae233feea92a37ff9a08fd2d3e714d", "/ExtraOptions/GUI scale/GameGuiScale_2_0.reg");
        TRLE3573FileList.addFile("d41d8cd98f00b204e9800998ecf8427e", "/saves/DoNotDeleteMe.txt");
        TRLE3573FileList.addFile("b6b9558acd88d27d178f5ddd93712539", "/pix/CREDIT08.bmp");
        TRLE3573FileList.addFile("19717be459462654588e8c04af835aa1", "/pix/CREDIT09.bmp");
        TRLE3573FileList.addFile("d4d4f7f732b22d5663083a95a2990a8f", "/pix/CREDIT03.bmp");
        TRLE3573FileList.addFile("3e890a03be2103e28bee9b477a614be4", "/pix/JUNGLE.bmp");
        TRLE3573FileList.addFile("871667781ffd7b5aa62ff5d8fe200d8b", "/pix/CREDIT02.bmp");
        TRLE3573FileList.addFile("7456ab850b9444fa622250ca70aac6d5", "/pix/HOUSE.BMP");
        TRLE3573FileList.addFile("7a326ead2573ef384a7ed66cfe811455", "/pix/CREDIT07.bmp");
        TRLE3573FileList.addFile("a96adc2e023c956585da49ca602cc736", "/pix/THEEND2.BMP");
        TRLE3573FileList.addFile("5e01244292efd5a3d0778bd8c6c8206d", "/pix/CULT.BMP");
        TRLE3573FileList.addFile("9b6ffaa5aedbadb963bd17cb5e4dddfd", "/pix/CREDIT04.bmp");
        TRLE3573FileList.addFile("15b6a39694385b99fd4af1408914216b", "/pix/Shivaville.bmp");
        TRLE3573FileList.addFile("fd75cb4a4dd6ac3f926c9d379fdf3e24", "/pix/THEEND.BMP");
        TRLE3573FileList.addFile("baed7ff30d55b1988e06b67530a7babe", "/pix/CREDIT01.BMP");
        TRLE3573FileList.addFile("92b252f2f7a15312933944edc7d73985", "/pix/TEMPLE.bmp");
        TRLE3573FileList.addFile("4a805c7a26e8dba8f77907ffda8670fe", "/pix/LEGAL.BMP");
        TRLE3573FileList.addFile("4ee7698bb0eb16e11e2a89b785ff6309", "/pix/TITLEUK.bmp");
        TRLE3573FileList.addFile("57383262f0911906bed674e7cc675467", "/pix/CREDIT06.bmp");
        TRLE3573FileList.addFile("deca815eb164646d9a1809dc28a193fb", "/pix/CREDIT05.bmp");
        TRLE3573FileList.addFile("4268b6a03d90684c6c683138105f00ad", "/audio/cdaudio.wad");
        TRLE3573FileList.addFile("d22b338dfd742b37bbd5db972f58fcec", "/data/HOUSE.tr2");
        TRLE3573FileList.addFile("0be8a7cbe3ae6343c08551517da79caf", "/data/Level3.tr2");
        TRLE3573FileList.addFile("8dabee4f0fd2721fd387a119114ef5a6", "/data/MAIN.SFX");
        TRLE3573FileList.addFile("8ed418451486f9528cf3432c85fb4280", "/data/tombpc.dat");
        TRLE3573FileList.addFile("6975ec26d2eb0f424d52357a9518a4de", "/data/Level1.tr2");
        TRLE3573FileList.addFile("28395720a88971b6dc590489ff47d9e3", "/data/TITLE.TR2");
        TRLE3573FileList.addFile("9daadb00c31009b332195f513e04c555", "/data/Level2.tr2");
        TRLE3573FileList.addFile("54476f906908bf70d73f9ec6d00b8cf4", "/data/Level4.tr2");
        TRLE3573FileList.addFile("90e439c3da36ff390c7aa51eea01a83a", "/data/Test.tr2");

        TRLE3573.setTitle("The Infada Cult");
        TRLE3573.setAuthor("Jonson");
        TRLE3573.setDifficulty("medium");
        TRLE3573.setDuration("long");
        TRLE3573.setLevelclass("Jungle");
        TRLE3573.setReleasedate("09-Nov-2023");
        TRLE3573.setType("TR3");
        TRLE3573.setScreen("");
        TRLE3573.setScreensLarge("");
        TRLE3573.setBody("Notes:\n13/11/2023 V1.2 Removed some statics from Level 1 to optimize stability. Texture Corrections on Level 3. Removed two misplaced statics in Level 3. ReadMe Corrections. No need to update, if you play with an older version.\n11/11/2023 V1.1 Only with some minor collision corrections on level 1. No need to update, if you play with an older version.\n\"There must be a few more of these!\" thinks Lara, sitting in her treasure chamber and looking at the Infada stone in her collection of Polynesian meteor artefacts. There are rumours of Shiva statues becoming alive. Also, a sinister cult founded by former RX Tech members, whose leader is able to shoot fireballs, is wreaking havoc in the small country of Shivania (fictional, in South Asia). To prevent such parties with bad intentions from misusing these additional artifacts, she decides to travel there and find them herself.\n\nImportant: Please do play this game with the executable in this package!\n\nHow to leave the boat? Stop it completely and press the duck roll key (default is the 'End' key) and left or right.");
        TRLE3573.setWalkthrough("some other day...");
        TRLE3573.setZipName("Jonson-TheInfadaCult.zip");
        TRLE3573.setZipSize(255.0);
    }
    /**
     * 
     */
    void run() override {
        // Perform hard drive operations

        if (original == true && id == 3)
        {
            packOriginalGame(3, folderPath, directoryPath);
        }
        else if (original == false && id == 3573)
        {
            FileDownloader downloader;
            QUrl url("https://www.trle.net/levels/levels/2023/3573/Jonson-TheInfadaCult.zip");
            QString localFilePath = "Jonson-TheInfadaCult.zip";
            downloader.downloadFile(url, localFilePath);
            //TRLE3573FileList
            //TRLE3573
        }
        /*

        qDebug() << "Hard drive operation in progress...";

        // Simulate hard drive operation
        sleep(2);

        // Perform database operations
        qDebug() << "Database operation in progress...";

        // Connect to the database
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("mydatabase.db");

        if (db.open()) {
            // Simulate database operation
            QSqlQuery query;
            query.exec("CREATE TABLE IF NOT EXISTS mytable (id INTEGER PRIMARY KEY, name TEXT)");
            query.exec("INSERT INTO mytable (name) VALUES ('John')");
            qDebug() << "Database operation completed.";
        } else {
            qDebug() << "Failed to open the database.";
        }
        */
    }
private:
    /**
     * 
     */
    void extractZip(const QString& zipFilename, const QString& extractPath)
    {
        QuaZip zip(zipFilename);
        if (!zip.open(QuaZip::mdUnzip))
        {
            qDebug() << "Error opening ZIP file" << Qt::endl;
            return;
        }

        QuaZipFileInfo info;
        QuaZipFile file(&zip);

        for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
        {
            if (zip.getCurrentFileInfo(&info) && file.open(QIODevice::ReadOnly))
            {
                QByteArray data = file.readAll();
                file.close();

                // Create directories if they don't exist
                QString filePath = extractPath + QDir::separator() + info.name;
                QString directory = filePath.left(filePath.lastIndexOf(QDir::separator()));

                QDir().mkpath(directory);

                // Create a new file on disk and write the data
                QFile outFile(filePath);
                if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered))
                {
                    outFile.write(data);
                    outFile.close();
                }
                else
                {
                    qDebug() << "Error opening file for writing: " << filePath << Qt::endl;
                }
            }
            else
            {
                qDebug() << "Error reading file info from ZIP archive" << Qt::endl;
            }
        }
        zip.close();
    }
    /**
     * 
     */
    const QString calculateMD5(const QString& filename)
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "Error opening file for reading: " << file.errorString();
            return "";
        }

        QCryptographicHash md5(QCryptographicHash::Md5);

        char buffer[1024];
        qint64 bytesRead;

        while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0)
        {
            md5.addData(buffer, static_cast<int>(bytesRead));
        }

        file.close();
        return QString(md5.result().toHex());
    }
    /**
     * 
     */
    void packOriginalGame(int game , const QString folderPath, const QString directoryPath )
    {
        const size_t s = TR3FileList.getSize();
    

        // Create the directory if it doesn't exist
        if (!QDir(directoryPath).exists())
        {
            if (QDir().mkpath(directoryPath))
            {
                qDebug() << "Directory created successfully.";
            }
            else
            {
                qDebug() << "Error creating directory.";
                return;
            }
        }
        else
        {
            qDebug() << "Directory already exists.";
        }

        for (size_t i = 0; i < s; i++)
        {
            const QString fMd5sum = TR3FileList.getMd5sum(i);
            const QString fPath = TR3FileList.getPath(i);
            const QString sourcePath = folderPath + "/" + fPath;
            const QString destinationPath = directoryPath + "/" + fPath;

            const QString  calculated = calculateMD5(sourcePath);
            if(fMd5sum == calculated)
            {
                // Ensure the destination directory exists
                const QFileInfo destinationFileInfo(destinationPath);
                QDir destinationDir(destinationFileInfo.absolutePath());
                if (!destinationDir.exists())
                {
                    if (!QDir().mkpath(destinationDir.absolutePath()))
                    {
                        qDebug() << "Error creating destination directory.";
                        return;
                    }
                }

                // Copy the file
                if (QFile::copy(sourcePath, destinationPath))
                {
                    qDebug() << "File copy successfully.";
                }
                else
                {
                    if(QFile::exists(destinationPath))
                    {
                        if(fMd5sum == calculateMD5(destinationPath))
                            qDebug() << "File exist";
                    }
                    else
                    {
                        qDebug() << "Failed to copy file " << fPath << Qt::endl;
                        return;
                    }
                }
            }
            else
            {
                qDebug() << "Original file was modified, had" << fMd5sum
                    << " got " << calculated << " for file " << fPath << Qt::endl;
                QDir directory(directoryPath);
                if (directory.exists())
                {
                    if (directory.removeRecursively())
                    {
                        qDebug() << "Working Directory removed successfully.";
                    }
                    else
                    {
                        qDebug() << "Error removing directory.";
                    }
                }
            else
            {
                qDebug() << "Directory does not exist.";
            }
                return;
            }
        }
        QDir directory(folderPath);

        if (directory.exists())
        {
            if (directory.removeRecursively())
            {
                qDebug() << "Copied Directory removed successfully.";
            }
            else
            {
                qDebug() << "Error removing directory.";
            }
        }
        else
        {
            qDebug() << "Directory does not exist.";
        }

        // Create a symbolic link
        if (QFile::link(directoryPath, folderPath))
        {
            qDebug() << "Symbolic link created successfully.";
        }
        else
        {
            qDebug() << "Failed to create symbolic link.";
        }

    }
};




#endif
