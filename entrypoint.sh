#!/bin/bash

# This is the entrypoint for the docker image

: "${USERNAME:="builderbob"}"
: "${GROUPNAME:="builderbob"}"
: "${USERID:="1000"}"
: "${GROUPID:="1000"}"

groupadd --gid "${GROUPID}" "${GROUPNAME}"
useradd --create-home --uid "${USERID}" --gid "${GROUPID}" "${USERNAME}" 2>/dev/null

export PS1='\u@\h \w\n\$ '

if [[ -n "$*" ]]; then
    exec su-exec "${USERNAME}":"${GROUPNAME}" "$@"
else
    exec su-exec "${USERNAME}":"${GROUPNAME}" "bash"
fi
