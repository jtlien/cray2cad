cat verifile_nl | grep -v "complement" | sed '1,$s/^ *//' | sed '1,$s/assign//'  | sed '1,$s/;/ ;/' >zz1nl.c
fixbol.exe zz1nl.c >zznl.c
cat zznl.c | grep -v "^~" >yynl1
cat zznl.c | grep "^~" | sed '1,$s/=/= ~(/' | sed '1,$s/;/);/' | sed '1,$s/^~//' >yynl2
cat yynl1 yynl2 | sed '1,$s/^ *END/}/' | sed '1,$s/BEGIN//' >yynl.c
echo #include "tf.h" >yyynl.c
echo #include "stdio.h" >>yyynl.c
echo void tf_nl() >>yyynl.c
echo { >>yyynl.c
cat yynl.c >>yyynl.c
echo } >>yyynl.c
cat yyynl.c | grep -v "module" | sed '1,$s/1.b//' >zzznl.c

