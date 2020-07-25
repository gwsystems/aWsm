#include <config.h>
/* $Id: saynum.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
 */
char *saynum_id = "$Id: saynum.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $";

#include <stdio.h>
#include "proto.h"
#include "darray.h"
#include "say.h"

/*
   **              Integer to Readable ASCII Conversion Routine.
   **
   ** Synopsis:
   **
   **      say_cardinal(value)
   **              long int     value;          -- The number to output
   **
   **      The number is translated into a string of words
   **
 */
static char *Cardinals[] =
{
 "zero", "one", "two", "three",
 "four", "five", "six", "seven",
 "eight", "nine",
 "ten", "eleven", "twelve", "thirteen",
 "fourteen", "fifteen", "sixteen", "seventeen",
 "eighteen", "nineteen"
};


static char *Twenties[] =
{
 "twenty", "thirty", "forty", "fifty",
 "sixty", "seventy", "eighty", "ninety"
};


static char *Ordinals[] =
{
 "zeroth", "first", "second", "third",
 "fourth", "fifth", "sixth", "seventh",
 "eighth", "ninth",
 "tenth", "eleventh", "twelfth", "thirteenth",
 "fourteenth", "fifteenth", "sixteenth", "seventeenth",
 "eighteenth", "nineteenth"
};


static char *Ord_twenties[] =
{
 "twentieth", "thirtieth", "fortieth", "fiftieth",
 "sixtieth", "seventieth", "eightieth", "ninetieth"
};

/*
   ** Translate a number to phonemes.  This version is for CARDINAL numbers.
   **       Note: this is recursive.
 */
unsigned
xlate_cardinal(value, phone)
long int value;
darray_ptr phone;
{
 unsigned nph = 0;
 if (value < 0)
  {
   nph += xlate_string("minus", phone);
   value = (-value);
   if (value < 0)                 /* Overflow!  -32768 */
    {
     nph += xlate_string("a lot", phone);
     return nph;
    }
  }
 if (value >= 1000000000L)
  /* Billions */
  {
   nph += xlate_cardinal(value / 1000000000L, phone);
   nph += xlate_string("billion", phone);
   value = value % 1000000000;
   if (value == 0)
    return nph;                   /* Even billion */
   if (value < 100)
    nph += xlate_string("and", phone);
   /* as in THREE BILLION AND FIVE */
  }
 if (value >= 1000000L)
  /* Millions */
  {
   nph += xlate_cardinal(value / 1000000L, phone);
   nph += xlate_string("million", phone);
   value = value % 1000000L;
   if (value == 0)
    return nph;                   /* Even million */
   if (value < 100)
    nph += xlate_string("and", phone);
   /* as in THREE MILLION AND FIVE */
  }

 /* Thousands 1000..1099 2000..99999 */
 /* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
 if ((value >= 1000L && value <= 1099L) || value >= 2000L)
  {
   nph += xlate_cardinal(value / 1000L, phone);
   nph += xlate_string("thousand", phone);
   value = value % 1000L;
   if (value == 0)
    return nph;                   /* Even thousand */
   if (value < 100)
    nph += xlate_string("and", phone);
   /* as in THREE THOUSAND AND FIVE */
  }
 if (value >= 100L)
  {
   nph += xlate_string(Cardinals[value / 100], phone);
   nph += xlate_string("hundred", phone);
   value = value % 100;
   if (value == 0)
    return nph;                   /* Even hundred */
  }
 if (value >= 20)
  {
   nph += xlate_string(Twenties[(value - 20) / 10], phone);
   value = value % 10;
   if (value == 0)
    return nph;                   /* Even ten */
  }
 nph += xlate_string(Cardinals[value], phone);
 return nph;
}

/*
   ** Translate a number to phonemes.  This version is for ORDINAL numbers.
   **       Note: this is recursive.
 */
unsigned
xlate_ordinal(value, phone)
long int value;
darray_ptr phone;
{
 unsigned nph = 0;
 if (value < 0)
  {
   nph += xlate_string("minus", phone);
   value = (-value);
   if (value < 0)                 /* Overflow!  -32768 */
    {
     nph += xlate_string("a lot", phone);
     return nph;
    }
  }
 if (value >= 1000000000L)
  /* Billions */
  {
   nph += xlate_cardinal(value / 1000000000L, phone);
   value = value % 1000000000;
   if (value == 0)
    {
     nph += xlate_string("billionth", phone);
     return nph;                  /* Even billion */
    }
   nph += xlate_string("billion", phone);
   if (value < 100)
    nph += xlate_string("and", phone);
   /* as in THREE BILLION AND FIVE */
  }

 if (value >= 1000000L)
  /* Millions */
  {
   nph += xlate_cardinal(value / 1000000L, phone);
   value = value % 1000000L;
   if (value == 0)
    {
     nph += xlate_string("millionth", phone);
     return nph;                  /* Even million */
    }
   nph += xlate_string("million", phone);
   if (value < 100)
    nph += xlate_string("and", phone);
   /* as in THREE MILLION AND FIVE */
  }

 /* Thousands 1000..1099 2000..99999 */
 /* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
 if ((value >= 1000L && value <= 1099L) || value >= 2000L)
  {
   nph += xlate_cardinal(value / 1000L, phone);
   value = value % 1000L;
   if (value == 0)
    {
     nph += xlate_string("thousandth", phone);
     return nph;                  /* Even thousand */
    }
   nph += xlate_string("thousand", phone);
   if (value < 100)
    nph += xlate_string("and", phone);
   /* as in THREE THOUSAND AND FIVE */
  }
 if (value >= 100L)
  {
   nph += xlate_string(Cardinals[value / 100], phone);
   value = value % 100;
   if (value == 0)
    {
     nph += xlate_string("hundredth", phone);
     return nph;                  /* Even hundred */
    }
   nph += xlate_string("hundred", phone);
  }
 if (value >= 20)
  {
   if ((value % 10) == 0)
    {
     nph += xlate_string(Ord_twenties[(value - 20) / 10], phone);
     return nph;                  /* Even ten */
    }
   nph += xlate_string(Twenties[(value - 20) / 10], phone);
   value = value % 10;
  }
 nph += xlate_string(Ordinals[value], phone);
 return nph;
}
