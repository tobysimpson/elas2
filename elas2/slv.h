//
//  slv.h
//  frac2
//
//  Created by Toby Simpson on 12.01.24.
//

#ifndef slv_h
#define slv_h


//solve
int slv_mtx(struct prm_obj *prm, struct ocl_obj *ocl)
{
    //unsymmetric
    SparseAttributes_t atts;
    atts.kind = SparseOrdinary;
    atts.transpose = false;
    
//    //symmetric
//    SparseAttributes_t atts;
//    atts.triangle = SparseLowerTriangle;
//    atts.kind = SparseSymmetric;
    
    //size of input array
    long    blk_num  = 27*prm->nv_tot;
    uint8_t blk_sz   = 4;
    int     num_rows = prm->nv_tot;
    int     num_cols = prm->nv_tot;
    
    //create
    SparseMatrix_Float A = SparseConvertFromCoordinate(num_rows, num_cols, blk_num, blk_sz, atts, ocl->mtx_A.ii.hst, ocl->mtx_A.jj.hst, ocl->mtx_A.vv.hst);  //duplicates sum
    
    //vecs (count,data)
    DenseVector_Float u = {4*prm->nv_tot, (float*)ocl->vtx_uu.hst};
    DenseVector_Float f = {4*prm->nv_tot, (float*)ocl->vtx_ff.hst};
    
    /*
    ========================
    solve
    ========================
    */
    
//    //GMRES
//    SparseGMRESOptions options;
//    options.maxIterations =  4*prm->nv_tot;
//    options.nvec = 100;
//    options.atol = 1e-3f;
//    options.rtol = 1e-3f;
//    options.variant = SparseVariantGMRES;
//    SparseSolve(SparseGMRES(), A, f, u);
    
//    //CG
//    SparseCGOptions options;
//    options.maxIterations = 4*prm->nv_tot;
////    options.atol = 1e-3f;
////    options.rtol = 1e-3f;
    SparseSolve(SparseConjugateGradient(), A, f, u);

//    //QR
//    SparseOpaqueFactorization_Float QR = SparseFactor(SparseFactorizationQR, A);
//    SparseSolve(QR, f , u);
//    SparseCleanup(QR);
    
    //clean
    SparseCleanup(A);

    return 0;
}


#endif /* slv_h */
