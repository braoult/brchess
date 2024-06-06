#!/usr/bin/env bash
#
# Fetch all branches from remotes, and track missing ones.
#
# The value of variable ORIGIN is used for repository origin. If
# not set, the default is "origin".

default_origin=origin

origin=${ORIGIN:-$default_origin}

declare -a local_b
declare -A alocal_b

# fetch all remotes
git fetch --all --tags

# get local branches, and build reverse associative array
readarray -t local_b < <(git for-each-ref --format='%(refname:short)' refs/heads/)
for ref in "${local_b[@]}"; do
    alocal_b[$ref]=1
done

# get "origin" branches
readarray -t orig_b  < <(git for-each-ref --format='%(refname:short)' \
                             refs/remotes/"$origin"/)

# find-out missing local branches and track them.
# bugs:
#  - We only check local branch existence, not tracking information correctness.
#  - What about sub-branches ? Like remote/a and remote/a/b not being tracked ?
for remote_b in "${orig_b[@]}"; do
    short=${remote_b#"$origin"/};
    # OR (??): short=${remote_b##*/}

    if ! [[ -v alocal_b[$short] ]]; then
        printf "local branch %s set to track %s.\n" "$short" "$remote_b"
        git branch --track "$short" "$remote_b"
    else
        printf "skipping %s.\n" "$remote_b"
    fi
done

git pull -a
