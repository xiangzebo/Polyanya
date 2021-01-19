#!/bin/bash
bin="${0%/*}/../bin"
$bin/gridmap2poly | $bin/poly2mesh | grep -e "^[^-* ].*"
