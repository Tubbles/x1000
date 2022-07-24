#!/bin/bash

my_dir="$(dirname "$(realpath "$0")")"

: "${build_dir:="${my_dir}/build"}"
: "${use_docker:="yes"}"
: "${use_conan_venv:="yes"}"
: "${conan_dir:="${HOME}/.conan"}"

if [[ "${use_docker}" == "yes" ]]; then
    if command -v docker &>/dev/null; then
        echo "$0: Info: Using docker for build.."
        exec "${my_dir}/docker-run.sh" "$0" "$@"
    fi
fi

mkdir -p "${build_dir}"
cd "${build_dir}" || exit 1
if [[ "$1" == "--distclean" ]]; then
    rm -fr -- * .* &>/dev/null
    shift
fi

if [[ "$1" == "-r" ]]; then
    rm -fr -- * .ninja*
    shift
fi

if [[ "${use_conan_venv}" == "yes" ]]; then
    conan_dir="${build_dir}/.conan"
    export CONAN_USER_HOME="${build_dir}"
    if [[ ! -d "${conan_dir}" ]]; then
        echo "$0 Info: No conan cache found, setting up basic config.. "
        :
        mkdir -p "${conan_dir}"
        conan profile new --detect default &>/dev/null
        conan profile update settings.compiler.libcxx=libstdc++11 default
        conan profile show default
        echo "$0 Info: Please double check that the settings are correct and re-run this script"
        echo "$0 Info: If you need to make changes, edit the file ${conan_dir}/profiles/default"
        exit 1
    fi
fi

conan install .. --build=missing &&
    CMAKE_BUILD_TYPE=Release cmake .. -G Ninja &&
    ninja "$@"
