#!/bin/sh

set -e

usage() {
cat << EOT
usage:
    $0 [-h]
    $0 [-r] [-l | -u | PATTERN] <[dir/]...file>...
options:
    -h              shows this message
    -r              recurse directiories
    -l | -u         lowercase | uppercase, gnu sed required
    PATTERN         sed pattern used to motify file names
misc:
    options may be specified in either -abc or -a -b -c format.
    each option and exclusive options (both -l and -u) may be specified,
    the last one wins (-lu will behave as only -u would be specified).
    one of -l, -u or PATTERN is required.
    when neither -l or -u was specified first positional argument is treated
    as the pattern.

    for example usage see modify_examples.
EOT

    exit 0
}

LOWERCASE='s/[A-Z]/\L&/g'
UPPERCASE='s/[a-z]/\U&/g'

[ 0 -eq "$#" ] && usage

while getopts 'hrlu' option; do
    case $option in
        h) usage ;;
        r) recursive='true' ;;
        l) pattern="$LOWERCASE" ;;
        u) pattern="$UPPERCASE" ;;
        *) exit 1 ;;
    esac
done
unset option

shift $((OPTIND - 1))

# use gsed (gnu sed) if available
if command -v gsed > /dev/null; then
    SED='gsed'
else
    SED='sed'
fi

if [ -z "$pattern" ]; then
    if [ 1 -gt "$#" ]; then
        echo 'no pattern provided' >&2
        exit 1
    fi
    pattern="$1"
    shift
# if pattern is set then it must be either LOWERCASE or UPPERCASE
# check if $SED is gnu sed - BSD sed doesn't seem to have --version
elif ! $SED --version > /dev/null 2>&1; then
    echo 'gnu sed is required for -l or -u' >&2
    exit 1
fi

exists() {
    [ -e "$1" ] || [ -L "$1" ]
}

modify() {
    exists "$1" || return 0

    base="$(basename -- "$1" | $SED -e "$pattern")"
    if [ -z "$base" ]; then
        echo 'empty filename not allowed - ignoring' >&2
        return 0
    fi

    target="$(dirname -- "$1")/$base"

    if exists "$target"; then
        echo "'$target' already exists - ignoring'" >&2
    else
        mv -n -- "$1" "$target"
    fi
}

recurse() {
    if [ -d "$1" ]; then
        for h in "$1"/*; do
            recurse "$h"
        done
        unset h
    fi
    modify "$1"
}

if [ 1 -gt "$#" ]; then
    echo 'no files provided, using files in current directory'
    if [ 'true' = "$recursive" ]; then
        set -- .
    else
        set -- *
    fi
fi

for f do
    if [ -d "$f" ] && [ 'true' = "$recursive" ]; then
        # when directory is provided and recursive set modify in directory,
        # not directory itself:
        # modify -ru dir -> dir/FILE, not DIR/FILE
        for g in "$f"/*; do
            recurse "$g"
        done
        unset g
    elif exists "$f"; then
        modify "$f"
    else
        echo "'$f' does not exist" >&2
    fi
done
unset f
