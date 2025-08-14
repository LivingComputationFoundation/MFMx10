#!/usr/bin/gnuplot
STARTX="2025-06-10 00:00:00"
DAYSPERTIC=7
INPUT = "../../bin/riscv10-builds.dat"

THRESHOLD=2*60*60*24
#PREV_X=1750709150
PREV_X=1

set terminal pngcairo  background "black" enhanced fontscale 1.8 size 1000, 1000
set output 'size-plot.png'

set border lc rgb "yellow"
set tics textcolor rgb "yellow"

set xdata time
set timefmt "%s"
set format x "%d-%b-%y" 

set format y "%4.0f"

set rmargin 5

set ylabel "MFMx10 code size (KiBytes)" textcolor rgb "yellow"
set xtics STARTX, DAYSPERTIC*24*60*60
#set xrange [ STARTX , * ]
#set tics scale 1
unset key
## Last datafile plotted: "../../bin/riscv10-builds.dat"
#plot INPUT using 1:($5/1024) w lp lw 3 pointsize 3
plot INPUT using (CURRENT_X = column(1), \
     (abs(CURRENT_X - PREV_X) <= THRESHOLD ? (PREV_X = CURRENT_X, CURRENT_X) : (PREV_X = CURRENT_X, 1/0)) \
    ):($5/1024) w lp lw 3 pointsize 3
#    EOF

set output 'size-plot-1M.png'
set yrange [ 0 : 1000]
plot INPUT using (CURRENT_X = column(1), \
     (abs(CURRENT_X - PREV_X) <= THRESHOLD ? (PREV_X = CURRENT_X, CURRENT_X) : (PREV_X = CURRENT_X, 1/0)) \
    ):($5/1024) w lp lw 3 pointsize 3
