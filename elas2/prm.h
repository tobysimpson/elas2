//
//  prm.h
//  frac2
//
//  Created by Toby Simpson on 01.02.24.
//

#ifndef prm_h
#define prm_h


//object
struct prm_obj
{
    cl_int3     ele_dim;
    cl_int3     vtx_dim;
    
    int         ne_tot;
    int         nv_tot;
    
    cl_float3   x0;
    cl_float3   x1;
    cl_float4   dx;
    
    cl_float8   mat;
};


//init
void prm_init(struct prm_obj *prm)
{
    //dim
    prm->ele_dim.x = 8;
    prm->ele_dim.y = prm->ele_dim.x;
    prm->ele_dim.z = prm->ele_dim.x;
    
    prm->vtx_dim = (cl_int3){prm->ele_dim.x+1, prm->ele_dim.y+1, prm->ele_dim.z+1};
    
    printf("ele_dim %d %d %d\n", prm->ele_dim.x, prm->ele_dim.y, prm->ele_dim.z);
    printf("vtx_dim %d %d %d\n", prm->vtx_dim.x, prm->vtx_dim.y, prm->vtx_dim.z);
    
    //x1,dx, dt
    prm->x0 = (cl_float3){0e+0f, 0e+0f, 0e+0f};
    prm->x1 = (cl_float3){1e+0f, 1e+0f, 1e+0f};
    
    prm->dx.x = (prm->x1.x - prm->x0.x)/(float)prm->ele_dim.x;
    prm->dx.y = (prm->x1.y - prm->x0.y)/(float)prm->ele_dim.y;
    prm->dx.z = (prm->x1.z - prm->x0.z)/(float)prm->ele_dim.z;
    prm->dx.w = 1e-1f;                                             //dt
    
    printf("dx %+f %+f %+f %+f\n", prm->dx.x, prm->dx.y, prm->dx.z, prm->dx.w);
    
    //material params
    prm->mat.s0 = 1e-0f;     //lamé      lambda
    prm->mat.s1 = 1e-0f;     //lamé      mu
    prm->mat.s2 = 1e+0f;     //density   rho     mg/mm^3
    prm->mat.s3 = 1e-2f;     //gravity   g       mm/ms
    prm->mat.s4 = 0e+0f;
    prm->mat.s5 = 0e+0f;
    prm->mat.s6 = 0e+0f;
    prm->mat.s7 = 0e+0f;

    
//    printf("mat %e %e %e %e\n", msh->mat.s0, msh->mat.s1, msh->mat.z, msh->mat.w);
    printf("mat.s0 %f\n", prm->mat.s0);
    printf("mat.s1 %f\n", prm->mat.s1);
    printf("mat.s2 %f\n", prm->mat.s2);
    printf("mat.s3 %f\n", prm->mat.s3);
    printf("mat.s4 %f\n", prm->mat.s4);
    printf("mat.s5 %f\n", prm->mat.s5);
    printf("mat.s6 %f\n", prm->mat.s6);
    printf("mat.s7 %f\n", prm->mat.s7);
    
    //totals
    prm->ne_tot = prm->ele_dim.x*prm->ele_dim.y*prm->ele_dim.z;
    prm->nv_tot = prm->vtx_dim.x*prm->vtx_dim.y*prm->vtx_dim.z;
    
    printf("ne_tot=%d\n", prm->ne_tot);
    printf("nv_tot=%d\n", prm->nv_tot);
    
    return;
}


#endif /* prm_h */
