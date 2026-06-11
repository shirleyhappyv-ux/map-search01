#include <iostream>
#include <string>
#include <gdal/ogrsf_frmts.h>

void InitGDAL() {
    GDALAllRegister();
    OGRRegisterAll();
}

int main() {
    InitGDAL();

    std::string gpkg_path = "fixed_map_data.gpkg";
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(gpkg_path.c_str(), GDAL_OF_VECTOR | GDAL_OF_READONLY, NULL, NULL, NULL);
    if (poDS == NULL) {
        std::cerr << "错误：无法打开 GPKG 文件" << std::endl;
        return 1;
    }

    // ==========================================
    // 第一步：从建筑图层获取【鑫隆工业园】中心点
    // ==========================================
    OGRLayer* poBuildingLayer = poDS->GetLayerByName("gis_osm_buildings_a_free");
    if (poBuildingLayer == NULL) {
        std::cerr << "未找到建筑图层！" << std::endl;
        GDALClose(poDS);
        return 1;
    }

    OGRFeature* poBaseFeature = poBuildingLayer->GetFeature(3); 
    if (poBaseFeature == NULL) {
        std::cerr << "未找到 FID 为 3 的基础建筑！" << std::endl;
        GDALClose(poDS);
        return 1;
    }

    OGRGeometry* poGeometry = poBaseFeature->GetGeometryRef();
    OGRPoint centerPoint;
    if (poGeometry->getGeometryType() == wkbPolygon || poGeometry->getGeometryType() == wkbMultiPolygon) {
        static_cast<OGRSurface*>(poGeometry)->Centroid(&centerPoint);
    } else {
        centerPoint = *(static_cast<OGRPoint*>(poGeometry));
    }

    double centerX = centerPoint.getX();
    double centerY = centerPoint.getY();
    std::cout << ">>> 基准点【鑫隆工业园】坐标: " << centerX << ", " << centerY << std::endl;
    OGRFeature::DestroyFeature(poBaseFeature);

    // ==========================================
    // 第二步：构建 3 公里空间过滤器 (Bounding Box)
    // ==========================================
    double delta_lon = 0.035; 
    double delta_lat = 0.027;

    OGRLinearRing oRing;
    oRing.addPoint(centerX - delta_lon, centerY - delta_lat);
    oRing.addPoint(centerX + delta_lon, centerY - delta_lat);
    oRing.addPoint(centerX + delta_lon, centerY + delta_lat);
    oRing.addPoint(centerX - delta_lon, centerY + delta_lat);
    oRing.closeRings();

    OGRPolygon oSpatialFilterBox;
    oSpatialFilterBox.addRing(&oRing);

    // ==========================================
    // 第三步：切换至 POI 图层，应用复合过滤器
    // ==========================================
    // 注意：OSM 的点状设施一般在 pois 图层，如果是面状的大型医院可能在 landuse 图层
    // 这里我们先尝试标准点图层 "gis_osm_pois_free"
    std::string poi_layer_name = "gis_osm_pois_free"; 
    OGRLayer* poPoiLayer = poDS->GetLayerByName(poi_layer_name.c_str());
    
    if (poPoiLayer == NULL) {
        // 如果图层名不对，尝试打印系统中带 "pois" 或 "poi" 的图层以便排查
        std::cerr << "未找到图层: " << poi_layer_name << "，请确认你的20个图层中POI图层的真实名称。" << std::endl;
        GDALClose(poDS);
        return 1;
    }

    // 1. 施加空间过滤器
    poPoiLayer->SetSpatialFilter(&oSpatialFilterBox);

    // 2. 施加属性过滤器 (根据 OSM 标准，医院的 fclass 通常为 'hospital')
    std::string attr_filter = "fclass = 'hospital'";
    if (poPoiLayer->SetAttributeFilter(attr_filter.c_str()) != OGRERR_NONE) {
        std::cerr << "设置属性过滤器失败，请检查字段名是否包含 fclass。" << std::endl;
    }

    // ==========================================
    // 第四步：遍历并输出检索结果
    // ==========================================
    std::cout << "--- 开始检索【鑫隆工业园】周边 3 公里内的医院 ---" << std::endl;
    poPoiLayer->ResetReading();
    OGRFeature* poPoiFeature;
    int hospital_count = 0;

    while ((poPoiFeature = poPoiLayer->GetNextFeature()) != NULL) {
        hospital_count++;
        GIntBig poi_id = poPoiFeature->GetFID();
        const char* name = poPoiFeature->GetFieldAsString("name");
        const char* fclass = poPoiFeature->GetFieldAsString("fclass");

        std::cout << "医院 [" << hospital_count << "] -> FID: " << poi_id 
                  << " | 名称: " << (name && strlen(name) > 0 ? name : "未命名医疗机构") 
                  << " | 分类: " << (fclass ? fclass : "未知") 
                  << std::endl;

        OGRFeature::DestroyFeature(poPoiFeature);
    }

    std::cout << "--- 检索结束，共找到 " << hospital_count << " 家医院 ---" << std::endl;

    GDALClose(poDS);
    return 0;
}
