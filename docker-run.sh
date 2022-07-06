#!/bin/bash

my_dir="$(dirname "$(realpath "$0")")"

: "${username:="$(id -nu)"}"
: "${groupname:="$(id -ng)"}"
: "${userid:="$(id -u)"}"
: "${groupid:="$(id -g)"}"
: "${silent:="no"}"
: "${conan_dir:="${HOME}/.conan"}"
: "${use_conan_cache:="yes"}"

if [[ "${use_conan_cache}" == "yes" ]]; then
    conan_cache="--volume ${conan_dir}:${conan_dir}"
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
    ${conan_cache} \
    --volume "${my_dir}":"${my_dir}" \
    --workdir "${my_dir}" \
    x1000 "$@"
