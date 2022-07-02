#!/bin/bash

# This is the entrypoint for the docker image

my_dir="$(dirname "$(realpath "$0")")"

: "${username:="$(id -nu)"}"
: "${groupname:="$(id -ng)"}"
: "${userid:="$(id -u)"}"
: "${groupid:="$(id -g)"}"
: "${entrypoint:="${my_dir}/entrypoint.sh"}"
: "${silent:="no"}"

if [[ -n "${conan_dir+x}" ]]; then
    if [[ -n "${CONAN_USER_HOME}" ]]; then
        conan_dir="${CONAN_USER_HOME}/.conan"
        conan_dir_env="--env CONAN_USER_HOME=${CONAN_USER_HOME}"
    else
        conan_dir="${HOME}/.conan"
        conan_dir_env=""
    fi
    conan_cache="--volume ${conan_dir}:${conan_dir}"
    echo "Using conan cache in ${conan_dir}"
else
    conan_cache=""
fi

set -x
# shellcheck disable=SC2086
docker run --rm -it \
    --env USERNAME="${username}" \
    --env GROUPNAME="${groupname}" \
    --env USERID="${userid}" \
    --env GROUPID="${groupid}" \
    ${conan_dir_env} \
    ${conan_cache} \
    --volume "${my_dir}":"${my_dir}" \
    --workdir "${my_dir}" \
    --entrypoint "${entrypoint}" \
    x1000 "$@"
