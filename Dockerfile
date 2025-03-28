# 使用官方GCC镜像
FROM ubuntu:22.04

# 设置工作目录
WORKDIR /app

# 从项目根目录复制
COPY A3/ ./A3/


# 安装依赖并编译
RUN apt-get update && \
    apt-get install -y make && \
    make -C A3 all

# 设置默认运行命令
CMD ["/app/A3/DR/bin/DataReader"] 