#ifndef ATL_cGetNB_geqrf

/*
 * NB selection for GEQRF: Side='RIGHT', Uplo='UPPER'
 * M : 25,240,320,400,560,1120,2240
 * N : 25,240,320,400,560,1120,2240
 * NB : 5,20,80,80,80,80,240
 */
#define ATL_cGetNB_geqrf(n_, nb_) \
{ \
   if ((n_) < 132) (nb_) = 5; \
   else if ((n_) < 280) (nb_) = 20; \
   else if ((n_) < 1680) (nb_) = 80; \
   else (nb_) = 240; \
}


#endif    /* end ifndef ATL_cGetNB_geqrf */