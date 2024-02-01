//
//  msh.h
//  frac2
//
//  Created by Toby Simpson on 01.02.24.
//

#ifndef msh_h
#define msh_h


//object
struct msh_obj
{
    cl_int3     ele_dim;
    cl_int3     vtx_dim;
    cl_float4   dx;
    cl_float8   mat_prm;
    
    int         ne_tot;
    int         nv_tot;
};


//init
void msh_init(struct msh_obj *msh)
{
    //dim
    msh->ele_dim.x = 4;
    msh->ele_dim.y = msh->ele_dim.x;
    msh->ele_dim.z = msh->ele_dim.x;
    
    msh->vtx_dim = (cl_int3){msh->ele_dim.x+1, msh->ele_dim.y+1, msh->ele_dim.z+1};
    
    printf("ele_dim %d %d %d\n", msh->ele_dim.x, msh->ele_dim.y, msh->ele_dim.z);
    printf("vtx_dim %d %d %d\n", msh->vtx_dim.x, msh->vtx_dim.y, msh->vtx_dim.z);
    
    //dx, dt
    msh->dx.x = 1e+0f/(float)msh->ele_dim.x;
    msh->dx.y = 1e+0f/(float)msh->ele_dim.y;
    msh->dx.z = 1e+0f/(float)msh->ele_dim.z;
    msh->dx.w = 1e-1f;
    
    printf("dx %+f %+f %+f %+f\n", msh->dx.x, msh->dx.y, msh->dx.z, msh->dx.w);
    
    //material params
    msh->mat_prm.s0 = 1e-0f;     //lamé      lambda
    msh->mat_prm.s1 = 1e-0f;     //lamé      mu
    msh->mat_prm.s2 = 1e+0f;     //density   rho     mg/mm^3
    msh->mat_prm.s3 = 1e-2f;     //gravity   g       mm/ms
    msh->mat_prm.s4 = 0e+0f;
    msh->mat_prm.s5 = 0e+0f;
    msh->mat_prm.s6 = 0e+0f;
    msh->mat_prm.s7 = 0e+0f;

    
//    printf("mat_prm %e %e %e %e\n", msh->mat_prm.s0, msh->mat_prm.s1, msh->mat_prm.z, msh->mat_prm.w);
    printf("mat_prm.s0 %f\n", msh->mat_prm.s0);
    printf("mat_prm.s1 %f\n", msh->mat_prm.s1);
    printf("mat_prm.s2 %f\n", msh->mat_prm.s2);
    printf("mat_prm.s3 %f\n", msh->mat_prm.s3);
    printf("mat_prm.s4 %f\n", msh->mat_prm.s4);
    printf("mat_prm.s5 %f\n", msh->mat_prm.s5);
    printf("mat_prm.s6 %f\n", msh->mat_prm.s6);
    printf("mat_prm.s7 %f\n", msh->mat_prm.s7);
    
    //totals
    msh->ne_tot = msh->ele_dim.x*msh->ele_dim.y*msh->ele_dim.z;
    msh->nv_tot = msh->vtx_dim.x*msh->vtx_dim.y*msh->vtx_dim.z;
    
    printf("ne_tot=%d\n", msh->ne_tot);
    printf("nv_tot=%d\n", msh->nv_tot);
    
    return;
}


#endif /* msh_h */
