#include <iostream>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>

int main() {
    // 1. 初始化 GDAL
    GDALAllRegister();

    // 2. 打开数据集
    GDALDataset *poDS = (GDALDataset*) GDALOpenEx("../data/map_data.gpkg", GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == nullptr) {
        std::cerr << "无法打开数据库文件！" << std::endl;
        return 1;
    }

    // 3. 遍历图层
    std::cout << "--- 正在查找可用图层 ---" << std::endl;
    int layerCount = poDS->GetLayerCount();
    for(int i = 0; i < layerCount; i++) {
        OGRLayer *layer = poDS->GetLayer(i);
        std::cout << "发现图层 [" << i << "]: " << layer->GetName() << std::endl;
    }

    // 4. 诊断：检查首个图层的第一个要素是否含有面积数据
    OGRLayer *firstLayer = poDS->GetLayer(0);
    OGRFeature *poFeature = firstLayer->GetNextFeature();
    if (poFeature != nullptr) {
        std::cout << "\n--- 诊断图层: " << firstLayer->GetName() << " ---" << std::endl;
        OGRGeometry *poGeom = poFeature->GetGeometryRef();
        if (poGeom) {
            std::cout << "几何类型代码: " << poGeom->getGeometryType() << std::endl;
            // 简单判断是否有点
            if (wkbFlatten(poGeom->getGeometryType()) == wkbPoint) {
                std::cout << "结论: 该图层是点数据，无法通过几何直接获取面积。" << std::endl;
            }
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDS);
    return 0;
}