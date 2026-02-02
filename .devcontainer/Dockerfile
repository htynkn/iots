FROM docker.1ms.run/espressif/idf:release-v5.4

# 1. 设置上海时区
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# 2. 优化 APT 源 (使用清华大学镜像)
RUN sed -i 's/archive.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list

# 3. 安装基础依赖
RUN apt-get update && apt-get install -y ca-certificates curl gnupg xz-utils sudo \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# 4. 创建与宿主机匹配的用户和组
ARG USERNAME=htynkn
ARG USER_UID=1000
ARG USER_GID=1000
RUN groupadd --non-unique --gid $USER_GID $USERNAME \
    && useradd --non-unique --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && usermod -aG dialout $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# 5. 【核心优化】使用淘宝/npmmirror 镜像源安装 Node.js 二进制包
ENV NODE_VERSION=v20.11.0
RUN curl -fsSL https://npmmirror.com/mirrors/node/$NODE_VERSION/node-$NODE_VERSION-linux-x64.tar.xz | tar -xJ -C /usr/local --strip-components=1

# 6. 设置 NPM 淘宝镜像并安装 Qwen CLI
USER $USERNAME
WORKDIR /home/$USERNAME
RUN npm config set registry https://registry.npmmirror.com \
    && npm install -g @qwen-code/qwen-code@latest

USER root
WORKDIR /workspace

RUN git config --global --add safe.directory /workspace && \
    git config --global user.name "Yunkun Huang" && \
    git config --global user.email "htynkn@gmail.com"

ENV IDF_MIRROR_BASE_URL=https://dl.espressif.cn/dl/

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

# 确保工作目录对 htynkn 用户可写
RUN chown -R $USERNAME:$USERNAME /workspace
