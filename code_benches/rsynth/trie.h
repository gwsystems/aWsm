/* $Id: trie.h,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
*/
typedef struct trie_s *trie_ptr;

extern void trie_insert PROTO((trie_ptr *r,char *s,void *value));
extern void *trie_lookup PROTO((trie_ptr *r,char **sp));
