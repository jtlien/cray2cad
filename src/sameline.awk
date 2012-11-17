
BEGIN {

  prevfirst = "..."
  cnt = 1
  }

{
    if ($1 == prevfirst)
    { 
	backarray[cnt] = $2
        cnt = cnt+1
    }
    else
    {
       
        printf "%s -", prevfirst
	for(i=1; i < cnt ; i=i+1)
	{    
	    printf " %s" , backarray[i]
	}
       backarray[1] = $2
       cnt = 2
        
	printf " . \n"
     }
    prevfirst = $1
    
}
END {

}
