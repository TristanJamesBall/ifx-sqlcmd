#include <sqlhdr.h>
#include <sqliapi.h>
#line 1 "esqlc-1576054.ec"
#include <sqlca.h>
#include <stdio.h>
int main(void) { if (sqlca.sqlcode != 0) puts("non-zero sqlca.sqlcode"); return(0); }

#line 3 "esqlc-1576054.ec"
