#ifndef ATL_zGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,96,240,528,1056,2112
 * N : 25,96,240,528,1056,2112
 * NB : 9,48,48,48,48,48
 */
#define ATL_zGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 60) (nb_) = 9; \
   else (nb_) = 48; \
}


#endif    /* end ifndef ATL_zGetNB_geqrf */