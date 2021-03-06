/* This file generated by /home/cborntra/REPOS/ATLAS/build/..//tune/blas/gemm/txover.c
*/
#ifndef ATL_TXOVER_H
#define ATL_TXOVER_H 1

#define ATL_PDIM 2
static const int ATL_tmmNN_SQmnk_XO[2] =
{160,166};
static const int ATL_tmmNT_SQmnk_XO[2] =
{160,166};
static const int ATL_tmmTN_SQmnk_XO[2] =
{160,166};
#define ATL_tmmTT_SQmnk_XO ATL_tmmNN_SQmnk_XO
static const int *ATL_tmm_SQmnk_XO[4] =
{ATL_tmmNN_SQmnk_XO, ATL_tmmNT_SQmnk_XO,
ATL_tmmTN_SQmnk_XO, ATL_tmmTT_SQmnk_XO};
static const int ATL_tmmNN_SmnLk_XO[18] =
{4778,6715,2128,3353,1393,2337,450,1051,267,324,172,431,172,324,172,327,0,80};
static const int ATL_tmmNT_SmnLk_XO[18] =
{2668,4426,1467,1875,1467,2426,0,568,172,431,172,324,172,324,172,327,0,80};
static const int ATL_tmmTN_SmnLk_XO[18] =
{0,0,2830,4080,1729,5213,0,568,172,324,172,324,172,324,172,327,0,80};
#define ATL_tmmTT_SmnLk_XO ATL_tmmNN_SmnLk_XO
static const int *ATL_tmm_SmnLk_XO[4] =
{ATL_tmmNN_SmnLk_XO, ATL_tmmNT_SmnLk_XO,
ATL_tmmTN_SmnLk_XO, ATL_tmmTT_SmnLk_XO};
static const int ATL_tmmNN_SmkLn_XO[18] =
{2967,3691,1589,2618,1377,1551,0,630,172,324,172,324,172,324,172,327,80,172};
static const int ATL_tmmNT_SmkLn_XO[18] =
{2908,5435,1921,4235,1118,1620,475,614,172,324,172,324,172,324,172,327,80,172};
#define ATL_tmmTN_SmkLn_XO ATL_tmmNN_SmkLn_XO
#define ATL_tmmTT_SmkLn_XO ATL_tmmNT_SmkLn_XO
static const int *ATL_tmm_SmkLn_XO[4] =
{ATL_tmmNN_SmkLn_XO, ATL_tmmNT_SmkLn_XO,
ATL_tmmTN_SmkLn_XO, ATL_tmmTT_SmkLn_XO};
static const int ATL_tmmNN_SnkLm_XO[18] =
{0,0,6212,7500,1613,2741,522,661,172,324,172,324,172,324,172,327,80,172};
#define ATL_tmmNT_SnkLm_XO ATL_tmmNN_SnkLm_XO
static const int ATL_tmmTN_SnkLm_XO[18] =
{5826,8000,3513,4661,1323,1412,471,1180,172,324,172,324,172,324,172,327,80,172};
#define ATL_tmmTT_SnkLm_XO ATL_tmmTN_SnkLm_XO
static const int *ATL_tmm_SnkLm_XO[4] =
{ATL_tmmNN_SnkLm_XO, ATL_tmmNT_SnkLm_XO,
ATL_tmmTN_SnkLm_XO, ATL_tmmTT_SnkLm_XO};
static const int ATL_tmmNN_SmLnk_XO[18] =
{0,320,0,320,171,320,171,320,171,320,171,320,171,320,171,325,80,171};
static const int ATL_tmmNT_SmLnk_XO[18] =
{254,320,171,320,171,320,171,320,171,320,171,320,171,320,171,325,80,171};
static const int ATL_tmmTN_SmLnk_XO[18] =
{0,320,0,320,171,320,171,320,171,320,171,320,171,320,171,325,80,171};
static const int ATL_tmmTT_SmLnk_XO[18] =
{171,320,171,320,171,320,171,320,171,320,171,320,171,320,171,325,80,171};
static const int *ATL_tmm_SmLnk_XO[4] =
{ATL_tmmNN_SmLnk_XO, ATL_tmmNT_SmLnk_XO,
ATL_tmmTN_SmLnk_XO, ATL_tmmTT_SmLnk_XO};
static const int ATL_tmmNN_SnLmk_XO[18] =
{0,320,0,320,171,320,171,320,171,320,171,320,171,320,171,325,80,171};
static const int ATL_tmmNT_SnLmk_XO[18] =
{0,424,171,320,171,320,171,320,171,320,171,320,171,320,171,325,80,171};
static const int ATL_tmmTN_SnLmk_XO[18] =
{171,858,171,454,171,320,171,320,171,320,171,320,171,320,171,325,80,171};
static const int ATL_tmmTT_SnLmk_XO[18] =
{254,320,171,320,171,320,171,320,171,320,171,320,171,320,171,325,80,171};
static const int *ATL_tmm_SnLmk_XO[4] =
{ATL_tmmNN_SnLmk_XO, ATL_tmmNT_SnLmk_XO,
ATL_tmmTN_SnLmk_XO, ATL_tmmTT_SnLmk_XO};
static const int ATL_tmmNN_SkLmn_XO[18] =
{0,171,0,171,0,171,0,171,0,171,0,171,0,171,171,171,80,171};
static const int ATL_tmmNT_SkLmn_XO[18] =
{185,185,0,171,0,171,0,171,0,171,0,171,0,171,171,171,80,171};
static const int ATL_tmmTN_SkLmn_XO[18] =
{0,171,0,171,0,171,0,171,0,171,0,171,0,171,171,171,80,171};
#define ATL_tmmTT_SkLmn_XO ATL_tmmNN_SkLmn_XO
static const int *ATL_tmm_SkLmn_XO[4] =
{ATL_tmmNN_SkLmn_XO, ATL_tmmNT_SkLmn_XO,
ATL_tmmTN_SkLmn_XO, ATL_tmmTT_SkLmn_XO};

#endif /* end ifndef ATL_TXOVER_H */
