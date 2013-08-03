#!/bin/bash

pid="$(cat thttpd.pid)" 
kill "$pid" || { echo "failed to kill $pid"; exit 1; }
echo "killed $pid"

