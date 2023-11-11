ARG UBUNTU_RELEASE=bionic

#######################################
# Hack to get base image name in file #
#######################################

ARG DOCKER_BASEIMG

FROM ${DOCKER_BASEIMG} AS base

#######################################
# Stage 2: Checkout LLVM rev. & build #
#######################################

FROM ubuntu:${UBUNTU_RELEASE} AS builder

WORKDIR /root

#######################################
# Add LLVM project from previous step #
#######################################

COPY --from=base /root/llvm-project /root/llvm-project

#######################################
# Add scripts for building LLVM/Clang #
#######################################

ADD docker/install-cmake.sh /root/install-cmake.sh
ADD docker/build-llvm.sh /root/llvm-project/build-llvm.sh

#######################################
# Start building LLVM project & Clang #
#######################################

ARG CMAKE_VERSION
ARG CMAKE_TARGETS
ARG PARALLEL_JOBS
ARG LLVM_CHECKOUT

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        ca-certificates git wget build-essential ninja-build python3 zlib1g-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && \
    ./install-cmake.sh "${CMAKE_VERSION}" && \
    cd llvm-project && ./build-llvm.sh "${LLVM_CHECKOUT}" "${PARALLEL_JOBS}"

#######################################
# Stage 3: Copy finished build folder #
#######################################

FROM ubuntu:${UBUNTU_RELEASE} AS installer

WORKDIR /root

#######################################
# Add LLVM project from previous step #
#######################################

COPY --from=builder /root/llvm-project /root/llvm-project

#######################################
# Add script to install LLVM projects #
#######################################

ADD cmake/LLVMLibraries.cmake /root/LLVMLibraries.cmake
ADD docker/install-cmake.sh /root/install-cmake.sh
ADD docker/install-llvm.sh /root/llvm-project/install-llvm.sh

ARG CMAKE_VERSION
ARG CMAKE_TARGETS

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        ca-certificates git wget build-essential ninja-build python3 zlib1g-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && \
    ./install-cmake.sh "${CMAKE_VERSION}" && \
    cd llvm-project && ./install-llvm.sh ${CMAKE_TARGETS}

#######################################
# Stage 4: Copy clang++ and libraries #
#######################################

FROM ubuntu:${UBUNTU_RELEASE} AS release

WORKDIR /root

COPY --from=installer /usr/local/bin /usr/local/bin
COPY --from=installer /usr/local/lib /usr/local/lib
COPY --from=installer /usr/local/include /usr/local/include

ADD docker/install-cmake.sh /root/install-cmake.sh

ARG CMAKE_VERSION

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        libc6-dev libstdc++-7-dev wget ca-certificates binutils ninja-build zlib1g-dev && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && \
    ./install-cmake.sh "${CMAKE_VERSION}"
