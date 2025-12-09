#include <sqlca.h>
#include <stdio.h>
int main(void) { if (sqlca.sqlcode != 0) puts("non-zero sqlca.sqlcode"); return(0); }
