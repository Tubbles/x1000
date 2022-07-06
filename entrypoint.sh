#!/bin/bash

# This is the entrypoint for the docker image

: "${USERNAME:="builderbob"}"
: "${GROUPNAME:="builderbob"}"
: "${USERID:="1000"}"
: "${GROUPID:="1000"}"

groupadd --gid "${GROUPID}" "${GROUPNAME}"
useradd --create-home --uid "${USERID}" --gid "${GROUPID}" "${USERNAME}" 2>/dev/null
user_home="$(getent passwd "${USERNAME}" | cut -d: -f6)"
chown "${USERNAME}":"${GROUPNAME}" "${user_home}"
install -o "${USERID}" -g "${GROUPID}" -t "${user_home}" $(find "/etc/skel" -type f)

su-exec "${USERNAME}":"${GROUPNAME}" conan profile new default --detect &>/dev/null
test $? && su-exec "${USERNAME}":"${GROUPNAME}" conan profile update settings.compiler.libcxx=libstdc++11 default &>/dev/null

if [[ -n "$*" ]]; then
    exec su-exec "${USERNAME}":"${GROUPNAME}" "$@"
else
    exec su-exec "${USERNAME}":"${GROUPNAME}" env PS1='\u@\h \w\n\$ ' "bash"
fi
