#!/usr/bin/env bash
#
# Duplicate file, with history (well, sort of)
#
# Copy a git file, keeping history.
# Sources:
#   https://stackoverflow.com/a/53849613/3079831
#   https://stackoverflow.com/a/75942970/3079831

CMDNAME=${0##*/}                                  # script name

if (( $# != 2 )); then
   printf "Usage: %s original copy\n" "$CMDNAME"
   exit 1
fi

from="$1"
to="$2"
branch="split-file"

printf "Dup from=[%s] to=[%s] branch=[%s]\n" "$from" "$to" "$branch"

git checkout -b "$branch"                         # create and switch to branch

git mv "$from" "$to"                              # make the duplicate
git commit -m "Duplicate $from to $to"

git checkout HEAD~ "$from"                        # bring back the original
git commit -m "Restore duplicated $from"

git checkout -                                    # switch back to source branch
git merge --no-ff "$branch" -m "Merge $branch"    # merge dup into source branch


exit 0
