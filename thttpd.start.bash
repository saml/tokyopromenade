#!/bin/bash

thttpd -C thttpd.conf -l "$PWD/thttpd.log" "$@"
