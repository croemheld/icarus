#!/usr/bin/env bash

set -e

DOCKER_REP="croemheld/icarus-ci"
DOCKER_TAG="clang"

LLVM_VERSIONS=()

PARALLEL_JOBS=""
PRINT_DRY_RUN=""

INSTALL_TARGETS=()

function show_usage() {
  cat << EOF
Usage: ./docker/build-docker.sh [-h | --help] [-d | --dry-run] [-j | --jobs <jobs>] [-r | --repository <repository>]
                                [-t | --tag <tag>] [--llvm-version <llvm version>] [-i | --install-target <target>]
EOF
}

function docker_build() {
  if [ "${PRINT_DRY_RUN}" == "" ]; then
    printf '%s\n' "Building docker image(s):"
    "${@}"
  else
    printf '%s\n' "Build command for building docker image(s):"
    "${@}" --dry-run
  fi
}

while [[ "${#}" -gt "0" ]]; do
  case "${1}" in
    -h|--help)
      show_usage
      exit 0;;
    -d|--dry-run)
      PRINT_DRY_RUN="1"
      shift;;
    -j|--jobs)
      shift
      PARALLEL_JOBS="${1}"
      shift;;
    -r|--repository)
      shift
      DOCKER_REP="${1}"
      shift;;
    -t|--tag)
      shift
      DOCKER_TAG="${1}"
      shift;;
    -i|--install-target)
      shift
      INSTALL_TARGETS+=("-i ${1}")
      shift;;
    --llvm-version)
      shift
      LLVM_VERSIONS+=("${1}")
      shift;;
    --install-all)
      INSTALL_TARGETS+=("-a")
      shift;;
    *)
      printf '%s\n' "Unknown argument '${1}'"
      exit 1;;
  esac
done

ICARUS_SRC=$(dirname -- "$(dirname -- "$(readlink -f -- "${BASH_SOURCE:-$0}")")")

if [ "${PARALLEL_JOBS}" == "" ]; then
  PARALLEL_JOBS="1"
fi

for LLVM_VERSION in "${LLVM_VERSIONS[@]}"; do
  IFS=. read -r LLVM_MAJORVER _ <<< "${LLVM_VERSION}"
  DOCKER_LLVM_VERSION="${LLVM_MAJORVER}"
  if [[ "${LLVM_MAJORVER}" -lt "7" ]]; then
    DOCKER_LLVM_VERSION="${DOCKER_LLVM_VERSION}.0"
  fi
  docker_build "${ICARUS_SRC}/docker/build-docker.sh" -s "${ICARUS_SRC}" \
    -r "${DOCKER_REP}" \
    -t "${DOCKER_TAG}-${DOCKER_LLVM_VERSION}" \
    -c "release/${LLVM_MAJORVER}.x" \
    -j "${PARALLEL_JOBS}" \
    ${INSTALL_TARGETS[*]}
done
