#!/usr/bin/env bash
#
# git-split.sh - duplicate GIT file, with history (well, sort of)
# Sources:
#   https://stackoverflow.com/a/53849613/3079831
#   https://stackoverflow.com/a/75942970/3079831
#
# (C) Bruno Raoult ("br"), 2024
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#
# You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
#
# SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>

CMDNAME=${0##*/}                                  # script name

if (( $# == 0 || $# % 2 )); then
   printf "Usage: %s orig1 copy [orig2 copy2 ...]\n" "$CMDNAME"
   exit 1
fi

yes_or_no() {
    while true; do
        read -rp "$* [y/N]: " yn
        case $yn in
            [Yy])
                return 0
                ;;
            [Nn]|"")
                printf "Aborted.\n"
                return  1
                ;;
        esac
    done
    # not reached
}

# prints out and run a command.
echorun () {
    printf ">>> %s\n" "$*"
    "$@"
    return $?
}

declare -A files
declare -a from                                   # to keep order

while (($#)); do
    if [[ ! -f "$1" ]]; then
        printf "Missing '%s'. Aborting.\n" "$1"
        exit 1
    elif [[ -f "$2" ]]; then
        printf "'%s' already exists. Aborting.\n" "$2"
        exit 1
    fi
    files["$1"]="$2"
    from+=("$1")
    shift 2
done

printf "The following files will be split :\n"

for file in "${from[@]}"; do
    printf "%s -> %s\n" "$file" "${files[$file]}"
done

yes_or_no "Proceed (y/N)? " || exit 1

branch="split-file"

printf -v msg1 "Splitting '%s'" "${from[0]}"
printf -v msg2 "Restore duplicated %s" "${from[0]}"
if (( ${#from[@]} > 1 )); then
    msg1+=" and $(( ${#from[@]} - 1 )) others"
    msg2+=" and $(( ${#from[@]} - 1 )) others"

fi
printf "%s using  branch '$branch'.\n" "$msg1"

if git show-ref --verify --quiet "refs/heads/$branch"; then
    echorun git switch "$branch"
    echorun git merge master
else
    echorun git switch -c "$branch"
fi

for file in "${from[@]}"; do                      # make the duplicates
    echorun git mv "$file" "${files[$file]}"
done
echorun git commit -m "$msg1"                     # ... and commit

for file in "${from[@]}"; do                      # bring back the original
    echorun git checkout HEAD~ "$file"
done
echorun git commit -m "$msg2"                     # ... and commit

echorun git checkout -                                 # switch back to source branch
echorun git merge --no-ff "$branch" -m "Merge $branch" # merge dup into source branch

exit 0
