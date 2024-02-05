//
//  slv.h
//  frac2
//
//  Created by Toby Simpson on 12.01.24.
//

#ifndef slv_h
#define slv_h



void dsp_vec(DenseVector_Float v)
{
    for(int i=0; i<v.count; i++)
    {
        printf("%+e ", v.data[i]);
    }
    printf("\n\n");
    
    return;
}


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
    long blk_num = 27*16*prm->nv_tot;
    int num_rows = 4*prm->nv_tot;
    int num_cols = 4*prm->nv_tot;
    uint8_t blk_sz = 1;

    //create
    SparseMatrix_Float A = SparseConvertFromCoordinate(num_rows, num_cols, blk_num, blk_sz, atts, ocl->mtx_A.ii.hst, ocl->mtx_A.jj.hst, ocl->mtx_A.vv.hst);  //duplicates sum
    
    //vecs
    DenseVector_Float u;
    DenseVector_Float f;
    
    u.count = 4*prm->nv_tot;
    f.count = 4*prm->nv_tot;
    
    u.data = (float*)ocl->vtx_uu.hst;
    f.data = (float*)ocl->vtx_ff.hst;
    
    /*
     ========================
     print
     ========================
     */
    
//    int col_idx = 0;
//    
//    int sum = 0;
//
//    for(int i=0; i<A.structure.columnStarts[A.structure.columnCount]; i++)
//    {
//        if(i == A.structure.columnStarts[col_idx+1])
//        {
//            col_idx += 1;
//        }
//        
//        if(A.structure.rowIndices[i]==col_idx)
//        {
//            printf("(%3d,%3d) %f\n",A.structure.rowIndices[i],col_idx,A.data[i]);
//            
//            sum += 1;
//        }
//    }
//    
//    printf("%3d\n",sum);

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

//    //LSMR
//    SparseSolve(SparseLSMR(), A, f, u); //minres - symmetric
    
//    //QR
//    SparseOpaqueFactorization_Float QR = SparseFactor(SparseFactorizationQR, A);
//    SparseSolve(QR, f , u);
//    SparseCleanup(QR);
    
    //clean
    SparseCleanup(A);

    return 0;
}


#endif /* slv_h */
