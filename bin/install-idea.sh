#!/bin/sh

set -eu

dot_ipd="$(cd "$(dirname "$0")"/.. && pwd)"
src="$dot_ipd"/idea
dst="$(dirname "$dot_ipd")"/.idea

rm -Rf "$dst"
mkdir -p "$dst"

cd "$src"

for file in *; do
    ln -vs ../.ipd/idea/"$file" "$dst"
done
