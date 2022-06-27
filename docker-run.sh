#!/bin/bash

# This is the entrypoint for the docker image

my_dir="$(dirname "$(realpath "$0")")"

#: "${VARIABLE:=DEFAULT_VALUE}"
: "${username:="$(id -nu)"}"
: "${groupname:="$(id -ng)"}"
: "${userid:="$(id -u)"}"
: "${groupid:="$(id -g)"}"
: "${conan_cache:="--volume /home/${username}/.conan:/home/${username}/.conan"}"
: "${entrypoint:="${my_dir}/entrypoint.sh"}"

set -x
# shellcheck disable=SC2068
# shellcheck disable=SC2086
docker run --rm -it \
    --env USERNAME="${username}" \
    --env GROUPNAME="${groupname}" \
    --env USERID="${userid}" \
    --env GROUPID="${groupid}" \
    ${conan_cache} \
    --volume "${my_dir}":"${my_dir}" \
    --workdir "${my_dir}" \
    --entrypoint "${entrypoint}" \
    x1000 $@
