FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        g++ \
        cmake \
        ninja-build \
        git \
        gcc-arm-none-eabi \
        binutils-arm-none-eabi \
        libnewlib-arm-none-eabi \
        python3 \
        python3-pip \
        bash-completion \
        vim \
        emacs-nox \
    && pip install --break-system-packages lizard \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

CMD ["bash", "-c", "\
    cmake --preset host-debug \
    && cmake --build --preset host-debug \
    && ctest --preset host-debug \
    && cmake --preset arm-cortexm4 \
    && cmake --build --preset arm-cortexm4 \
    && ./scripts/compare_size.sh \
    && ./scripts/lizard_report.sh"]
