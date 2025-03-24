#!/bin/bash
bunzip2 -c $1.bz2 | gzip > $1.gz

