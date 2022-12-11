#!/bin/sh

THIS_DIR=$(dirname "$0")
cmake $THIS_DIR -B$THIS_DIR/build/ -GNinja