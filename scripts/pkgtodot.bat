cat tfpkg | cut -c7-10 >a1
cat tfpkg | cut -c11-14 >a2
cat tfpkg | cut -c15-18 >a3
cat tfpkg | cut -c19-22 >a4
cat a1 a2 a3 a4 | tr [a-z] [A-Z] | grep -v "\." | sort | uniq >a5
cat a5 | sed '1,$s/$/;/' >a55
cat tfcon | grep " I" | cut -c10-13 | sort | uniq >it
cat it | sed '1,$s/^I/int I/' | sed '1,$s/$/;/' >a6
echo int ZZI; >a7
echo int ZZO; >>a7
cat a6 a7 a55 >tf.h



