#!/bin/bash

my_dir="$(dirname "$(realpath "$0")")"

: "${build_dir:="${my_dir}/build"}"
: "${use_docker:="yes"}"
: "${conan_dir:="${HOME}/.conan"}"

if [[ "${use_docker}" == "yes" ]]; then
    echo "Using docker for build.."
    exec "${my_dir}/docker-run.sh" env use_docker=no "$0" "$@"
fi

mkdir -p "${build_dir}"
cd "${build_dir}" || exit 1
if [[ "$1" == "-r" ]]; then
    rm -fr -- * .*
fi

mkdir -p "${conan_dir}"

conan install .. --build=missing &&
    CMAKE_BUILD_TYPE=Debug cmake .. -G Ninja &&
    ninja "$@"
