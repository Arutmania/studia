#!/bin/sh

# exit on error
set -e

# show help and exit
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

# if no arguments show help and exit
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

# OPTIND is the index of last consumed argument. see: man getopts
# it is necessary because we cannot shift in the loop because arguments may be
# specified as -a -b or -ab
# shift so that first not used yet argument is first
shift $((OPTIND - 1))

# use gsed (gnu sed) if available
if command -v gsed > /dev/null; then
    SED='gsed'
else
    SED='sed'
fi

# use first argument as pattern if no pattern was specified by -l or -u
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

# it's not enough to check test -e because it doesn't work for broken symlinks
# without returns exit status is the exit status of last command
exists() {
    [ -e "$1" ] || [ -L "$1" ]
}

modify() {
    # return if source doesn't exist
    exists "$1" || return 0

    base="$(basename -- "$1")"
    # case used to maintain full POSIX shell compatibility
    # if there is a dot that is not a first character of filename
    # set $extension to dot and what follows first dot that is not a first
    # character of a filename using parameter substitution:
    #   # - remove smallest prefix matching
    #   ? - any one character
    #   * - zero or more characters
    #   . - literal dot
    case $base in
        ?*.*) extension=".${base#?*.}" ;;
        *   ) ;;
    esac

    # if extension is not set it expands to empty string and everything just werks
    base="$(basename -- "$1" "$extension" | $SED -e "$pattern")$extension"

    # it may so happen that applying sed will result in empty string
    # but files without name are not allowed - return
    if [ -z "$base" ]; then
        echo 'empty filename not allowed - ignoring' >&2
        return 0
    fi

    target="$(dirname -- "$1")/$base"

    # don't overwrite existing files
    if exists "$target"; then
        echo "'$target' already exists - ignoring'" >&2
    else
        # -n = REALLY DON'T OVERWRITE EXISTING FILES
        # -- used so that files starting with dashes won't cause problems
        mv -n -- "$1" "$target"
    fi
    unset base extension target
}

# visit all files (and directories) in postorder and modify them
recurse() {
    # if directory recurse all children, modify on the way up
    if [ -d "$1" ]; then
        for h in "$1"/*; do
            recurse "$h"
        done
        unset h
    fi
    # if here we are either going back up or encountered a leaf node (file)
    # right now all files (also directories) are modified - if directories are
    # not to be modified following statement should be put in elif [ -f "$1" ]
    modify "$1"
}

# if no other arguments are provided default to current directory
# for recursive it will be just current directory
# and for not recursive all files in current directory
if [ 1 -gt "$#" ]; then
    echo 'no files provided, using files in current directory'
    # check is done in this way because maybe if someone set $recursive to value
    # starting with dash it could possibly cause problems? not sure though
    if [ 'true' = "$recursive" ]; then
        # set argument array to be current directory
        set -- .
    else
        # set argument array to be globbed all files in current directory
        # -- for when there could be files that start with dash
        set -- *
    fi
fi

# when iterating over arguments we can use shorter syntax
# we use the argument array because it integrates uniformly when using
# positional arguments and when defaulted to files in current directory
# if we would to use variables for the defaulted case files with whitespace etc.
# would break and there is no other way to create an array in standard POSIX shell
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
