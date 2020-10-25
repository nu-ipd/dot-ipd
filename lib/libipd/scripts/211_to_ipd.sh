#!/bin/sh

# 211_to_ipd.sh - replaces various forms of "211" with "IPD"

set -eu

usage () {
    cat <<-....EOF
	$(basename "$0") - changes forms of “211” to “IPD.”
	Usage: $0 FILE...
....EOF
}

main () {
    process_options "$@"

    for file; do
        process_one_file "$file"
    done
}

process_options () {
    if [ $# = 0 ]; then
        usage>&2
        exit 2
    fi

    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -*)
            echo>&2 "$0: Unknown option: ‘$1’"
            exit 1
            ;;
    esac
}

process_one_file () {
    sed -i.bak -E '
        s/CS 211/IPD/g
        s/lib211/libipd/g
        s/211([.]h|_[a-z])/ipd\1/g
        s/211(_[A-Z])/IPD\1/g
    ' "$1"
    rm "$1".bak
}

#########
main "$@"
#########
