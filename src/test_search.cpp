#include <iostream>
#include "../include/MapSearch.h"

int main() {
    // 假设 dbPath 是相对于 build 目录的路径
    MapSearch searcher("../data/map_data.gpkg");

    std::string testKey = "重庆"; // 先用英文测试，避免中文编码干扰
    std::cout << "正在搜索: " << testKey << " ..." << std::endl;

    nlohmann::json results = searcher.searchByName(testKey);

    if (results.empty()) {
        std::cout << "结果为空！请检查搜索关键词或数据是否包含该地名。" << std::endl;
    } else {
        std::cout << "搜索成功！找到 " << results.size() << " 个结果：" << std::endl;
        std::cout << results.dump(2) << std::endl;
    }

    return 0;
}