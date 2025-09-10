include_directories(../include)
set(HyperGraphSrc
        dataloader.cpp
        hypergraph.cpp
        common.cpp
        pattern.cpp
       )

add_library(HyperMining
            SHARED ${HyperGraphSrc})

# 链接 OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(HyperMining PUBLIC OpenMP::OpenMP_CXX)
endif()

# 禁用 AVX-512，避免在不支持的CPU上触发SIGILL
target_compile_options(HyperMining PRIVATE -mavx2 -mno-avx512f -mno-avx512vl -mno-avx512bw -mno-avx512dq)