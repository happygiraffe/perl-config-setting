#define BUFSIZE 255

#define MAX_PTYPE 20

#define POST 1
#define MAIL 2

#define FULLMSG 1
#define HEADERS 2

#define MAXSUBJECTLEN 32
#define MAXAREALEN 16
#define MSGIDLEN 8
#define TYPELEN 4

struct header_struct {
  char *subject;
  char *area;
  char *author;
  char *msgid;
};
