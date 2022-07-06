#!/bin/bash

my_dir="$(dirname "$(realpath "$0")")"

"${my_dir}/docker-run.sh" env use_docker=no "${my_dir}/build.sh" "$@"
