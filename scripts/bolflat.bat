cat verifile_lat | sed '1,$s/.=/=/' | sed '1,$s/^ *//' >zz1.c
fixbol.exe zz1.c >zz.c
cat zz.c | grep -v "^~" >yy1
cat zz.c | grep "^~" | sed '1,$s/=/= ~(/' | sed '1,$s/;/);/' | sed '1,$s/^~//' >yy2
cat yy1 yy2 | sed '1,$s/END/}/' | sed '1,$s/BEGIN//' | grep -v "}" >yy.c
echo #include "ra.h" >yyy.c
echo #include "stdio.h" >>yyy.c
echo void ra_lat() >>yyy.c
echo { >>yyy.c
echo int V[1000]; >>yyy.c
cat yy.c | grep -v module | grep -v always | sort -r >yy1.c
echo } >yy2
cat yyy.c  yy1.c  yy2 >zzx.c
fixlat zzx.c >zzz.c

