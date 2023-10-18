#!/bin/bash

###############################################################
# Helper script for building docker image with LLVM libraries #
###############################################################

# The script was heavily inspired by LLVM's docker scripts in
# order to keep the resemblance. The differences between them
# are as follows:
#
# - We do not support cherry-picking 

set -e

ICARUS_SRC=""

DOCKER_REP=""
DOCKER_TAG=""

LLVM_CHECKOUT=""

CMAKE_OPTIONS=""
CMAKE_INSTALL=""

PARALLEL_JOBS=""

function show_usage() {
	cat << EOF

EOF
}

while [[ $# -gt 0 ]]; do
	case "$1" in
		-h|--help)
			show_usage
			exit 0;;
		-s|--source)
			shift
			DOCKER_SRC="$1"
			shift;;
		-r|--repository)
			shift
			DOCKER_REP="$1"
			shift;;
		-t|--tag)
			shift
			DOCKER_TAG="$1"
			shift;;
		-c|--checkout)
			shift
			LLVM_CHECKOUT="$1"
			shift;;
		-i|--install-target)
			shift
			CMAKE_INSTALL="$CMAKE_INSTALL $1"
			shift;;
		-j|--compile-jobs)
			shift
			PARALLEL_JOBS="$1"
			shift;;
		*)
			echo "Unknown argument '$1'"
			exit 1;;
	esac
done

command -v docker > /dev/null || {
	echo "Docker binary not found. Please install Docker to use this script."
	exit 1
}

if [ "$DOCKER_REP" == "" ]; then
	echo "Missing argument '-r|--repository'."
	exit 1
fi

if [ "$DOCKER_TAG" == "" ]; then
	echo "Missing argument '-t|--tag'."
	exit 1
fi

if [ "$ICARUS_SRC" == "" ]; then
	ICARUS_SRC=$(dirname -- "$(dirname -- "$(readlink -f -- "$BASH_SOURCE")")")
fi

if [ "$LLVM_CHECKOUT" == "" ]; then
	LLVM_CHECKOUT="main"
fi

if [ "$PARALLEL_JOBS" == "" ]; then
	PARALLEL_JOBS="1"
fi

if [ "$CMAKE_INSTALL" == "" ]; then
	CMAKE_TARGETS="clang clang-headers llvm-headers cmake-exports"
	CMAKE_INSTALL=$(echo "$CMAKE_TARGETS" | sed 's/[^ ]* */install-&/g')
fi

if [ "$(docker images -q ${DOCKER_REP}:base 2> /dev/null)" == "" ]; then
	docker build \
		-t "${DOCKER_REP}:base" \
		-f "$ICARUS_SRC/docker/Dockerfile.base" \
		--build-arg "UBUNTU_RELEASE=bionic" \
		--build-arg "CMAKE_VERSION=3.13.4" \
		"$ICARUS_SRC"
fi

echo "Building ${DOCKER_REP}:${DOCKER_TAG}"
docker build \
	-t "${DOCKER_REP}:${DOCKER_TAG}" \
	-f "$ICARUS_SRC/docker/Dockerfile" \
	--build-arg "DOCKER_BASEIMG=${DOCKER_REP}:base" \
	--build-arg "UBUNTU_RELEASE=bionic" \
	--build-arg "CMAKE_VERSION=3.27.7" \
	--build-arg "PARALLEL_JOBS=$PARALLEL_JOBS" \
	--build-arg "LLVM_CHECKOUT=$LLVM_CHECKOUT" \
	--build-arg "CMAKE_INSTALL=$CMAKE_INSTALL" \
	"$ICARUS_SRC"
