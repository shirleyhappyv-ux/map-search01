#include <iostream>  // <--- 必须加上这一行！
#include <fstream>
#include "include/MapSearch.h"

int main() {
    std::string path = "../data/map_data.gpkg";
    std::cout << "尝试打开数据库: " << path << std::endl;
    
    MapSearch searcher(path);
    searcher.printAllFields("gis_osm_places_free");
    return 0;
}