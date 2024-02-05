//
//  prg.cl
//  frac2
//
//  Created by Toby Simpson on 12.01.24.
//


/*
 ===================================
 prototypes
 ===================================
 */

int     fn_idx1(int3 pos, int3 dim);
int     fn_idx3(int3 pos);

int     fn_bnd1(int3 pos, int3 dim);
int     fn_bnd2(int3 pos, int3 dim);

float   fn_f1(float4 p);
float   fn_u1(float4 p);

void    mem_gr3(global float4 *buf, float4 uu3[27], int3 pos, int3 dim);
void    mem_lr2(float4 uu3[27], float4 uu2[8], int3 pos);

void    bas_eval(float3 p, float ee[8]);
void    bas_grad(float3 p, float3 gg[8], float4 dx);
float4  bas_itpe(float4 uu2[8], float bas_ee[8]);
float16 bas_itpg(float4 uu2[8], float3 bas_gg[8]);
float16 bas_tens(int dim, float3 g);

float3  mtx_mv(float16 A, float3 v);
float16 mtx_mm(float16 A, float16 B);
float16 mtx_uvT(float3 u, float3 v);
float16 mtx_mT(float16 A);
float16 mtx_md(float16 A, float3 D);

float   sym_tr(float8 A);
float   sym_det(float8 A);
float8  sym_vvT(float3 v);
float3  sym_mv(float8 A, float3 v);
float8  sym_mm(float8 A, float8 B);
float8  sym_mdmT(float16 V, float3 D);
float8  sym_sumT(float16 A);
float   sym_tip(float8 A, float8 B);

float8  mec_E(float16 du);
float8  mec_S(float8 E, float8 mat);

/*
 ===================================
 constants
 ===================================
 */


constant int3 off2[8] = {{0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}};

constant int3 off3[27] = {{0,0,0},{1,0,0},{2,0,0},{0,1,0},{1,1,0},{2,1,0},{0,2,0},{1,2,0},{2,2,0},
                          {0,0,1},{1,0,1},{2,0,1},{0,1,1},{1,1,1},{2,1,1},{0,2,1},{1,2,1},{2,2,1},
                          {0,0,2},{1,0,2},{2,0,2},{0,1,2},{1,1,2},{2,1,2},{0,2,2},{1,2,2},{2,2,2}};


/*
 ===================================
 analytic
 ===================================
 */

//rhs
float fn_f1(float4 p)
{
    return 2e0f*(p.x*p.y*(p.x - 1e0f)*(p.y - 1e0f) + p.x*p.z*(p.x - 1e0f)*(p.z - 1e0f) + p.y*p.z*(p.y - 1e0f)*(p.z - 1e0f));
}

//soln
float fn_u1(float4 p)
{
    return p.x*(1e0f - p.x)*p.y*(1e0f - p.y)*p.z*(1e0f - p.z);
}

/*
 ===================================
 utilities
 ===================================
 */

//flat index
int fn_idx1(int3 pos, int3 dim)
{
    return pos.x + dim.x*(pos.y + dim.y*pos.z);
}

//index 3x3x3
int fn_idx3(int3 pos)
{
    return pos.x + 3*pos.y + 9*pos.z;
}

//in-bounds
int fn_bnd1(int3 pos, int3 dim)
{
    return all(pos>=0)*all(pos<dim);
}

//on the boundary
int fn_bnd2(int3 pos, int3 dim)
{
    return (pos.x==0)||(pos.y==0)||(pos.z==0)||(pos.x==dim.x-1)||(pos.y==dim.y-1)||(pos.z==dim.z-1);
}

/*
 ===================================
 quadrature [0,1]
 ===================================
 */

//1-point gauss [0,1]
constant float qp1 = 5e-1f;
constant float qw1 = 1e+0f;

//2-point gauss [0,1]
constant float qp2[2] = {0.211324865405187f,0.788675134594813f};
constant float qw2[2] = {5e-1f,5e-1f};

//3-point gauss [0,1]
constant float qp3[3] = {0.112701665379258f,0.500000000000000f,0.887298334620742f};
constant float qw3[3] = {0.277777777777778f,0.444444444444444f,0.277777777777778f};

/*
 ===================================
 basis
 ===================================
 */

//eval at qp
void bas_eval(float3 p, float ee[8])
{
    float x0 = 1e0f - p.x;
    float y0 = 1e0f - p.y;
    float z0 = 1e0f - p.z;
    
    float x1 = p.x;
    float y1 = p.y;
    float z1 = p.z;
    
    ee[0] = x0*y0*z0;
    ee[1] = x1*y0*z0;
    ee[2] = x0*y1*z0;
    ee[3] = x1*y1*z0;
    ee[4] = x0*y0*z1;
    ee[5] = x1*y0*z1;
    ee[6] = x0*y1*z1;
    ee[7] = x1*y1*z1;
    
    return;
}


//grad at qp
void bas_grad(float3 p, float3 gg[8], float4 dx)
{
    float x0 = 1e0f - p.x;
    float y0 = 1e0f - p.y;
    float z0 = 1e0f - p.z;
    
    float x1 = p.x;
    float y1 = p.y;
    float z1 = p.z;
    
    //{/dx,/dy,/dz}
    gg[0] = (float3){-y0*z0, -x0*z0, -x0*y0}/dx.xyz;
    gg[1] = (float3){+y0*z0, -x1*z0, -x1*y0}/dx.xyz;
    gg[2] = (float3){-y1*z0, +x0*z0, -x0*y1}/dx.xyz;
    gg[3] = (float3){+y1*z0, +x1*z0, -x1*y1}/dx.xyz;
    gg[4] = (float3){-y0*z1, -x0*z1, +x0*y0}/dx.xyz;
    gg[5] = (float3){+y0*z1, -x1*z1, +x1*y0}/dx.xyz;
    gg[6] = (float3){-y1*z1, +x0*z1, +x0*y1}/dx.xyz;
    gg[7] = (float3){+y1*z1, +x1*z1, +x1*y1}/dx.xyz;
    
    return;
}


//interp eval
float4 bas_itpe(float4 uu2[8], float bas_ee[8])
{
    float4 u = 0e0f;
    
    for(int i=0; i<8; i++)
    {
        u += uu2[i]*bas_ee[i];
    }
    return u;
}


//interp grad, Jacobian, u_grad[i] = du[i](rows)/{dx,dy,dz}(cols)
float16 bas_itpg(float4 uu2[8], float3 bas_gg[8])
{
    float16 du = 0e0f;
    
    for(int i=0; i<8; i++)
    {
        du.s048 += uu2[i].x*bas_gg[i];
        du.s159 += uu2[i].y*bas_gg[i];
        du.s26a += uu2[i].z*bas_gg[i];
        du.s37b += uu2[i].w*bas_gg[i];
    }
    return du;
}


//tensor basis gradient
float16 bas_tens(int dim, float3 g)
{
    float16 du = 0e0f;
    
    du.s048 = (dim==0)?g:0e0f;
    du.s159 = (dim==1)?g:0e0f;
    du.s26a = (dim==2)?g:0e0f;
    
    return du;
}



/*
 ===================================
 memory
 ===================================
 */


//global read 3x3x3 vectors
void mem_gr3(global float4 *buf, float4 uu3[27], int3 pos, int3 dim)
{
    for(int i=0; i<27; i++)
    {
        int3 adj_pos1 = pos + off3[i] - 1;
        int  adj_idx1 = fn_idx1(adj_pos1, dim);
        
        //copy/cast
        uu3[i] = buf[adj_idx1];
    }
    return;
}


//local read 2x2x2 from 3x3x3 vector
void mem_lr2(float4 uu3[27], float4 uu2[8], int3 pos)
{
    for(int i=0; i<8; i++)
    {
        int3 adj_pos3 = pos + off2[i];
        int  adj_idx3 = fn_idx3(adj_pos3);
        
        //copy
        uu2[i] = uu3[adj_idx3];
    }
    return;
}


/*
 ===================================
 matrix R^3x3
 ===================================
 */

//mmult Av
float3 mtx_mv(float16 A, float3 v)
{
    return v.x*A.s012 + v.y*A.s456 + v.z*A.s89a;
}

float16 mtx_mm(float16 A, float16 B)
{
    float16 C = 0e0f;
    
    C.s012 = mtx_mv(A,B.s012);
    C.s456 = mtx_mv(A,B.s456);
    C.s89a = mtx_mv(A,B.s89a);
    
    return C;
}

//outer prod
float16 mtx_uvT(float3 u, float3 v)
{
    return (float16){(float3)v.x*u,0e0f,(float3)v.y*u,0e0f,(float3)v.z*u,0e0f,0e0f,0e0f,0e0f,0e0f};
}

//transpose
float16 mtx_mT(float16 A)
{
    return A.s048c159d26ae37bf;
}

//matrix * diagonal
float16 mtx_md(float16 A, float3 D)
{
    return (float16){(float3)D.x*A.s012, 0e0f, (float3)D.y*A.s456, 0e0f, (float3)D.z*A.s89a, 0e0f, 0e0f, 0e0f, 0e0f, 0e0f};
}


/*
 ===================================
 symmetric R^3x3
 ===================================
 */

//sym trace
float sym_tr(float8 A)
{
    return dot(A.s035,(float3){1e0f,1e0f,1e0f});
}

//sym determinant
float sym_det(float8 A)
{
    return dot((float3){A.s0,A.s1,A.s2}, cross((float3){A.s1, A.s3, A.s4}, (float3){A.s2, A.s4, A.s5}));
}

//outer product vv^T
float8 sym_vvT(float3 v)
{
    return (float8){v.x*v.x, v.x*v.y, v.x*v.z, v.y*v.y, v.y*v.z, v.z*v.z, 0e0f, 0e0f};
}

//sym mtx-vec
float3 sym_mv(float8 A, float3 v)
{
    return v.x*A.s012 + v.y*A.s134 + v.z*A.s245;
}

//sym mtx-mtx
float8 sym_mm(float8 A, float8 B)
{
    return (float8){A.s0*B.s0 + A.s1*B.s1 + A.s2*B.s2,
        A.s0*B.s1 + A.s1*B.s3 + A.s2*B.s4,
        A.s0*B.s2 + A.s1*B.s4 + A.s2*B.s5,
        A.s1*B.s1 + A.s3*B.s3 + A.s4*B.s4,
        A.s1*B.s2 + A.s3*B.s4 + A.s4*B.s5,
        A.s2*B.s2 + A.s4*B.s4 + A.s5*B.s5, 0e0f, 0e0f};
}

//mult V, diag D
float8  sym_mdmT(float16 V, float3 D)
{
    return (float8){D.x*V.s0*V.s0 + D.y*V.s4*V.s4 + D.z*V.s8*V.s8,
                    D.x*V.s0*V.s1 + D.y*V.s4*V.s5 + D.z*V.s8*V.s9,
                    D.x*V.s0*V.s2 + D.y*V.s4*V.s6 + D.z*V.s8*V.sa,
                    D.x*V.s1*V.s1 + D.y*V.s5*V.s5 + D.z*V.s9*V.s9,
                    D.x*V.s1*V.s2 + D.y*V.s5*V.s6 + D.z*V.s9*V.sa,
                    D.x*V.s2*V.s2 + D.y*V.s6*V.s6 + D.z*V.sa*V.sa, 0e0f, 0e0f};
}

//sum S = A+A^T
float8 sym_sumT(float16 A)
{
    float8 S = 0e0f;
    
    S.s0123 = A.s0125 + A.s0485;
    S.s45 = A.s6a + A.s9a;

    return S;
}

//sym tensor inner prod
float sym_tip(float8 A, float8 B)
{
    return dot(A.s0123,B.s0123) + dot(A.s45,B.s45);
}


/*
 ===================================
 mechanics
 ===================================
 */

//strain (du + du^T)/2
float8 mec_E(float16 du)
{
    return 5e-1f*sym_sumT(du);
}


//stress pk2 = lam*tr(e)*I + 2*mu*e
float8 mec_S(float8 E, float8 mat)
{
    float8 S = 2e0f*mat.s1*E;
    S.s035 += mat.s0*sym_tr(E);
    
    return S;
}


/*
 ===================================
 kernels
 ===================================
 */

//init
kernel void vtx_init(const  float4  dx,
                     global float4  *vtx_xx,
                     global float4  *vtx_uu,
                     global float4  *vtx_vv,
                     global float4  *vtx_aa,
                     global float4  *vtx_ff,
                     global int16   *A_ii,
                     global int16   *A_jj,
                     global float16 *A_vv)
{
    int3 vtx_dim = {get_global_size(0), get_global_size(1), get_global_size(2)};
    int3 vtx1_pos1 = {get_global_id(0)  , get_global_id(1),   get_global_id(2)};
    int  vtx1_idx1 = fn_idx1(vtx1_pos1, vtx_dim);
    
    //coord
    float4 x = dx*convert_float4((int4){vtx1_pos1,0});

//    printf("init %3d %v3d %v3f\n", vtx1_idx1, vtx1_pos1, vtx_xx[vtx1_idx1].xyz);
    
    //vec
    vtx_xx[vtx1_idx1] = x;
    vtx_uu[vtx1_idx1] = 0e0f;
    vtx_vv[vtx1_idx1] = (float4){0e0f, 0e0f, 0e0f, fn_u1(x)};   //ana
    vtx_aa[vtx1_idx1] = 0e0f;                                   //err
    vtx_ff[vtx1_idx1] = 0e0f;                                   //rhs
    
    //vtx2
    for(int vtx2_idx3=0; vtx2_idx3<27; vtx2_idx3++)
    {
        int3 vtx2_pos1 = vtx1_pos1 + off3[vtx2_idx3] - 1;
        int  vtx2_idx1 = fn_idx1(vtx2_pos1, vtx_dim);
        int  vtx2_bnd1 = fn_bnd1(vtx2_pos1, vtx_dim);
        
        //block
        int idx1 = 27*vtx1_idx1 + vtx2_idx3;
        
        //block pointers
        global int*   ii = (global int*)&A_ii[idx1];
        global int*   jj = (global int*)&A_jj[idx1];
        global float* vv = (global float*)&A_vv[idx1];
        
        //dim1
        for(int dim1=0; dim1<4; dim1++)
        {
            //dim2
            for(int dim2=0; dim2<4; dim2++)
            {
                //uu
                int idx2 = 4*dim1 + dim2;
                ii[idx2] = vtx2_bnd1*(4*vtx1_idx1 + dim1);
                jj[idx2] = vtx2_bnd1*(4*vtx2_idx1 + dim2);
//                vv[idx2] = vtx2_bnd1*(vtx1_idx1==vtx2_idx1)*(dim1==3)*(dim2==3); //set Ac=I
                vv[idx2] = vtx2_bnd1*(vtx1_idx1==vtx2_idx1)*(dim1==dim2)*(dim1<3); //set Au=I

            } //dim2
            
        } //dim1
        
    } //vtx2
    
    return;
}


//assemble
kernel void vtx_assm(const  float4   dx,
                     const  float8   mat,
                     global float4   *vtx_xx,
                     global float4   *vtx_uu,
                     global float4   *vtx_vv,
                     global float4   *vtx_aa,
                     global float4   *vtx_ff,
                     global float16  *A_vv)
{
    int3 vtx_dim = {get_global_size(0), get_global_size(1), get_global_size(2)};
    int3 ele_dim = vtx_dim - 1;
    int3 vtx1_pos1  = {get_global_id(0)  ,get_global_id(1)  ,get_global_id(2)};
    int  vtx1_idx1 = fn_idx1(vtx1_pos1, vtx_dim);
    
    //volume
    float vlm = dx.x*dx.y*dx.z;
    
//    printf("vtx1_pos1 %v3d\n", vtx1_pos1);
    
    //read 3x3x3
    float4  xx3[27];
//    float4  uu3[27];
    mem_gr3(vtx_xx, xx3, vtx1_pos1, vtx_dim);
//    mem_gr3(vtx_uu, uu3, vtx1_pos1, vtx_dim);
    
    //ref - avoids -ve int bug
    int  vtx1_idx2 = 8;
    
    
    //ele1
    for(uint ele1_idx2=0; ele1_idx2<8; ele1_idx2++)
    {
        int3 ele1_pos2 = off2[ele1_idx2];
        int3 ele1_pos1 = vtx1_pos1 + ele1_pos2 - 1;
        int  ele1_bnd1 = fn_bnd1(ele1_pos1, ele_dim);
        
        //ref vtx (decrement to avoid bug)
        vtx1_idx2 -= 1;
        
        //in-bounds
        if(ele1_bnd1)
        {
            //read 2x2x2
            float4 xx2[8];
//            float4 uu2[8];
            mem_lr2(xx3, xx2, ele1_pos2);
//            mem_lr2(uu3, uu2, ele1_pos2);

            //qpt1 (change limit with scheme 1,8,27)
            for(int qpt1=0; qpt1<8; qpt1++)
            {
//                //1pt
//                float3 qp = (float3){qp1,qp1,qp1};
//                float  qw = qw1*qw1*qw1;
                
                //2pt
                float3 qp = (float3){qp2[off2[qpt1].x], qp2[off2[qpt1].y], qp2[off2[qpt1].z]};
                float  qw = qw2[off2[qpt1].x]*qw2[off2[qpt1].y]*qw2[off2[qpt1].z];
                
//                //3pt
//                float3 qp = (float3){qp3[off3[qpt1].x], qp3[off3[qpt1].y], qp3[off3[qpt1].z]};
//                float  qw = qw3[off3[qpt1].x]*qw3[off3[qpt1].y]*qw3[off3[qpt1].z];
                
                //vlm
                qw *= vlm;
                
                //basis
                float  bas_ee[8];
                float3 bas_gg[8];
                bas_eval(qp, bas_ee);
                bas_grad(qp, bas_gg, dx);
                
                //interp
                float4 x = bas_itpe(xx2, bas_ee);
                
                //gravity
//                float3 g = (float3){0e+0f, 0e+0f, -mat.s2*mat.s3};
                
                //rhs
//                vtx_ff[vtx1_idx1].xyz += g*bas_ee[vtx1_idx2]*qw;
                vtx_ff[vtx1_idx1].w += fn_f1(x)*bas_ee[vtx1_idx2]*qw;
                
                //vtx2
                for(int vtx2_idx2=0; vtx2_idx2<8; vtx2_idx2++)
                {
                    //idx
                    int3 vtx2_pos3 = ele1_pos2 + off2[vtx2_idx2];
                    int  vtx2_idx3 = fn_idx3(vtx2_pos3);
                    
                    //block ptr
                    int idx1 = 27*vtx1_idx1 + vtx2_idx3;
                    global float* vv = (global float*)&A_vv[idx1];
                    
                    //poisson
                    vv[15] += dot(bas_gg[vtx2_idx2], bas_gg[vtx1_idx2])*qw;
                    
//                    //dim1
//                    for(int dim1=0; dim1<3; dim1++)
//                    {
//                        //tensor basis
//                        float16 du1 = bas_tens(dim1, bas_gg[vtx1_idx2]);
//                        
//                        //strain
//                        float8 E1 = mec_E(du1);
//                          
//                        //dim2
//                        for(int dim2=0; dim2<3; dim2++)
//                        {
//                            //tensor basis
//                            float16 du2 = bas_tens(dim2, bas_gg[vtx2_idx2]);
//                            
//                            //strain
//                            float8 E2 = mec_E(du2);
//                            
//                            //stress
//                            float8 S2 = mec_S(E2, mat);
//                            
//                            //write uu
//                            int idx2 = 4*dim1 + dim2;
//                            vv[idx2] += sym_tip(S2, E1)*qw;
//
//                        } //dim2
//                        
//                    } //dim1
                    
                } //vtx2
                
            } //qpt
            
        } //ele1_bnd1
        
    } //ele
    
    return;
}




//zero dirichlet Au->I, vtx_ff->0
kernel void vtx_bnd1(global float4  *vtx_ff,
                     global float16 *A_vv)
{
    int3 vtx_dim = {get_global_size(0), get_global_size(1), get_global_size(2)};
    int3 vtx1_pos1  = {get_global_id(0), get_global_id(1), get_global_id(2)};
    int  vtx1_idx1 = fn_idx1(vtx1_pos1, vtx_dim);
    
    //bools
//    int b1 = (vtx1_pos1.x == 0);                    //side
    int b1 = fn_bnd2(vtx1_pos1, vtx_dim);            //all sides
    

    //K=I, vtx_ff=0
    if(b1>0)
    {
        //vtx_ff=0
        vtx_ff[vtx1_idx1] = (float4){0e0f, 0e0f, 0e0f, 0e0f};
        
        //vtx2
        for(int vtx2_idx3=0; vtx2_idx3<27; vtx2_idx3++)
        {
            int3 vtx2_pos1 = vtx1_pos1 + off3[vtx2_idx3] - 1;
            int  vtx2_idx1 = fn_idx1(vtx2_pos1, vtx_dim);
            int  vtx2_bnd1 = fn_bnd1(vtx2_pos1, vtx_dim);
            
            //local pointer
            int idx1 = 27*vtx1_idx1 + vtx2_idx3;
            global float* vv = (global float*)&A_vv[idx1];
            
            //dim1
            for(int dim1=0; dim1<4; dim1++)
            {
                //dim2
                for(int dim2=0; dim2<4; dim2++)
                {
                    //Ku=I
                    int idx2 = 4*dim1 + dim2;
                    vv[idx2] = vtx2_bnd1*(vtx1_idx1==vtx2_idx1)*(dim1==dim2);
                    
                } //dim2
                
            } //dim1
            
        } //vtx2
        
    } //if
    
    return;
}


//eliminate cols for symmetry
kernel void vtx_elim(global float4  *vtx_ff,
                     global float16 *A_vv)
{
    int3 vtx_dim = {get_global_size(0), get_global_size(1), get_global_size(2)};
    int3 vtx1_pos1  = {get_global_id(0), get_global_id(1), get_global_id(2)};
    int  vtx1_idx1 = fn_idx1(vtx1_pos1, vtx_dim);
    
    
    //vtx2 - along the row of vtx1
    for(int vtx2_idx3=0; vtx2_idx3<27; vtx2_idx3++)
    {
        int3 vtx2_pos1 = vtx1_pos1 + off3[vtx2_idx3] - 1;
        
        //bc
//        int b2 = (vtx2_pos1.x==0);        //side
        int b2 = fn_bnd2(vtx2_pos1, vtx_dim);            //all sides
        
        //test bc
        if(b2)
        {
            //blocks
            int idx1 = 27*vtx1_idx1 + vtx2_idx3;
            global float* vv = (global float*)&A_vv[idx1];
            
            //dim1
            for(int dim1=0; dim1<4; dim1++)
            {
                //dim2
                for(int dim2=0; dim2<4; dim2++)
                {
                    //cols
                    int idx2 = 4*dim1 + dim2;
                    vv[idx2] = 0e0f; //vtx2_bnd1*(vtx1_idx1!=vtx2_idx1);
                    
                } //dim2
                
            } //dim1
            
        }//if
        
    } //vtx2

    return;
}


//error
kernel void vtx_err1(global float4  *vtx_uu,
                      global float4  *vtx_vv,
                      global float4  *vtx_aa)
{
    int3 vtx_dim = {get_global_size(0), get_global_size(1), get_global_size(2)};
    int3 vtx1_pos1  = {get_global_id(0), get_global_id(1), get_global_id(2)};
    int  vtx1_idx1 = fn_idx1(vtx1_pos1, vtx_dim);
    
    vtx_aa[vtx1_idx1] = vtx_uu[vtx1_idx1] - vtx_vv[vtx1_idx1];
    
    return;
}
