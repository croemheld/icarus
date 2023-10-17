ARG UBUNTU_RELEASE=bionic

#######################################
# Hack to get base image name in file #
#######################################

ARG DOCKER_BASEIMG

FROM $DOCKER_BASEIMG AS base

#######################################
# Stage 2: Checkout LLVM rev. & build #
#######################################

FROM ubuntu:$UBUNTU_RELEASE AS builder

WORKDIR /root

#######################################
# Add CMake, LLVM from previous stage #
#######################################

COPY --from=base /usr/bin/cmake /usr/bin/cmake
COPY --from=base /root/llvm-project /root/llvm-project

ENV PATH="/usr/bin/cmake/bin:${PATH}"

#######################################
# Add scripts for building LLVM/Clang #
#######################################

ADD build-llvm.sh /root/llvm-project/build-llvm.sh

#######################################
# Start building LLVM project & Clang #
#######################################

ARG CMAKE_VERSION
ARG CMAKE_INSTALL
ARG PARALLEL_JOBS
ARG LLVM_CHECKOUT

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        ca-certificates git wget build-essential ninja-build python3 zlib1g-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && cd llvm-project && \
    ./build-llvm.sh $LLVM_CHECKOUT $PARALLEL_JOBS

#######################################
# Stage 3: Copy finished build folder #
#######################################

FROM ubuntu:$UBUNTU_RELEASE AS installer

WORKDIR /root

#######################################
# Add CMake, LLVM from previous stage #
#######################################

COPY --from=builder /usr/bin/cmake /usr/bin/cmake
COPY --from=builder /root/llvm-project /root/llvm-project

ENV PATH="/usr/bin/cmake/bin:${PATH}"

#######################################
# Add script to install LLVM projects #
#######################################

ADD install-llvm.sh /root/llvm-project/install-llvm.sh

ARG CMAKE_VERSION
ARG CMAKE_INSTALL

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        ca-certificates git wget build-essential ninja-build python3 zlib1g-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && cd llvm-project && \
    ./install-llvm.sh $CMAKE_INSTALL

#######################################
# Stage 4: Copy clang++ and libraries #
#######################################

FROM ubuntu:$UBUNTU_RELEASE

WORKDIR /root

COPY --from=installer /usr/local/bin /usr/local/bin
COPY --from=installer /usr/local/lib /usr/local/lib
COPY --from=installer /usr/local/include /usr/local/include

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        libc6-dev libstdc++-7-dev wget ca-certificates binutils ninja-build zlib1g-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/*\
    wget https://github.com/Kitware/CMake/releases/download/v"$CMAKE_VERSION"/cmake-"$CMAKE_VERSION"-Linux-x86_64.sh \
        -q -O cmake-install.sh && \
    chmod +x cmake-install.sh && ./cmake-install.sh --prefix=/usr --exclude-subdir --skip-license && \
    rm -f cmake-install.sh
