#include <bsdxml.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/param.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

int Deep = 0;
#define SZ_BUFFER    1024
#define SZ_BIGBUFFER 0xFFFF	

typedef struct buffer_t {
  unsigned char * data;
  unsigned len;
  unsigned size; 
} buffer_t;

typedef struct path_t {
  char * path;
  char * from;
  unsigned rev;
  char action;
  char kind;
  struct path_t * next;
} path_t;

typedef struct commit_t {
  unsigned rev;
  char date[20];
  char action[20];
  char kind[20];
  char author[20];
  int path_count;
  char * message;
  path_t * path;
  buffer_t data;
} commit_t;

enum act_t {
  ACT_FILE,
  ACT_COMMIT,
};

void free_data(buffer_t * p)
{
  if (p->size) {
    free(p->data);
    p->data = NULL;
    p->size = p->len = 0;
  }
}

commit_t _G = { .path = NULL };
int action;

char * getval(char * key,const char ** array)
{
  int i;
 
  for (i = 0; array[i]; i += 2) {
    if (strcmp(array[i], key) == 0)
      return (char *)array[i + 1];
  } 
  return NULL;
}

void
start(void *data, const char *el, const char **attr)
{
  char * val;
  path_t * p;


  if (strcmp(el, "logentry")==0) {
    while(_G.path) {
      p = _G.path;
      _G.path = p->next;

      if (p->path) {
        free(p->path);
      }
      if (p->from) {
        free(p->from);
      } 
      free(p); 
    }
    memset(&_G,0,sizeof(commit_t));

    val = getval("revision",attr);
    assert(val);
    _G.rev = atol(val);  

  } else
  if (strcmp(el, "path")==0) {
    _G.path_count++;

    p = malloc(sizeof(path_t));
    assert(p);
    memset(p,0,sizeof(path_t));
    p->next = _G.path;
    _G.path = p;

    val = getval("action",attr);
    assert(val);
    p->action = val[0];

    val = getval("kind",attr);
    assert(val);
    p->kind = val[0];

    if((val = getval("copyfrom-path",attr))) {
      assert(strlen(val) <= MAXPATHLEN);
      p->from = strdup(val);
    }
    
    if((val = getval("copyfrom-rev",attr))) {
      p->rev = atol(val);
    }
  }

  Deep++;
}

void
end(void *data, const char *el) 
{

  if(_G.data.len) {
   if (strcmp(el, "date") == 0) {
      assert(_G.data.len+1 < SZ_BUFFER);
      strncpy(_G.date, (char *)_G.data.data, _G.data.len+1);	\
      free_data(&_G.data);
    } else
    if (strcmp (el, "author") == 0) {
      assert(_G.data.len+1 < SZ_BUFFER);
      strncpy(_G.author, (char *)_G.data.data, _G.data.len+1);	\
      free_data(&_G.data);
    } else
    if (strcmp (el, "msg") == 0) {
      _G.message = (char *)_G.data.data;
      _G.data.len = _G.data.size = 0;
      _G.data.data = NULL;
    } else
    if (strcmp (el, "path") == 0) {
      _G.path->path = (char *)_G.data.data;
      _G.data.len = _G.data.size = 0;
      _G.data.data = NULL;
    }
  }

  if(action == ACT_FILE && strcmp(el, "path") == 0) {
    path_t * p = _G.path;
    printf("%c;%c;%s;%u;%s;%s;%d\n",
          p->kind,
          p->action,
          _G.author,
          _G.rev,
          p->path,
          p->from ? : "",
          p->rev);
  } else
  if(action == ACT_COMMIT && strcmp(el, "logentry") == 0) {
    printf("%d;%s;%s\n",
           _G.rev, _G.author, _G.date);
  }
  Deep--;
}

void
data(void *data, const XML_Char *s, int len) {
  unsigned char * ptr;
  int    count;

  if (len == 0 || (len == 1 && s[0] == '\n')) {
    return;
  }

  count = _G.data.size - _G.data.len;
  if (len >= count) {
    int al = MAX(SZ_BUFFER,len+1);
    if (_G.data.size) {
      _G.data.data = realloc(_G.data.data, al+_G.data.size);
    } else {
      _G.data.data = malloc(al);
    }
    _G.data.size += al;
    assert(_G.data.data);
  }

  ptr = _G.data.data + _G.data.len;
  _G.data.len += len;
  memcpy(ptr,s,len);
  ptr[len] = '\0';
	
}

void usage(char * bin) {
  printf("usage : %s <output> <file>\n"
         "  ouput: file|commit\n"
         "  file : svn xml history file\n",
         bin);
}

int main(int argc, char ** argv)
{
  FILE * f;
  char *buff;
  int len;
  XML_Parser p;

  if(argc != 2) {
    usage(basename(argv[0]));
    exit (-1);
  }

  if (strcmp(argv[1],"file")==0) {
    action = ACT_FILE;
  } else
  if (strcmp(argv[1],"commit") == 0) {
    action = ACT_COMMIT;
  } else {
    usage(basename(argv[0]));
    exit (-1);
  }

  p = XML_ParserCreate(NULL);
  XML_SetElementHandler(p, start, end);
  XML_SetCharacterDataHandler(p, data);
  buff = XML_GetBuffer(p, SZ_BIGBUFFER);

/*  f = fopen(argv[2],"rb");
  if(!f) {
    usage(basename(argv[0]));
    exit (-1);
  }
*/
  while((len = fread(buff,1,SZ_BIGBUFFER-1,stdin))>0){
    XML_ParseBuffer(p, len, len == 0);
    buff = XML_GetBuffer(p,SZ_BIGBUFFER);
  }
  /*fclose(f);*/
}
