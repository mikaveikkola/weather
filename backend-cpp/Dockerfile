# Stage 1: Builder
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake git pkg-config \
    libssl-dev libbrotli-dev zlib1g-dev \
    libc-ares-dev uuid-dev libjsoncpp-dev \
    libpq-dev \
    libcurl4-openssl-dev libpugixml-dev \
    && rm -rf /var/lib/apt/lists/*

# Build libpqxx 7.9.2 from source (Ubuntu's 7.8.1 package has an ABI issue)
WORKDIR /tmp/libpqxx
RUN git clone --depth 1 --branch 7.9.2 \
    https://github.com/jtv/libpqxx.git . \
    && cmake -B build \
       -DCMAKE_BUILD_TYPE=Release \
       -DBUILD_TEST=OFF \
    && cmake --build build -j$(nproc) \
    && cmake --install build

# Build Drogon v1.9.6 from source
WORKDIR /tmp/drogon
RUN git clone --depth 1 --branch v1.9.6 \
    https://github.com/drogonframework/drogon.git . \
    && git submodule update --init

RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_CTL=OFF \
    -DBUILD_TESTING=OFF \
    && cmake --build build -j$(nproc) \
    && cmake --install build

# Build our application
WORKDIR /app
COPY CMakeLists.txt .
COPY src/ src/

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libssl3 libbrotli1 zlib1g libc-ares2 \
    libuuid1 libjsoncpp25 libpq5 \
    libcurl4 ca-certificates \
    && (apt-get install -y libpqxx-7.9 2>/dev/null || apt-get install -y libpqxx-dev) \
    && (apt-get install -y libpugixml1v5 2>/dev/null || apt-get install -y libpugixml-dev) \
    && rm -rf /var/lib/apt/lists/*

# Copy Drogon/Trantor shared libraries from builder
COPY --from=builder /usr/local/lib/libdrogon*.so* /usr/local/lib/
COPY --from=builder /usr/local/lib/libtrantor*.so* /usr/local/lib/
RUN ldconfig

# Copy our binary
COPY --from=builder /app/build/weather_backend /usr/local/bin/weather_backend

EXPOSE 8001

CMD ["weather_backend"]
