#!/bin/bash
cd "$(dirname "$0")" || exit 1
FZF_DEFAULT_COMMAND="grep -Rn '' src" \
  fzf --delimiter=: --bind 'enter:execute(less -N +{2} {1})'
