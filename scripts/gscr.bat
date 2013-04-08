cat tf.h | grep " O" >otrm
cat tf.h | grep " I" >itrm
gawk -f awkg <itrm >tf_i.c
gawk -f awkh <otrm >tf_o.c
gawk -f awkf <tf.h >m4file
cat m4file zzz.c >m4in1
m4 m4in1 | sed '1d' | sed '1,$s/()/(int* tf0)/' >tf_lat.c
cat m4file zzznl.c >m4in2
m4 m4in2 | sed '1d' | sed '1,$s/()/(int* tf0)/' >tf_nl.c

 