#!/usr/bin/env bash
#
# Copy a git file, keeping history.
# Source: https://stackoverflow.com/a/53849613/3079831
#
# Buggy, "to" file has no history...
# Maybe try:
#   https://stackoverflow.com/a/75942970/3079831


# if (( $# != 2 )) ; then
#   echo "Usage: git-split.sh original copy"
#   exit 1
# fi

# from="$1"
# to="$2"
# branch="split-file"
# tmp="$from-temp-copy"

# git switch -c "$branch"
# git mv "$from" "$to"
# git commit -n -m "Split $from to $to - step 1"

# #REV=$(git rev-parse HEAD)
# git switch -

# git mv "$from" "$tmp"
# git commit -n -m "Split $from to $to - step 2"
# git merge "$branch"

# git commit -a -n -m "Split $from to $to - step 3"
# git mv "$tmp" "$from"
# git commit -n -m "Split file $from to $to - step 4"

# git branch -d "$branch"

exit 0
