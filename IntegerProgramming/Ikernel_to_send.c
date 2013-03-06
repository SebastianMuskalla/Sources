/*---------------------------------------------------------------------------
/                         Integral Kernel using LLL Algorithm               |
/                                                                           |
/                           Implemented by                                  |
/            Alfredo S�nchez-Navarro and Alberto Vigneron-Tenorio           |
/            alfredo.sanchez@uca.es  alberto.vigneron@uca.es,               |
/                 E.U.E.Empresariales de Jerez.                             |
/                   Universidad de C�diz                                    |
/                   Porvera 54.                                             |
/                 Jerez de la Frontera (C�diz, Spain)                       |
/                                                                           |
/                                                                           |
/ This application computes the LLL-reduced basis of the Z-kernel of a      |
/ matrix using the algorithm 2.7.2 appeared in:                             |
/ Cohen, H. A course in computational algebraic number theory.              |
/ GTM 138, Springer 1996.                                                   |
/                                                                           |
/ The Input is a file with the structure:                                   |
/ Number of coordinates, Number of vectors,                                 |
/ b11 b12 b13 ... b1C                                                       |
/ b21 b22 b23 ... b2C                                                       |
/       ...                                                                 |
/ bF1 bF2 bF3 ... bFC                                                       |
/                                                                           |
/ All these inputs are integer numbers.                                     |
/                                                                           |
/ The Output is a LLL-reduced basis of the kernel of the matrix            |
/ b11 b21 b31 ... bF1                                                       |
/ b12 b22 b32 ... bF2                                                       |
/       ...                                                                 |
/ b1C b2C bC3 ... bFC                                                       |
/                                                                           |
/                                                                           |
/ All these outputs are integer numbers.                                    |
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
| EXAMPLE:                                                                  |
|                                                                           |
| Let prf be the file:                                                      |
| 4,3                                                                       |
| 1 -3 2                                                                    |
| 1 0 -1                                                                    |
| 2 -4 2                                                                    |
| 4 -7 3                                                                    |
| then:                                                                     |
|                                                                           |
| ./ikernel <prf                                                            |
| Output:                                                                   |
|  -4  -2  3  0                                                             |
|  -9  -5  5  1                                                             |
|                                                                           |
| The output is a LLL-reduced basis of the Z-kernel of ZxZxZxZ generated by |
| (-4,-2,3,0), (-9,-5,5,1)                                                  |
|                                                                           |
---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <new>
#include <gmp.h>

#define llln_max(a,b) (a > b ? a : b)

/* Global variables */
mpz_t *llln_d;
int *llln_f;
mpz_t **llln_lambda;
mpz_t **llln_b;
mpz_t **llln_H;

mpz_t **llln_proc_entrada (int *F, int *C);
void llln_swapk (int k, int kmax, int C, int F);
void llln_redi (int k, int l, int C, int F);
void llln_test (int *k, int kma, int C, int n);
void llln_producto (mpz_t *a, mpz_t *f, int n, mpz_t *prod);
//int llln_max (int a, int b);

void main (void)
{
   int i, j, k, kmax, r;
   int F,C;
  mpz_t t;
   mpz_t prod;
   mpz_t u;
   mpz_t auxi1;
   mpz_t auxi2;

   mpz_init (prod);
   mpz_init (u);
   mpz_init (auxi1);
   mpz_init (auxi2);
 mpz_init (t);

   k = 2;
   kmax = 1;

   llln_b=llln_proc_entrada(&F,&C);

   llln_H=new (mpz_t *)[F+1];
   for (i=1; i<=F; i++) {
      llln_H[i]=new mpz_t[F+1];
      for (j=1; j<F+1; j++) {
         mpz_init (llln_H[i][j]);
         if (i==j)
            mpz_set_ui (llln_H[i][j],1);
     else
            mpz_set_ui (llln_H[i][j],0);
      }
   }


   llln_f=new int[F+1];

   llln_d=new mpz_t[F+1];
   llln_lambda=new (mpz_t *)[F+1];
   mpz_init (llln_d[0]);
   for (i=1; i<=F; i++) {
      mpz_init (llln_d[i]);
      llln_lambda[i]=new mpz_t[F+1];
      for (j=1; j<=F; j++)
         mpz_init(llln_lambda[i][j]);
   }

   mpz_set_ui (llln_d[0],1);
   llln_producto (llln_b[1],llln_b[1], C, &prod);
 mpz_set (t, prod);

 if (mpz_cmp_ui (t,0) != 0 ) {
  mpz_set (llln_d[1], t);
  llln_f[1]=1;
 }
 else {
  mpz_set_ui (llln_d[1],1);
  llln_f[1]=0;
 }

 while (k <= F) {
      if (k > kmax) {
         kmax = k;
       for (j=1;j<=k;j++) {
      if (llln_f[j]==0 & j<k)
               mpz_set_ui (llln_lambda[k][j], 0);
    else {
               llln_producto (llln_b[k],llln_b[j], C, &prod);
               mpz_set (u, prod);

             for (i=1; i<=j-1; i++) {
      if (llln_f[i] != 0) {
         mpz_mul (auxi1, llln_d[i], u);
                    mpz_mul (auxi2, llln_lambda[k][i], llln_lambda[j][i]);
                    mpz_sub (u, auxi1, auxi2);
                    mpz_tdiv_q ( u, u, llln_d[i-1]);
      }
             }

     if (j<k) {
        mpz_set (llln_lambda[k][j], u);
         }
             else {
        if (mpz_sgn (u) == 0) {
         mpz_set (llln_d[k], llln_d[k-1]);
       llln_f[k]=0;
      }
      else {
         mpz_set (llln_d[k], u);
       llln_f[k]=1;
      }
         }
    }
         }
      }
      llln_test (&k, kmax, C, F);
      for (i=k-2; i>0; i--) {
     if (llln_f[i] != 0)
            llln_redi (k,i,C,F);
    }
      k = k+1;
   }


 r=0;
   i=1;
 while (llln_f[i] == 0 & i<=F)
  r = i++;

   for (i=1; i<=r; i++) {
  printf("\n");
      for (j=1; j<=F; j++) {
     fprintf(stdout, " ");
         mpz_out_str (stdout, 0, llln_H[i][j]);
   fprintf(stdout, " ");
  }
      printf ("\n");
   }
   mpz_clear (auxi1);
   mpz_clear (auxi2);
   mpz_clear (u);
   mpz_clear (prod);
 mpz_clear (t);

 for (i = 1; i <= F; i++)
  {
    mpz_clear (llln_d[i]);
    for (j = 1; j <= F; j++)
      mpz_clear (llln_lambda[i][j]);
   mpz_clear (llln_H[i][j]);
    delete[]llln_lambda[i];
  delete[]llln_H[i];
  }
  delete[]llln_lambda;
 delete[]llln_H;
  delete[]llln_d;
 delete[]llln_f;
  for (i = 1; i < F + 1; i++)
  {
    for (j = 1; j < C + 1; j++)
       mpz_clear (llln_b[i][j]);
    delete[]llln_b[i];
  }
  delete[]llln_b;
}

/*---------------------------------------------------------------------------
| proc_entrada {}:                             |
| Read the input file.                           |
|___________________________________________________________________________|*/

mpz_t **llln_proc_entrada (int *F, int *C) /* Inputs */
{
  int numF, numC;
  int i, j, c;
  mpz_t **base;
  mpz_t valor;
  char *aux, caux;
  int tamano=100;

  aux=new char[tamano];
  int res = EOF;
  do 
  {
    res = fscanf (stdin, "%i, %i\n", &numF, &numC);
  } while( (res == EOF) and (errno == EINTR));
  base = new (mpz_t *)[numF+1];
  for (i = 1; i <= numF; i++) {
    base[i] = new mpz_t[numC+1];
    for (j = 1; j <= numC; j++) {
      do {
        caux=getc (stdin);
      }
      while (!feof(stdin) && !(caux=='-' || isdigit(caux)));
      mpz_init (base[i][j]);
      c=0;
      if (!feof(stdin)) {
        do {
          aux[c++]=caux;
          if (c==tamano) {
            char *nuevo=new char[3*tamano/2];
            memcpy (nuevo, aux, tamano);
            delete[] aux;
            aux=nuevo;
            tamano=3*tamano/2;
          }
          caux=getc(stdin);
        }
        while (!feof(stdin) && caux!=' ' && caux!='\n' && caux!='\t');
        aux[c]='\0';
        mpz_set_str (base[i][j], aux, 0);
      }
      else {
        printf("Empty file.\n");
        mpz_set_ui (base[i][j],0);
      }
   }
  }
  *F = numF;
  *C = numC;
  delete[] aux;
  return (base);
}

/*---------------------------------------------------------------------------
| llln_producto {}:                              |
| Multiply the integer vectors a and f with n coordinates.          |
|___________________________________________________________________________|*/

void  llln_producto (mpz_t * a, mpz_t * f, int n, mpz_t * prod)
{
  int i;
  mpz_t auxi;

  mpz_init (auxi);
  mpz_set_ui (*prod, 0);
  for (i = 1; i < n + 1; i++) {
    mpz_mul (auxi, a[i], f[i]);
    mpz_add (*prod, *prod, auxi);
  }
  mpz_clear (auxi);
}



void llln_test (int *k, int kma, int C, int n)
{
   int i;

   if (llln_f[*k-1] != 0)
      llln_redi (*k, *k -1, C, n);

   if (llln_f[*k-1] != 0 & llln_f[*k] == 0) {
      llln_swapk (*k, kma, C, n);
      *k = llln_max (2, *k-1);
      llln_test (k, kma, C, n);
   }
}


/*---------------------------------------------------------------------------
| lll_redi {}:                                                              |
| Make some changes in global variables.                                    |
|___________________________________________________________________________|*/


void llln_redi (int k, int l, int C, int F)
{
   int i;
   mpz_t q;
   mpz_t auxi1;

   mpz_init (q);
   mpz_init (auxi1);

   mpz_mul_ui (auxi1, llln_lambda[k][l], 2);
   mpz_abs (auxi1, auxi1);

   if (mpz_cmp (auxi1, llln_d[l]) > 0) {
      mpz_mul_ui (auxi1, llln_lambda[k][l], 2);
      mpz_add (auxi1, auxi1, llln_d[l]);
      mpz_mul_ui (q, llln_d[l], 2);
      mpz_fdiv_q (q, auxi1, q);

      for (i=1; i<F+1; i++) {
         mpz_mul (auxi1, q, llln_H[l][i]);
         mpz_sub (llln_H[k][i], llln_H[k][i], auxi1);
      }
    for (i=1; i<C+1; i++) {
         mpz_mul (auxi1, q, llln_b[l][i]);
         mpz_sub (llln_b[k][i], llln_b[k][i], auxi1);
      }
      mpz_mul (auxi1, q, llln_d[l]);

      mpz_sub (llln_lambda[k][l], llln_lambda[k][l], auxi1);

      for (i=1; i<=l-1; i++){
         mpz_mul (auxi1, q, llln_lambda[l][i]);
         mpz_sub (llln_lambda[k][i], llln_lambda[k][i], auxi1);
      }
 }

   mpz_clear (q);
   mpz_clear (auxi1);
}


/*---------------------------------------------------------------------------
| lll_swapi {}:                                                             |
| Exchange some global variables                                            |
|___________________________________________________________________________|*/


void llln_swapk (int k, int kmax, int C, int F)
{
   int i,j;
   mpz_t lamb;
   mpz_t auxi1;
   mpz_t t;
   mpz_t swap;

   mpz_init (lamb);
   mpz_init (auxi1);
   mpz_init (t);
   mpz_init (swap);

   for (j=1; j<F+1 ;j++) {
      mpz_set (swap, llln_H[k][j]);
      mpz_set (llln_H[k][j], llln_H[k-1][j]);
      mpz_set (llln_H[k-1][j], swap);
 }


 for (j=1; j<C+1 ;j++) {
      mpz_set (swap, llln_b[k][j]);
      mpz_set (llln_b[k][j], llln_b[k-1][j]);
      mpz_set (llln_b[k-1][j], swap);
   }

   if (k>2) {
      for (j=1;j<=k-2;j++) {
          mpz_set (swap, llln_lambda[k][j]);
          mpz_set (llln_lambda[k][j], llln_lambda[k-1][j]);
          mpz_set (llln_lambda[k-1][j], swap);
      }
   }
   mpz_set (lamb, llln_lambda[k][k-1]);
 if (mpz_sgn (lamb) == 0) {
    mpz_set (llln_d[k-1], llln_d[k-2]);
  llln_f[k-1]= 0;
  llln_f[k]= 1;
  mpz_set_ui (llln_lambda[k][k-1], 0);

    for (j=k+1; j <= kmax ; j++) {
      mpz_set (llln_lambda[j][k], llln_lambda[j][k-1]);
      mpz_set_ui (llln_lambda[j][k-1],0);
    }
  }
  else
  {
    for (j=k+1; j <= kmax ; j++)s
    {
      mpz_mul (auxi1, lamb, llln_lambda[j][k-1]);
      mpz_tdiv_q (llln_lambda[j][k-1], auxi1, llln_d[k-1]);
    }
    mpz_set (t, llln_d[k]);
    mpz_mul (auxi1, lamb, lamb);
    mpz_tdiv_q (llln_d[k-1], auxi1, llln_d[k-1]);
    mpz_set (llln_d[k], llln_d[k-1]);

    for (j=k+1; j <= kmax -1 ; j++)
    {
      for (i=j+1; i<= kmax; i++)
      {
        mpz_mul (auxi1, llln_lambda[i][j], llln_d[k-1]);
        mpz_tdiv_q (llln_lambda[i][j], auxi1, t);
      }
    }

    for (j=k+1; j<= kmax; j++)
    {
      mpz_mul (auxi1, llln_d[j], llln_d[k-1]);
      mpz_tdiv_q (llln_d[j], auxi1, t);
    }
  }

  mpz_clear (auxi1);
  mpz_clear (lamb);
  mpz_clear (t);
  mpz_clear (swap);
}
