#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,148,271,394,518,1012,2000
 * N : 25,148,271,394,518,1012,2000
 * NB : 12,24,24,24,48,72,120
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 86) (nb_) = 12; \
   else if ((n_) < 456) (nb_) = 24; \
   else if ((n_) < 765) (nb_) = 48; \
   else if ((n_) < 1506) (nb_) = 72; \
   else (nb_) = 120; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */
