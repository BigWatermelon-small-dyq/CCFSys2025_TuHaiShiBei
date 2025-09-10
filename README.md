# CCFSys2025_TuHaiShiBei
# 编译
readme.md同目录下 
```
cd OHMiner
cd build
cmake ..
make -j
```

# 测试运行
动态超图的正确性验证包括两方面，一是超边动态变化，二是超点动态变化。
超边动态变化
```
cd OHMiner
./build/bin/interactive_dynamic_system < ./testdata/generated/commands_all.txt
```

超点动态变化
```
cd OHMiner/testdata/vertex_operations && ../../build/bin/interactive_dynamic_system < clean_vertex_operations_commands.txt
```
