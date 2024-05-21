#!/usr/bin/env bash

origin=origin

declare -a remotes local

readarray -t remotes < <(git remote)
declare -A aremotes alocal_b

# fetch all remotes
for remote in "${remotes[@]}"; do
    aremotes["$remote"]=1
    echo doing git fetch -a "$remote"
done

# get local branches
readarray -t local_b < <(git for-each-ref --format='%(refname:short)' refs/heads/)

# build local ref array
for ref in "${local_b[@]}"; do
    alocal_b[$ref]=1
done


readarray -t orig_b  < <(git for-each-ref --format='%(refname:short)' \
                             refs/remotes/"$origin"/)

declare -p remotes
#declare -p aremotes
declare -p local_b orig_b

# find-out missing local branches
for remote_b in "${orig_b[@]}"; do
    short=${remote_b#"$origin"/};
    echo "$remote_b -> $short ${alocal_b[$short]}"
    if ! [[ -v alocal_b[$short] ]]; then
        echo git switch -t "$remote_b"
    fi
done






#git remote | xargs -n 1 git fetch -a
