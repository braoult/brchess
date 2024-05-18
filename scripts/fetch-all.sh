#!/usr/bin/env bash

git remote | xargs -n 1 git fetch -a
