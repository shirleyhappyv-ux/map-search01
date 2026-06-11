#include <iostream>
#include <string>
#include <vector>
#include "ogrsf_frmts.h" // GDAL/OGR 核心头文件

// 结构体：用于存储图层的元数据，方便后期构建搜索 UI
struct LayerMeta {
    std::string name;
    std::string geomType;
    int srid;
    std::vector<std::pair<std::string, std::string>> fields; // <字段名, 字段类型>
};

int main() {
    // 1. 初始化 GDAL/OGR 驱动
    GDALAllRegister();

    const char* gpkgPath = "/workspaces/map-search01/map_data.gpkg";

    // 2. 打开 GeoPackage 文件 (以只读模式)
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(gpkgPath, GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (poDS == nullptr) {
        std::cerr << "错误：无法打开地图文件 " << gpkgPath << "\n"
                  << "请确保该文件存在于当前运行目录下。" << std::endl;
        return 1;
    }

    int layerCount = poDS->GetLayerCount();
    std::cout << "========================================\n";
    std::cout << " 成功打开地图。检测到图层总数: " << layerCount << "\n";
    std::cout << "========================================\n\n";

    std::vector<LayerMeta> allLayersMeta;

    // 3. 方案优化：遍历并探查这 20 个图层的结构，而非盲目读前10条数据
    for (int i = 0; i < layerCount; ++i) {
        OGRLayer* poLayer = poDS->GetLayer(i);
        if (!poLayer) continue;

        LayerMeta meta;
        meta.name = poLayer->GetName();
        
        // 获取几何类型
        OGRwkbGeometryType geomType = poLayer->GetGeomType();
        meta.geomType = OGRGeometryTypeToName(geomType);

        // 获取空间参考 (EPSG/SRID)
        OGRSpatialReference* poSRS = poLayer->GetSpatialRef();
        meta.srid = poSRS ? atoi(poSRS->GetAttrValue("AUTHORITY", 1)) : 0;

        // 获取属性表字段结构
        OGRFeatureDefn* poDefn = poLayer->GetLayerDefn();
        for (int j = 0; j < poDefn->GetFieldCount(); ++j) {
            OGRFieldDefn* poFieldDefn = poDefn->GetFieldDefn(j);
            meta.fields.push_back({poFieldDefn->GetNameRef(), poFieldDefn->GetFieldTypeName(poFieldDefn->GetType())});
        }
        allLayersMeta.push_back(meta);

        // 打印图层结构报告
        std::cout << "【图层 " << i + 1 << "】: " << meta.name << "\n"
                  << "  -> 几何类型: " << meta.geomType << " (EPSG:" << meta.srid << ")\n"
                  << "  -> 属性字段: ";
        for (const auto& field : meta.fields) {
            std::cout << field.first << "(" << field.second << ") ";
        }
        std::cout << "\n----------------------------------------\n";
    }

    // 4. 搜索逻辑演练
    std::cout << "\n[执行搜索示例...] \n";
    if (layerCount > 0) {
        // 假设我们对第一个图层进行搜索
        OGRLayer* searchLayer = poDS->GetLayer(0);
        
        // ----------------------------------------------------
        // 场景 A: 空间范围搜索 (用户在界面上拉框圈选)
        // 设定一个经纬度矩形边界: MinX, MaxX, MinY, MaxY
        // ----------------------------------------------------
        std::cout << "-> 正在图层 [" << searchLayer->GetName() << "] 中执行空间矩形圈选...\n";
        searchLayer->SetSpatialFilterRect(116.0, 39.0, 117.0, 40.0); 

        // ----------------------------------------------------
        // 场景 B: 混合属性搜索 (在空间圈选的基础上，再过滤特定属性)
        // 假设该图层包含一个叫 "name" 或 "type" 的字段
        // ----------------------------------------------------
        // searchLayer->SetAttributeFilter("type = 'building' AND name LIKE '%学校%'");

        // 5. 获取搜索结果 (此时底层的 R-Tree 空间索引和 SQL 过滤器会高效协同工作)
        searchLayer->ResetReading();
        OGRFeature* poFeature;
        int matchCount = 0;

        while ((poFeature = searchLayer->GetNextFeature()) != nullptr && matchCount < 5) {
            matchCount++;
            std::cout << "   匹配到要素 ID: " << poFeature->GetFID();
            
            // 如果图层有字段，打印第一个字段的值看看
            if (poFeature->GetFieldCount() > 0) {
                std::cout << " | 第一个字段值: " << poFeature->GetFieldAsString(0);
            }
            std::cout << "\n";
            
            OGRFeature::DestroyFeature(poFeature);
        }
        std::cout << "   空间搜索完毕，展示前 " << matchCount << " 条命中结果。\n";
        
        // 清除过滤器，恢复图层状态
        searchLayer->SetSpatialFilter(nullptr);
        searchLayer->SetAttributeFilter(nullptr);
    }

    // 6. 释放资源
    GDALClose(poDS);
    return 0;
}
