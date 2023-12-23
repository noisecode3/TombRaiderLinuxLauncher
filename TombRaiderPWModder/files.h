#pragma once
#include <QString>
#include <vector>
#include <algorithm>
class file
{
public:
    file(const QString md5sum, const QString path)
        :  md5sum(md5sum), path(path) {}
    const QString& getPath() const { return path; }
    const QString& getMd5sum() const { return md5sum; }
private:
    const QString md5sum;
    const QString path; //path + name
};

class gameFileList
{
public:
    gameFileList(const QString name) : name(name) {}
    const QString getMd5sum(QString& path)
    {
        auto it = std::find_if(list.begin(), list.end(),
            [&path](const file& obj) { return obj.getPath() == path; });
        if (it != list.end())
            return it->getMd5sum();
        return "";
    }
    size_t getIndex(QString path)
    {
        auto it = std::find_if(list.begin(), list.end(),
            [&path](const file& obj) { return obj.getPath() == path; });
        if (it != list.end())
            return std::distance(list.begin(), it);
        else
            return 0;
    }
    size_t getSize()
    {
        return list.size();
    }
    void addFile(const QString path, const QString md5sum)
    {
        list.push_back(file(path,md5sum));
    }
    file getFile(size_t index)
    {
        return list.at(index);
    }
private:
    const QString name;
    std::vector<file> list;
};
