/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/*
* ABSTRACT: Gauss-Manin system normal form
*/

#ifndef GMS_H
#define GMS_H

lists gmsNF(ideal p,ideal g,matrix B,int D,int K);
BOOLEAN gmsNF(leftv res,leftv h);

#endif /* GMS_H */
