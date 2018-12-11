#!/bin/bash

nargs=$#
# Arguments should be even and >= 4
if [ $nargs -lt 3 ]
then
    echo "Usage: $0 [output] title file label [file label ...]"
    exit 0
fi

if [ $((nargs%2)) -eq 0 ]
then
    output=$1
    shift
    printf "set output '$output'\n"

    extension="${output##*.}"
    if [ "$extension" = "png" ]; then
        printf "set terminal pngcairo size 1200,900 linewidth 3\n"
    elif [ "$extension" = "pdf" ]; then
        printf "set terminal pdf size 12,9 linewidth 3\n"
    else
        echo "Extension $extension not supported"
        exit 1
    fi
fi

printf "set key font \",60\"\n"
printf "set rmargin 5\n"
printf "set xtics font \", 18\" 10000\n"
printf "set format x \"%%.0t*10^%%T\"\n"
printf "set ytics font \", 18\"\n"
printf "set yrange [0<*:]\n"
printf "set style fill transparent solid 0.4 noborder\n"
printf "set colorsequence podo\n"
printf "set key Left left center samplen 1.25\n"

title=$1
shift

# title and file setup
printf "set title \"$title\"\n"

COUNTER=1

# Setup for required first file (filename, title)
printf "plot '$1' using 1:3 with line smooth csplines title '$2' ls $COUNTER, \\"
printf "\n   '$1' using 1:(\$3-\$5):(\$3+\$5) every 99::0 with filledcurves ls $COUNTER notitle"
shift
shift

# Iterate over all remaining arguments
while [ $# -ne 0 ]
do
    COUNTER=$((COUNTER+1))
    printf ", \\"
    printf "\n   '$1' using 1:3 with line smooth csplines title '$2' ls $COUNTER, \\"
    printf "\n   '$1' using 1:(\$3-\$5):(\$3+\$5) every 99::0 with filledcurves ls $COUNTER notitle"
    shift
    shift
done

# Remember to launch gnuplot with the -p option to see the plots!
printf "\n"
