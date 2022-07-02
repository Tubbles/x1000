#!/bin/bash

my_dir="$(dirname "$(realpath "$0")")"

: "${build_dir:="${my_dir}/build"}"
: "${use_docker:="yes"}"
: "${CONAN_USER_HOME:="yes"}"

if [[ "${use_docker}" == "yes" ]]; then
    echo "Using docker for build.."
    exec "${my_dir}/docker-run.sh" env use_docker=no "$0" "$@"
fi

ls -Al

mkdir -p "${build_dir}"
cd "${build_dir}" || exit 1
if [[ "$1" == "-r" ]]; then
    rm -fr -- * .*
fi

if [[ -n "${CONAN_USER_HOME}" ]]; then
    conan_dir="${CONAN_USER_HOME}/.conan"
else
    conan_dir="${HOME}/.conan"
fi

mkdir -p "${conan_dir}"

conan install .. --build=missing &&
    cmake .. -G Ninja &&
    cmake --build . &&
    ./bin/hello
