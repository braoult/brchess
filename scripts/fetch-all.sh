#!/usr/bin/env bash

origin=origin

declare -a remotes local_b
#declare -A aremotes
declare -A alocal_b

# get remotes list
readarray -t remotes < <(git remote)

# fetch all remotes
git fetch --all

# fill associative array with remote
#for remote in "${remotes[@]}"; do
#    aremotes["$remote"]=1
#    echo doing git fetch -a "$remote"
#done

# get local branches
readarray -t local_b < <(git for-each-ref --format='%(refname:short)' refs/heads/)

# build local ref array
for ref in "${local_b[@]}"; do
    alocal_b[$ref]=1
done

# get origin branches
readarray -t orig_b  < <(git for-each-ref --format='%(refname:short)' \
                             refs/remotes/"$origin"/)

declare -p remotes
declare -p local_b orig_b

# find-out missing local branches and track them
for remote_b in "${orig_b[@]}"; do
    short=${remote_b#"$origin"/};
    #echo "$remote_b -> $short ${alocal_b[$short]}"
    if ! [[ -v alocal_b[$short] ]]; then
        echo git branch --track "$short" "$remote_b"
        # echo git branch:  "$remote_b"
    fi
done

echo git pull --all






#git remote | xargs -n 1 git fetch -a
