//
//  main.c
//  frac2
//
//  Created by Toby Simpson on 12.01.24.
//


//dynamic linear elasticity

#include <stdio.h>
#include <OpenCL/opencl.h>
#include <Accelerate/Accelerate.h>

#include "prm.h"
#include "ocl.h"
#include "slv.h"
#include "io.h"

//for later
//clSetKernelArg(myKernel, 0, sizeof(cl_int), &myVariable).


//here
int main(int argc, const char * argv[])
{
    printf("hello\n");
    
    //objects
    struct prm_obj prm;
    struct ocl_obj ocl;
    
    //init obj
    prm_init(&prm);
    ocl_init(&prm, &ocl);
    
    //cast dims
    size_t nv[3] = {prm.vtx_dim.x, prm.vtx_dim.y, prm.vtx_dim.z};
    
    /*
     ==============================
     init
     ==============================
     */
    
    //init
    ocl.err = clEnqueueNDRangeKernel(ocl.command_queue, ocl.vtx_init, 3, NULL, nv, NULL, 0, NULL, NULL);
    ocl.err = clEnqueueNDRangeKernel(ocl.command_queue, ocl.vtx_assm, 3, NULL, nv, NULL, 0, NULL, NULL);
//    ocl.err = clEnqueueNDRangeKernel(ocl.command_queue, ocl.vtx_elim, 3, NULL, nv, NULL, 0, NULL, NULL);
    ocl.err = clEnqueueNDRangeKernel(ocl.command_queue, ocl.vtx_bnd1, 3, NULL, nv, NULL, 0, NULL, NULL);

    
    for(int t=0; t<2; t++)
    {
        //read vec
        ocl.err = clEnqueueReadBuffer(ocl.command_queue, ocl.vtx_xx.dev, CL_TRUE, 0, prm.nv_tot*sizeof(cl_float4), ocl.vtx_xx.hst,  0, NULL, NULL);
        ocl.err = clEnqueueReadBuffer(ocl.command_queue, ocl.vtx_uu.dev, CL_TRUE, 0, prm.nv_tot*sizeof(cl_float4), ocl.vtx_uu.hst,  0, NULL, NULL);
        ocl.err = clEnqueueReadBuffer(ocl.command_queue, ocl.vtx_vv.dev, CL_TRUE, 0, prm.nv_tot*sizeof(cl_float4), ocl.vtx_vv.hst,  0, NULL, NULL);
        ocl.err = clEnqueueReadBuffer(ocl.command_queue, ocl.vtx_aa.dev, CL_TRUE, 0, prm.nv_tot*sizeof(cl_float4), ocl.vtx_aa.hst,  0, NULL, NULL);
        ocl.err = clEnqueueReadBuffer(ocl.command_queue, ocl.vtx_ff.dev, CL_TRUE, 0, prm.nv_tot*sizeof(cl_float4), ocl.vtx_ff.hst,  0, NULL, NULL);

        //read mtx
        ocl.err = clEnqueueReadBuffer(ocl.command_queue, ocl.mtx_A.ii.dev, CL_TRUE, 0, 27*prm.nv_tot*sizeof(int),        ocl.mtx_A.ii.hst,  0, NULL, NULL);
        ocl.err = clEnqueueReadBuffer(ocl.command_queue, ocl.mtx_A.jj.dev, CL_TRUE, 0, 27*prm.nv_tot*sizeof(int),        ocl.mtx_A.jj.hst,  0, NULL, NULL);
        ocl.err = clEnqueueReadBuffer(ocl.command_queue, ocl.mtx_A.vv.dev, CL_TRUE, 0, 27*prm.nv_tot*sizeof(cl_float16), ocl.mtx_A.vv.hst,  0, NULL, NULL);
        
        //write vtk
        wrt_vtk(&prm, &ocl, t);
        
        //write raw
        wrt_raw(ocl.vtx_xx.hst, prm.nv_tot, sizeof(cl_float4), "vtx_xx");
        wrt_raw(ocl.vtx_uu.hst, prm.nv_tot, sizeof(cl_float4), "vtx_uu");
        wrt_raw(ocl.vtx_vv.hst, prm.nv_tot, sizeof(cl_float4), "vtx_vv");
        wrt_raw(ocl.vtx_aa.hst, prm.nv_tot, sizeof(cl_float4), "vtx_aa");
        wrt_raw(ocl.vtx_ff.hst, prm.nv_tot, sizeof(cl_float4), "vtx_ff");
        
        //write raw
        wrt_raw(ocl.mtx_A.ii.hst, 27*prm.nv_tot, sizeof(int),       "A_ii");
        wrt_raw(ocl.mtx_A.jj.hst, 27*prm.nv_tot, sizeof(int),       "A_jj");
        wrt_raw(ocl.mtx_A.vv.hst, 27*prm.nv_tot, sizeof(cl_float16),"A_vv");
        
        //reset
        memset(ocl.vtx_uu.hst, 0, prm.nv_tot*sizeof(cl_float4));
        
        //solve
        slv_mtx(&prm, &ocl);
        
        //soln->dev
        ocl.err = clEnqueueWriteBuffer(ocl.command_queue, ocl.vtx_uu.dev, CL_TRUE, 0, prm.nv_tot*sizeof(cl_float4), ocl.vtx_uu.hst, 0, NULL, NULL);
        
        //calc error
        ocl.err = clEnqueueNDRangeKernel(ocl.command_queue, ocl.vtx_err1, 3, NULL, nv, NULL, 0, NULL, NULL);
        
    }
    
    //clean
    ocl_final(&prm, &ocl);
    
    printf("done\n");
    
    return 0;
}
