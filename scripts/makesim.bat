cat fa.h | grep " O" >otrm
cat fa.h | grep " I" >itrm
gawk -f awkg <itrm >fa_i.c
gawk -f awkh <otrm > fa_o1.c
cat fa_o1.c | sed '1,$s/O/*O/' >fa_o.c
gawk -f awkf <fa.h >m4file
cat m4file zzz.c >m4in1
m4 m4in1 | sed '1d' | sed '1,$s/()/(int* fa0)/' >fa_lat.c
cat m4file zzznl.c >m4in2
m4 m4in2 | sed '1d' | sed '1,$s/()/(int* fa0)/' >fa_nl.c
echo #include "stdio.h" >fa_all.c
echo void fa_0(  >>fa_all.c
echo #include "fa_io.h" >>fa_all.c
echo            int* fa0 ) >>fa_all.c
echo { >>fa_all.c
echo #include "fa_i.c" >>fa_all.c
echo fa_nl(fa0); >>fa_all.c
echo fa_lat(fa0); >>fa_all.c
echo #include "fa_o.c" >>fa_all.c
echo } >>fa_all.c


 