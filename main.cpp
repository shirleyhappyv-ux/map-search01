#include <iostream>
#include <string>
#include <vector>
#include <ogrsf_frmts.h> // GDAL OGR 核心头文件

// 初始化 GDAL/OGR 驱动
void InitGDAL() {
    GDALAllRegister();
    OGRRegisterAll();
}

int main() {
    InitGDAL();

    // 1. 设置本地 GPKG 文件路径
    std::string gpkg_path = "fixed_map_data.gpkg";

    // 2. 打开数据源 (ReadOnly 模式以提高性能)
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(gpkg_path.c_str(), GDAL_OF_VECTOR | GDAL_OF_READONLY, NULL, NULL, NULL);
    if (poDS == NULL) {
        std::cerr << "错误：无法打开 GPKG 文件: " << gpkg_path << std::endl;
        return 1;
    }

    std::cout << "成功打开地图，该文件包含图层数量: " << poDS->GetLayerCount() << std::endl;

    // 3. 定义检索条件（模拟产品需求）
    // 假设我们要检索 "road_layer" 图层中，在某个矩形范围内，且状态为拥堵(3)的道路
    std::string target_layer_name = "road_layer"; // 替换为你20个图层中的实际图层名
    
    OGRLayer* poLayer = poDS->GetLayerByName(target_layer_name.c_str());
    if (poLayer == NULL) {
        std::cerr << "未找到目标图层: " << target_layer_name << std::endl;
        GDALClose(poDS);
        return 1;
    }

    // 4. 【空间关联检索】配置 - 设定空间过滤器 (Bounding Box / Envelope)
    // 模拟一个经纬度或投影坐标范围 (MinX, MaxX, MinY, MaxY)
    OGRLinearRing oRing;
    oRing.addPoint(116.30, 39.90);
    oRing.addPoint(116.45, 39.90);
    oRing.addPoint(116.45, 40.05);
    oRing.addPoint(116.30, 40.05);
    oRing.closeRings();

    OGRPolygon oSpatialFilterBox;
    oSpatialFilterBox.addRing(&oRing);

    // 将空间几何体绑定到图层，GDAL 会自动利用 GPKG 的 R*Tree 索引进行硬件级加速
    poLayer->SetSpatialFilter(&oSpatialFilterBox);

    // 5. 【属性与状态检索】配置 - 设定 SQL 过滤条件
    // 假设属性字段为 status，我们要筛选 status = 'congested' 的数据
    std::string attribute_filter = "status = 'congested'";
    if (poLayer->SetAttributeFilter(attribute_filter.c_str()) != OGRERR_NONE) {
        std::cerr << "设置属性过滤器失败，请检查字段名。" << std::endl;
    }

    // 6. 执行复合检索并遍历结果
    std::cout << "\n--- 开始执行复合检索 ---" << std::endl;
    poLayer->ResetReading(); // 重置读取指针
    OGRFeature* poFeature;
    int match_count = 0;

    while ((poFeature = poLayer->GetNextFeature()) != NULL) {
        match_count++;
        
        // 获取要素 ID
        GIntBig feat_id = poFeature->GetFID();
        
        // 获取某个特定属性，例如“道路名称” (假设字段名叫 'name')
        // 注意：实际开发中需根据你的 gpkg 字段名调整
        const char* road_name = poFeature->GetFieldAsString("name");
        const char* current_status = poFeature->GetFieldAsString("status");

        std::cout << "匹配要素 ID: " << feat_id 
                  << " | 名称: " << (road_name ? road_name : "未知") 
                  << " | 状态: " << (current_status ? current_status : "未知") 
                  << std::endl;

        // 释放要素内存（防止内存泄漏）
        OGRFeature::DestroyFeature(poFeature);
    }

    std::cout << "--- 检索结束，共找到 " << match_count << " 个满足条件的要素 ---" << std::endl;

    // 7. 清理并关闭数据源
    GDALClose(poDS);
    return 0;
}
