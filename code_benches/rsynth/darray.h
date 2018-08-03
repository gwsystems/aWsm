/* $Id: darray.h,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
*/
#if !defined(DARRAY_H)
#define DARRAY_H
typedef struct
 {char     *data;          /* the items */
  unsigned items;          /* number of slots used */
  unsigned alloc;          /* number of slots allocated */
  unsigned short esize;    /* size of items */
  unsigned short get;      /* number to get */
 } darray_t, *darray_ptr;

/* return pointer to nth item */
extern void *Darray_find PROTO((darray_t *a,unsigned n));
/* delete nth item */
extern int darray_delete PROTO((darray_t *a,unsigned n));
extern void darray_free  PROTO((darray_t *a));

#if defined(__GNUC__)
static inline void darray_init(darray_t *a,unsigned size,unsigned get)
{
 a->esize = size;
 a->get   = get;
 a->items = a->alloc = 0;
 a->data = NULL;
}

static inline void *darray_find(darray_t *a,unsigned n)
{
 if (n < a->alloc && n < a->items)
  return (void *) (a->data + n * a->esize);
 return Darray_find(a,n);
}
#else

#define darray_init(a,sz,gt) \
 ((a)->esize = (sz), (a)->get = (gt), (a)->items = (a)->alloc = 0, (a)->data = NULL)

#define darray_find(a,n) \
 (((n) < (a)->alloc && (n) < (a)->items) \
   ? (void *) ((a)->data + (n) * (a)->esize)  \
   : Darray_find(a,n))

#endif 
#endif

