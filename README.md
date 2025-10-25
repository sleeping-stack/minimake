# 项目结构

```
minimake/
├── CMakeLists.txt        # CMake 构建配置
├── README.md             # 项目总体介绍
├── 实现说明.md            # 实现细节与流程说明
├── bin/                  # 构建生成的可执行文件
├── build/                # CMake 产物与中间文件
├── include/              # 头文件，声明公共结构与接口
├── src/                  # 核心源码实现
├── test/                 # 测试辅助代码与示例
└── 笔记/                 # 学习笔记
```

## 如何构建（Build）

本项目使用 CMake 构建，生成的可执行文件位于 `bin/minimake`。

~~~zsh
cd build
cmake ..
make
~~~