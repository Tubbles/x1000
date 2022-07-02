#!/bin/bash

my_dir="$(dirname "$(realpath "$0")")"

: "${run_script:="${my_dir}/docker-run.sh"}"
: "${build_script:="${my_dir}/build.sh"}"

"${run_script}" env use_docker=no "${build_script}" "$@"
