#include <stdio.h>
#include <time.h>

#define ERRLOGFILE "/home/alex/arse"

int logerr(char *error, char *extra) {

  FILE *fp;
  time_t t;

  /* Don't try to log this error, that could get nasty. */
  if ((fp=fopen(ERRLOGFILE, "at"))==NULL) 
    printf("Bugger");

   time(&t);

  fprintf(fp, "%s  %s - %s\n", ctime(&t), error, extra);
# ifdef DEBUG
    printf("%s  %s - %s\n", ctime(&t), error, extra);
# endif

  fclose(fp);
  return(1);
}
