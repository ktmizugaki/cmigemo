#!/bin/bash

_migemo_complete_query() {
    local word=$1 pid
    local MIGEMO_DIR=@MIGEMO_DIR@
    local MIGEMO_BIN="$MIGEMO_DIR/cmigemo"
    local MIGEMO_DICT="$MIGEMO_DIR/dict/migemo-dict"
    local MIGEMO_SOCKET="$MIGEMO_DIR/socket"
    local MIGEMO_PID="$MIGEMO_DIR/socket.pid"

    if [ -n "$MIGEMO_SOCKET" ]; then
        pid=$(cat "$MIGEMO_PID" 2>/dev/null)
        if ! kill -0 "$pid" 2>/dev/null; then
            "$MIGEMO_BIN" -c "$MIGEMO_SOCKET" -d "$MIGEMO_DICT" -q -e -n
        fi
        "$MIGEMO_BIN" -c "$MIGEMO_SOCKET" -d "$MIGEMO_DICT" -q -e -n -w "$word"
    else
        "$MIGEMO_BIN" -d "$MIGEMO_DICT" -q -e -n -w "$word"
    fi
}

_migemo_complete_stop() {
    local pid

    local MIGEMO_DIR=@MIGEMO_DIR@
    local MIGEMO_BIN="$MIGEMO_DIR/cmigemo"
    local MIGEMO_DICT="$MIGEMO_DIR/dict/migemo-dict"
    local MIGEMO_SOCKET="$MIGEMO_DIR/socket"
    local MIGEMO_PID="$MIGEMO_DIR/socket.pid"

    pid=$(cat "$MIGEMO_PID" 2>/dev/null)
    if kill -0 "$pid" 2>/dev/null; then
        kill -TERM "$pid" && echo "stoped" || echo "stop failed"
        rm -f "$MIGEMO_SOCKET" "$MIGEMO_PID"
    else
        echo "not running"
    fi
}

_filedir() {

    local IFS=$'\n'
    local dir name files pattern require_dir

    if [[ $cur = */* ]]; then
        require_dir=1
        eval dir="${cur%/*}/"
        eval name="${cur##*/}"
    else
        dir="./"
        eval name="$cur"
    fi
    if [ ! -d "$dir" -o ! -x "$dir" ]; then
        unset COMPREPLY;
        return 1
    fi
    
    if [ "${1:-}" = -d ]; then
        files=$(cd "$dir" && find -L . -maxdepth 1 -type d 2>/dev/null | sed -n -e "s#^\./##p")
    else
        files=$(cd "$dir" && find -L . -maxdepth 1 2>/dev/null | sed -n -e "s#^\./##p")
    fi
    if [ -n "$name" ]; then
        pattern=$(_migemo_complete_query "$name" 2>/dev/null)
        files=$(echo "$files" | grep --color=never "^$pattern")
    fi
    if [ "$require_dir" = "1" -a -n "$files" ]; then
        files=$(echo "$files" | while read -r tmp; do
            echo "$dir$tmp"
        done)
    fi
    if [ -n "$files" ]; then
        compopt -o filenames 2> /dev/null;
        COMPREPLY=( ${COMPREPLY[@]:-} $files )
    fi

    return 0
}

_filedir_xspec() {
    _migemo_complete "$1" "$2"
}

_migemo_complete() {
    local cur=$2
    _filedir
}

_migemo_complete_dironly() {
    local cur=$2
    _filedir -d
}

complete -o filenames -F _migemo_complete_dironly  cd
complete -o filenames -D -F _migemo_complete
