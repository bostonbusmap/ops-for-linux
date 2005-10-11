#!/bin/sh
make LDFLAGS=-bind_at_load install="-o 0 -g 0"
