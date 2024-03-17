#!/usr/bin/env bash

# Run the gcovr tool. Defaults to the text output (.txt file). It will create a coverage.*
# file with the respective type (txt, xml, html) in the <path>/coverage folder. If the dir
# does not exist at the time, it will be created automatically.
#
# The HTML type is a bit special, it will create a HTML report directly.

show_usage() {
  printf "Usage: %s [-h|--help] [--txt|--xml|--html] <path>\n" "${0}"
  exit "${1}"
}

args=()
path=""
mime="txt"

while [[ "${#}" -gt "0" ]]; do
  case "${1}" in
    -h|--help)
      show_usage 0;;
    --txt)
      shift
      path="$(readlink -f -- "${1}")"
      shift;;
    --xml)
      shift
      path="$(readlink -f -- "${1}")"
      mime="xml"
      args+=(--cobertura-pretty)
      shift;;
    --html)
      shift
      path="$(readlink -f -- "${1}")"
      mime="html"
      args+=(--html-details --html-theme github.green)
      shift;;
    *)
      printf '%s\n' "Unknown argument '${1}'"
      show_usage 1;;
  esac
done

# Check if the specified directory exists.
printf "Check for path '%s'...\n" "${path}"
if [ ! -d "${path}" ]; then
  printf "The specified directory '%s' does not exist.\n" "${path}"
  exit 1
fi

# Create the coverage directory in the path.
if [ ! -d "${path}/coverage" ]; then
  mkdir -p "${path}/coverage"
fi

gcovr \
  --exclude ".*/Test*" --exclude ".*/json.hpp" --exclude ".*/doctest.h" \
  --gcov-executable "llvm-cov gcov" \
  --gcov-ignore-parse-errors all \
  --output "${path}/coverage/coverage.${mime}" \
  ${args[*]} \
  "${path}"
