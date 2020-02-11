#include <config.h>
/* $Id: trie.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
 */
char *trie_id = "$Id: trie.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $";
#include <useconfig.h>
#include <stdio.h>
#include "proto.h"
#include "trie.h"

struct trie_s
 {
  struct trie_s *otherwise;
  struct trie_s *more;
  void *value;
  char ch;
 };

void
trie_insert(r, s, value)
trie_ptr *r;
char *s;
void *value;
{
 trie_ptr p = NULL;
 char ch;
 while ((ch = *s++))
  {
   while ((p = *r))
    {
     if (p->ch == ch)
      break;
     else
      r = &p->otherwise;
    }
   if (!p)
    {
     p = (trie_ptr) malloc(sizeof(*p));
     memset(p, 0, sizeof(*p));
     p->ch = ch;
     *r = p;
    }
   r = &p->more;
  }
 p->value = value;
}

void *
trie_lookup(r, sp)
trie_ptr *r;
char **sp;
{
 char *s = *sp;
 char *value = NULL;
 char ch;
 while ((ch = *s))
  {
   trie_ptr *l = r;
   trie_ptr p;
   while ((p = *l))
    {
     if (p->ch == ch)
      break;
     else
      l = &p->otherwise;
    }
   if (p)
    {
     *l = p->otherwise;
     p->otherwise = *r;
     *r = p;
     r = &p->more;
     value = (char *) p->value;
     s++;
    }
   else
    break;
  }
 *sp = s;
 return value;
}
