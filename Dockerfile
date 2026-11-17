# Reproducible development / CI image with every tool the presets expect.
#   docker build -t geokernel-dev .
#   docker run --rm -it -v "$PWD":/src -w /src geokernel-dev scripts/build.sh asan
FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential g++-13 \
        clang-18 clang-tidy-18 clang-format-18 llvm-18 \
        cmake ninja-build git ca-certificates \
        python3 python3-pip pipx doxygen graphviz \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/clang        clang        /usr/bin/clang-18 100 \
    && update-alternatives --install /usr/bin/clang++      clang++      /usr/bin/clang++-18 100 \
    && update-alternatives --install /usr/bin/clang-tidy   clang-tidy   /usr/bin/clang-tidy-18 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-18 100

ENV PIPX_BIN_DIR=/usr/local/bin
RUN pipx install gcovr

# Non-root user for bind-mounted sources
RUN useradd -m dev
USER dev
WORKDIR /src
CMD ["bash"]
