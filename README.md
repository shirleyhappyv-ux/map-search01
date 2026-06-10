/map-search-module
├── CMakeLists.txt        # 编译配置文件（关键！）
├── include/
│   └── MapSearch.h       # 对外暴露的接口定义
├── src/
│   └── MapSearch.cpp     # 核心搜索逻辑实现
└── main.cpp              # 用于本地测试的入口文件


6/10 
main.cpp              # 用于本地测试的入口文件
会首先自动打印出 map_data.gpkg 中 20个图层的完整结构（字段名、几何类型），紧接着演示了如何高效地进行空间范围圈选（Bounding Box 搜索）和属性关键字搜索。

CMakeLists.txt

运行代码
mkdir build && cd build
cmake ..
make
./map_search_demo

后续与 QT 端对接的工程建议
当你把 C++ 端的搜索逻辑剥离出来后，后期的 QT 端开发只需要做两件事：

树状图层渲染：利用刚才代码里提取的 allLayersMeta（图层名列表），直接驱动 QT 的 QTreeView，不需要传输底层几十万条的大数据。

搜索触发器：当用户在 QT 界面上用鼠标拉出一个矩形，或者在输入框敲入关键字时，QT 只需要将矩形坐标 (x1, y1, x2, y2) 和文本字符串作为参数传递给你的 C++ 搜索函数（即代码中的 SetSpatialFilterRect 和 SetAttributeFilter），C++ 检索出结果后再返回给 QT 渲染。
