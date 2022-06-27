#!/bin/bash

my_dir="$(dirname "$(realpath "$0")")"

: "${script:="${my_dir}/docker-run.sh"}"

"${script}" "$@"
