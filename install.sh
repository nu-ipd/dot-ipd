#!/bin/sh

set -eu

main () {
    # Change to project root:
    cd "$(dirname "$(dirname "$(realpath "$0")")")"

    install_dot_idea
    install_dot_gitignore
    install_dot_github
}

install_dot_idea () {
    mkdir -p .idea

    local src
    local dst
    for src in .ipd/idea/*; do
        dst=.idea/"${src##*/}"
        rm -Rf "$dst"
        ln -vs ../"$src" "$dst"
    done
}

install_dot_gitignore () {
    rm -Rf .gitignore
    cp -v .ipd/gitignore .gitignore
}

install_dot_github () {
    rm -Rf .github
    cp -Rv .ipd/github .github
}

####
main
####
