#!/usr/bin/env bash

###############################################################
# Helper script for building docker image with LLVM libraries #
###############################################################

# The script was heavily inspired by LLVM's docker scripts in
# order to keep the resemblance. The differences between them
# are as follows:
#
# - We do not support cherry-picking 

set -e

UBUNTU_RELEASE="jammy"
CMAKE_VERSION="3.27.7"

ICARUS_SRC=""

DOCKER_REP=""
DOCKER_TAG=""

LLVM_CHECKOUT=""
CMAKE_TARGETS=""

PARALLEL_JOBS=""
PRINT_DRY_RUN=""

INSTALL_WHOLE=0

function show_usage() {
  cat << EOF
Usage: ./docker/build-docker.sh [-h | --help] [-s | --source <source>] [-r | --repository <repository>]
                                [-t | --tag <tag>] [-c | --commit <commit or branch>] [-i | --install-target <target>]
                                [-j | --jobs <jobs>] [-d | --dry-run] [-a | --install-all]
EOF
}

function docker_build() {
  if [ "${PRINT_DRY_RUN}" == "" ]; then
    printf '%s\n' "Building ${DOCKER_REP}:${DOCKER_TAG}:"
    "${@}"
  else
    printf '%s\n' "Build command for ${DOCKER_REP}:${DOCKER_TAG}:"
    printf '%s %s\n' "${@}" | sed '2,$s/^/  /g'
  fi
}

while [[ "${#}" -gt "0" ]]; do
  case "${1}" in
    -h|--help)
      show_usage
      exit 0;;
    -s|--source)
      shift
      ICARUS_SRC="${1}"
      shift;;
    -r|--repository)
      shift
      DOCKER_REP="${1}"
      shift;;
    -t|--tag)
      shift
      DOCKER_TAG="${1}"
      shift;;
    -c|--checkout)
      shift
      LLVM_CHECKOUT="${1}"
      shift;;
    -i|--install-target)
      shift
      CMAKE_TARGETS="${CMAKE_TARGETS} ${1}"
      shift;;
    -j|--jobs)
      shift
      PARALLEL_JOBS="${1}"
      shift;;
    -d|--dry-run)
      PRINT_DRY_RUN="1"
      shift;;
    -a|--install-all)
      INSTALL_WHOLE=1
      shift;;
    *)
      printf '%s\n' "Unknown argument '${1}'"
      exit 1;;
  esac
done

command -v docker > /dev/null || {
  printf '%s\n' "Docker binary not found. Please install Docker to use this script."
  exit 1
}

if [ "${DOCKER_REP}" == "" ]; then
  printf '%s\n' "Missing argument '-r|--repository'."
  exit 1
fi

if [ "${DOCKER_TAG}" == "" ]; then
  printf '%s\n' "Missing argument '-t|--tag'."
  exit 1
fi

if [ "${ICARUS_SRC}" == "" ]; then
  ICARUS_SRC=$(dirname -- "$(dirname -- "$(readlink -f -- "${BASH_SOURCE:-$0}")")")
fi

if [ "${LLVM_CHECKOUT}" == "" ]; then
  LLVM_CHECKOUT="main"
fi

if [ "${PARALLEL_JOBS}" == "" ]; then
  PARALLEL_JOBS="$(nproc)"
fi

if [ "$(docker images -q "${DOCKER_REP}:base" 2> /dev/null)" == "" ]; then
  docker_build docker build \
    -t "${DOCKER_REP}:base" \
    -f "${ICARUS_SRC}/docker/Dockerfile.base" \
    --build-arg "UBUNTU_RELEASE=${UBUNTU_RELEASE}" \
    "${ICARUS_SRC}"
fi

if [ "${INSTALL_WHOLE}" -eq "1" ]; then
  CMAKE_TARGETS="all"
fi

docker_build docker build \
  -t "${DOCKER_REP}:${DOCKER_TAG}" \
  -f "${ICARUS_SRC}/docker/Dockerfile" \
  --build-arg "DOCKER_BASEIMG=${DOCKER_REP}:base" \
  --build-arg "UBUNTU_RELEASE=${UBUNTU_RELEASE}" \
  --build-arg "CMAKE_VERSION=${CMAKE_VERSION}" \
  --build-arg "PARALLEL_JOBS=${PARALLEL_JOBS}" \
  --build-arg "LLVM_CHECKOUT=${LLVM_CHECKOUT}" \
  --build-arg "CMAKE_TARGETS=${CMAKE_TARGETS}" \
  "${ICARUS_SRC}"
