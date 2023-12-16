#pragma once
#include <string>
/* 
 * trle.net level info data
 * The data needed for the offline part of the client
 * I don't think we should bother the server and its faster
 * It there will be to many pictures or over 10 games I can
 * move the pictures to some other download, but they are for
 * testing now. The id number will be used at end of every URL
 *
 */
class leveldata
{
private:
    const int id;
    const std::string zipMd5sum;
public:
    leveldata(int& id, std::string& md5sum)
    : id(id), zipMd5sum(md5sum)  {}
    const std::string& getZipMd5sum() const { return zipMd5sum; }
    const int& getId() const { return id; }
    
    const std::string& getTitle() const { return title; }
    void setTitle(const std::string& s) { title = s; }

    const std::string& getAuthor() const { return author; }
    void setAuthor(const std::string& s) { author = s; }

    const std::string& getType() const { return type; }
    void setType(const std::string& s) { type = s; }

    const std::string& getLevelclass() const { return levelclass; }
    void setLevelclass(const std::string& s) { levelclass = s; }

    const std::string& getReleasedate() const { return releasedate; }
    void setReleasedate(const std::string& s) { releasedate = s; }

    const std::string& getDifficulty() const { return difficulty; }
    void setDifficulty(const std::string& s) { difficulty = s; }

    const std::string& getDuration() const { return duration; }
    void setDuration(const std::string& s) { duration = s; }

    const std::string& getScreensLarge() const { return screensLarge; }
    void setScreensLarge(const std::string& s) { screensLarge = s; }

    const std::string& getBody() const { return body; }
    void setBody(const std::string& s) { body = s; }

    const std::string& getWalkthrough() const { return walkthrough; }
    void setWalkthrough(const std::string& s) { walkthrough = s; }

    const float& getZipSize() const { return zipSize; }
    void setZipSize(const float& s) { zipSize = s; }

    const std::string& getZipName() const { return zipName; }
    void setZipName(const std::string& s) { zipName = s; }

private:
    std::string title;
    std::string author;
    std::string type; // like TR4
    std::string levelclass;
    std::string releasedate;
    std::string difficulty;
    std::string duration;
    std::string screen;
    std::string screensLarge; // separated by ,
    std::string body; //html
    std::string walkthrough; //html
    float zipSize;
    std::string zipName;
};
/*
 *
 *
 */
class pool
{
    const std::string levelUrl = "https://www.trle.net/sc/levelfeatures.php?lid=";
    const std::string walkthroughUrl = "https://www.trle.net/sc/Levelwalk.php?lid=";
    const std::string screensUrl = "https://www.trle.net/screens/";
    const std::string screensLargeUrl = "https://www.trle.net/screens/large/";
};
