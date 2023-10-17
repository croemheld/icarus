#!/bin/sh

set -e

# Read the version string and assign it to each variable
IFS=. read -r CMAKE_MAJOR CMAKE_MINOR CMAKE_PATCH <<EOF
$1
EOF

# Starting from 3.20.X, the name of the script was slightly changed
if [ $CMAKE_MAJOR -eq 3 -a $CMAKE_MINOR -ge 20 ]; then
  LINUX="linux"
else
  LINUX="Linux"
fi

# Download and execute the script to install the cmake binary
wget https://github.com/Kitware/CMake/releases/download/v"${1}"/cmake-"${1}"-"${LINUX}"-x86_64.sh -q -O cmake-install.sh
chmod +x cmake-install.sh
./cmake-install.sh --prefix=/usr --exclude-subdir --skip-license
rm -f cmake-install.sh
