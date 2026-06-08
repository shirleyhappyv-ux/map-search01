#ifndef MAP_SEARCH_H
#define MAP_SEARCH_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class MapSearch {
public:
    MapSearch(const std::string& dbPath);
    nlohmann::json searchByName(const std::string& name);
    void printAllFields(const std::string& layerName);
   
private:
    std::string dbPath;
};

#endif