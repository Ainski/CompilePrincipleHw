FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# ---------- 换用清华镜像源 ----------
RUN sed -i 's|http://archive.ubuntu.com|http://mirrors.tuna.tsinghua.edu.cn|g' /etc/apt/sources.list \
    && sed -i 's|http://security.ubuntu.com|http://mirrors.tuna.tsinghua.edu.cn|g' /etc/apt/sources.list

# ---------- 基础工具 + 构建依赖（此层很少变化，会被缓存）----------
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    flex \
    make \
    git \
    curl \
    zip \
    unzip \
    ca-certificates \
    mingw-w64 \
    pkg-config \
    mesa-common-dev \
    libglu1-mesa-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxi-dev \
    libxxf86vm-dev \
    && rm -rf /var/lib/apt/lists/*

# ---------- vcpkg + glfw3（此层很少变化，会被缓存）----------
ENV VCPKG_ROOT=/opt/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT} --depth 1 \
    && ${VCPKG_ROOT}/bootstrap-vcpkg.sh \
    && ${VCPKG_ROOT}/vcpkg install glfw3:x64-linux glfw3:x64-mingw-static \
    && rm -rf ${VCPKG_ROOT}/buildtrees ${VCPKG_ROOT}/downloads ${VCPKG_ROOT}/packages
ENV PATH="${VCPKG_ROOT}:${PATH}"

# ---------- 项目构建（此层经常变化）----------
WORKDIR /project
COPY . .

# flex 生成词法分析代码
RUN cd parser/src && flex -o lex.yy.cpp lexer.l

# 构建 Linux 版本
RUN cd /project/parser && mkdir -p build && cd build \
    && cmake .. && make -j$(nproc)

# 构建 Windows 版本
RUN cd /project/parser && mkdir -p build-win && cd build-win \
    && cmake .. \
       -DCMAKE_SYSTEM_NAME=Windows \
       -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
       -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
       -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
       -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
       -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32 \
       -DBUILD_WINDOWS=ON \
    && make -j$(nproc)

WORKDIR /project/parser

ENTRYPOINT ["./build/bin/parser"]
