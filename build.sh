#!/bin/bash

my_dir="$(dirname "$(realpath "$0")")"

: "${build_dir:="${my_dir}/build"}"
: "${use_docker:="yes"}"

if [[ "${use_docker}" == "yes" ]]; then
    echo "Using docker for build.."
    use_docker=no exec "${my_dir}/docker-run.sh" "$0" "$@"
fi

mkdir -p "${build_dir}"
cd "${build_dir}" || exit 1
if [[ "$1" == "-r" ]]; then
    rm -fr -- * .*
fi

conan install .. --build=missing &&
    cmake .. -G Ninja &&
    cmake --build . &&
    ./bin/hello
