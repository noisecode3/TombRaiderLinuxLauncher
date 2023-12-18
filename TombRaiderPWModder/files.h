#pragma once
#include <string>
#include <vector>
#include <algorithm>
class file
{
public:
    file(const std::string path, const std::string md5sum)
        : path(path), md5sum(md5sum)  {}
    const std::string& getPath() const { return path; }
    const std::string& getMd5sum() const { return md5sum; }
private:
    const std::string path; //path + name
    const std::string md5sum;
};
class gameFileList
{
public:
    gameFileList(const std::string name) : name(name) {}
    const std::string getMd5sum(std::string& path)
    {
        auto it = std::find_if(list.begin(), list.end(),
            [&path](const file& obj) { return obj.getPath() == path; });
        if (it != list.end())
            return it->getMd5sum();
        return "";
    }
    size_t getIndex(std::string path)
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
    void addFile(const std::string path, const std::string md5sum)
    {
        list.push_back(file(path,md5sum));
    }
    file getFile(size_t index)
    {
        return list.at(index);
    }
private:
    const std::string name;
    std::vector<file> list;
};
