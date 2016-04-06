/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File        : c2_11pf.c
*      Purpose     : Searches a 11 bit algebraic codebook containing 2 pulses
*                    in a frame of 40 samples.
*
*****************************************************************************
*/

/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/
#include "c2_11pf.h"

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
#include "codec_op_etsi.h"
#include "codec_op_vec.h"

#include "inv_sqrt.h"
#include "cnst.h"
#include "cor_h.h"
#include "set_sign.h"

/*
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES
*****************************************************************************
*/
#define NB_PULSE  2

#include "c2_11pf_tab.c"

/*
*****************************************************************************
*                         DECLARATION OF PROTOTYPES
*****************************************************************************
*/
static void search_2i40(
    Word16 dn[],        /* i : correlation between target and h[]            */
    Word16 rr[][L_CODE],/* i : matrix of autocorrelation                     */
    Word16 codvec[]     /* o : algebraic codebook vector                     */
);

static Word16 build_code(
    Word16 codvec[],    /* i : algebraic codebook vector                     */
    Word16 dn_sign[],   /* i : sign of dn[]                                  */
    Word16 cod[],       /* o : algebraic (fixed) codebook excitation         */
    Word16 h[],         /* i : impulse response of weighted synthesis filter */
    Word16 y[],         /* o : filtered fixed codebook excitation            */
    Word16 sign[]       /* o : sign of 2 pulses                              */
);

/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:  code_2i40_11bits()
 *
 *  PURPOSE:  Searches a 11 bit algebraic codebook containing 2 pulses
 *            in a frame of 40 samples.
 *
 *  DESCRIPTION:
 *    The code length is 40, containing 2 nonzero pulses: i0...i1.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    Pulse i0 can have 2x8=16 possible positions, pulse i1 can have
 *    4x8=32 positions.
 *
 *       i0 :  1, 6, 11, 16, 21, 26, 31, 36.
 *             3, 8, 13, 18, 23, 28, 33, 38.
 *       i1 :  0, 5, 10, 15, 20, 25, 30, 35.
 *             1, 6, 11, 16, 21, 26, 31, 36.
 *             2, 7, 12, 17, 22, 27, 32, 37.
 *             4, 9, 14, 19, 24, 29, 34, 39.
 *
 *************************************************************************/
Word16 code_2i40_11bits(
    Word16 x[],         /* i : target vector                                 */
    Word16 h[],         /* i : impulse response of weighted synthesis filter */
                        /*     h[-L_subfr..-1] must be set to zero.          */
    Word16 T0,          /* i : Pitch lag                                     */
    Word16 pitch_sharp, /* i : Last quantized pitch gain                     */
    Word16 code[],      /* o : Innovative codebook                           */
    Word16 y[],         /* o : filtered fixed codebook excitation            */
    Word16 * sign       /* o : Signs of 2 pulses                             */
)
{
    Word16 codvec[NB_PULSE];
    Word16 dn[L_CODE], dn2[L_CODE], dn_sign[L_CODE];
    Word16 rr[L_CODE][L_CODE];
    Word16 index, sharp;

    sharp = shl(pitch_sharp, 1);

    /*if (sub(T0, L_CODE) < 0)*/
    if ( T0 < (Word16)L_CODE )
    {
       CODEC_OpVcMultQ15Add(&h[T0],
                          &h[0],
                          L_CODE - T0,
                          sharp,
                          &h[T0]);
    }

    cor_h_x(h, x, dn, 1);
    set_sign(dn, dn_sign, dn2, 8); /* dn2[] not used in this codebook search */
    cor_h(h, dn_sign, rr);
    search_2i40(dn, rr, codvec);
                                     /* function result */
    index = build_code(codvec, dn_sign, code, h, y, sign);

  /*-----------------------------------------------------------------*
  * Compute innovation vector gain.                                 *
  * Include fixed-gain pitch contribution into code[].              *
  *-----------------------------------------------------------------*/

    /*if (sub(T0, L_CODE) < 0)*/
    if ( T0 < (Word16)L_CODE )
    {
       CODEC_OpVcMultQ15Add(&code[T0],
                          &code[0],
                          L_CODE - T0,
                          sharp,
                          &code[T0]);
    }
    return index;
}

/*
*****************************************************************************
*                         PRIVATE PROGRAM CODE
*****************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION  search_2i40()
 *
 *  PURPOSE: Search the best codevector; determine positions of the 2 pulses
 *           in the 40-sample frame.
 *
 *************************************************************************/

#define _1_2    (Word16)(32768L/2)
#define _1_4    (Word16)(32768L/4)
#define _1_8    (Word16)(32768L/8)
#define _1_16   (Word16)(32768L/16)

static void search_2i40(
    Word16 dn[],         /* i : correlation between target and h[] */
    Word16 rr[][L_CODE], /* i : matrix of autocorrelation          */
    Word16 codvec[]      /* o : algebraic codebook vector          */
)
{
    Word16 i0, i1;
    Word16 ix = 0; /* initialization only needed to keep gcc silent */
    Word16 track1, track2, ipos[NB_PULSE];
    Word16 psk, ps0, ps1, sq, sq1;
    Word16 alpk, alp, alp_16;
    Word32 s, alp0, alp1;
    Word16 i;

    psk = -1;
    alpk = 1;
    for (i = 0; i < NB_PULSE; i++)
    {
       codvec[i] = i;
    }

    /*------------------------------------------------------------------*
     * main loop: try 2x4  tracks.                                      *
     *------------------------------------------------------------------*/

    for (track1 = 0; track1 < 2; track1++)
    {
       for (track2 = 0; track2 < 4; track2++)
       {
          /* fix starting position */
          ipos[0] = startPos1[track1];
          ipos[1] = startPos2[track2];

          /*----------------------------------------------------------------*
           * i0 loop: try 8 positions.                                      *
           *----------------------------------------------------------------*/
           /* account for ptr. init. (rr[io]) */
          for (i0 = ipos[0]; i0 < L_CODE; i0 += STEP)
          {
             ps0 = dn[i0];
             alp0 = L_mult(rr[i0][i0], _1_4);

             /*-------------------------------------------------------------*
              * i1 loop: 8 positions.                                       *
              *-------------------------------------------------------------*/

             sq = -1;
             alp = 1;
             ix = ipos[1];

             /*---------------------------------------------------------------*
              * These index have low complexity address computation because   *
              * they are, in fact, pointers with fixed increment. For example,*
              * "rr[i0][i2]" is a pointer initialized to "&rr[i0][ipos[2]]"   *
              * and incremented by "STEP".                                    *
              *---------------------------------------------------------------*/

              /* account for ptr. init. (rr[i1]) */
              /* account for ptr. init. (dn[i1]) */
              /* account for ptr. init. (rr[io]) */
             for (i1 = ipos[1]; i1 < L_CODE; i1 += STEP) {
                ps1 = add(ps0, dn[i1]);   /* idx increment = STEP */

                /* alp1 = alp0 + rr[i0][i1] + 1/2*rr[i1][i1]; */

                alp1 = L_mac(alp0, rr[i1][i1], _1_4); /* idx incr = STEP */
                alp1 = L_mac(alp1, rr[i0][i1], _1_2); /* idx incr = STEP */

                sq1 = mult(ps1, ps1);

                alp_16 = round(alp1);

                s = L_msu(L_mult(alp, sq1), sq, alp_16);

                if (s > 0)
                {
                   sq = sq1;
                   alp = alp_16;
                   ix = i1;
                }
             }

             /*---------------------------------------------------------------*
              * memorise codevector if this one is better than the last one.  *
              *---------------------------------------------------------------*/

             s = L_msu(L_mult(alpk, sq), psk, alp);

             if (s > 0)
             {
                psk = sq;
                alpk = alp;
                codvec[0] = i0;
                codvec[1] = ix;
             }
          }
       }
    }
    return;
}

/*************************************************************************
 *
 *  FUNCTION:  build_code()
 *
 *  PURPOSE: Builds the codeword, the filtered codeword and index of the
 *           codevector, based on the signs and positions of 2 pulses.
 *
 *************************************************************************/

static Word16 build_code(
    Word16 codvec[],  /* i : position of pulses                            */
    Word16 dn_sign[], /* i : sign of pulses                                */
    Word16 cod[],     /* o : innovative code vector                        */
    Word16 h[],       /* i : impulse response of weighted synthesis filter */
    Word16 y[],       /* o : filtered innovative code                      */
    Word16 sign[]     /* o : sign of 2 pulses                              */
)
{
    Word16 i, j, k, track, index, _sign[NB_PULSE], indx, rsign;
    Word16 *p0, *p1;
    Word32 s;

    CODEC_OpVecSet(&cod[0], L_CODE, 0);

    indx = 0;
    rsign = 0;
    for (k = 0; k < NB_PULSE; k++)
    {
        i = codvec[k];      /* read pulse position */
        j = dn_sign[i];     /* read sign           */

        index = mult(i, 6554);    /* index = pos/5 */
                                  /* track = pos%5 */
        track = sub(i, extract_l(L_shr(L_mult(index, 5), 1)));

        /*if (sub(track, 0) == 0)*/
        if ( 0 == track )
        {
           track = 1;
           index = shl(index, 6);
        }
        /*else if (sub(track, 1) == 0)*/
        else if ( 1 == track )
        {
           /*if (sub(k, 0) == 0)*/
           if ( 0 == k )
           {
              track = 0;
              index = shl(index, 1);
           }
           else
           {
              track = 1;
              index = add(shl(index, 6), 16);
           }
        }
        /*else if (sub(track, 2) == 0)*/
        else if ( 2 == track )
        {
           track = 1;
           index = add(shl(index, 6), 32);
        }
        /*else if (sub(track, 3) == 0)*/
        else if ( 3 == track )
        {
           track = 0;
           index = add(shl(index, 1), 1);
        }
        /*else if (sub(track, 4) == 0)*/
        else if ( 4 == track )
        {
           track = 1;
           index = add(shl(index, 6), 48);
        }

        if (j > 0)
        {
           cod[i] = 8191;
           _sign[k] = 32767;
           rsign = add(rsign, shl(1, track));
        }
        else
        {
           cod[i] = -8192;
           _sign[k] = (Word16) - 32768L;
        }

        indx = add(indx, index);
    }
    *sign = rsign;

    p0 = h - codvec[0];
    p1 = h - codvec[1];

    for (i = 0; i < L_CODE; i++)
    {
        s = 0;
        s = L_mac(s, *p0++, _sign[0]);
        s = L_mac(s, *p1++, _sign[1]);
        y[i] = round(s);
    }

    return indx;
}