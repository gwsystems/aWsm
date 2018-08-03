#ifdef  JONS_PARWAVE 
#include "parwave.h"
extern klatt_frame_t def_pars;
extern klatt_global_t klatt_global;
extern int init_synth PROTO((int argc,char *argv[]));
#else
#include "nsynth.h"
#endif
