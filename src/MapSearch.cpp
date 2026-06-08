#include "../include/MapSearch.h"
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <iostream>

MapSearch::MapSearch(const std::string& path) : dbPath(path) {
    GDALAllRegister();
}

void MapSearch::printAllFields(const std::string& layerName) {
    GDALDataset *poDS = (GDALDataset*) GDALOpenEx(dbPath.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    
    // 增加检查 1：数据库是否打开成功
    if (poDS == nullptr) {
        std::cerr << "错误：无法打开数据文件: " << dbPath << std::endl;
        return;
    }

    OGRLayer *poLayer = poDS->GetLayerByName(layerName.c_str());
    
    // 增加检查 2：图层是否存在
    if (poLayer == nullptr) {
        std::cerr << "错误：无法找到图层: " << layerName << std::endl;
        GDALClose(poDS); // 记得在出错时也要关闭资源
        return;
    }

    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
    std::cout << "Layer Fields:" << std::endl;
    for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);
        std::cout << "- " << poFieldDefn->GetNameRef() << std::endl;
    }
    
    GDALClose(poDS);
}

nlohmann::json MapSearch::searchByName(const std::string& name) {
    nlohmann::json results = nlohmann::json::array();
    
    GDALDataset *poDS = (GDALDataset*) GDALOpenEx(dbPath.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == nullptr) return results;

    OGRLayer *poLayer = poDS->GetLayerByName("gis_osm_places_free");
    if (poLayer == nullptr) {
        GDALClose(poDS);
        return results;
    }

    // 这里我们使用刚刚确认的 "name" 字段
    std::string filter = "name LIKE '%" + name + "%'";
    poLayer->SetAttributeFilter(filter.c_str());

    OGRFeature *poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        nlohmann::json item;
        // 确保字段名与你刚才打印出来的一致
        item["name"] = poFeature->GetFieldAsString("name");
        item["fclass"] = poFeature->GetFieldAsString("fclass");
        
        OGRGeometry *poGeometry = poFeature->GetGeometryRef();
        if (poGeometry) {
            OGRPoint point;
            poGeometry->Centroid(&point);
            item["lat"] = point.getY();
            item["lon"] = point.getX();
        }
        
        results.push_back(item);
        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDS);
    return results;
}