//
//  ocl.h
//  frac2
//
//  Created by Toby Simpson on 12.01.24.
//

#ifndef ocl_h
#define ocl_h


#define ROOT_PRG    "/Users/toby/Documents/USI/postdoc/fracture/xcode/elas2/elas2"


struct buf_int
{
    int*            hst;
    cl_mem          dev;
};


struct buf_float
{
    float*          hst;
    cl_mem          dev;
};


struct buf_float4
{
    cl_float4*      hst;
    cl_mem          dev;
};


struct buf_coo
{
    struct buf_int   ii;
    struct buf_int   jj;
    struct buf_float vv;
    
};


//object
struct ocl_obj
{
    //environment
    cl_int              err;
    cl_platform_id      platform_id;
    cl_device_id        device_id;
    cl_uint             num_devices;
    cl_uint             num_platforms;
    cl_context          context;
    cl_command_queue    command_queue;
    cl_program          program;
    char                device_str[100];
    cl_event            event;  //for profiling
        
    //memory
    struct buf_float4 vtx_xx;
    struct buf_float4 vtx_uu;
    struct buf_float4 vtx_vv;
    struct buf_float4 vtx_aa;
    struct buf_float4 vtx_ff;
    
    struct buf_coo mtx_A;
    
    //kernels
    cl_kernel vtx_init;
    cl_kernel vtx_assm;
    cl_kernel vtx_bnd1;
    cl_kernel vtx_elim;
};


//init
void ocl_init(struct msh_obj *msh, struct ocl_obj *ocl)
{
    printf("__FILE__: %s\n", __FILE__);
    
    /*
     =============================
     environment
     =============================
     */
    
    ocl->err            = clGetPlatformIDs(1, &ocl->platform_id, &ocl->num_platforms);                                              //platform
    ocl->err            = clGetDeviceIDs(ocl->platform_id, CL_DEVICE_TYPE_GPU, 1, &ocl->device_id, &ocl->num_devices);              //devices
    ocl->context        = clCreateContext(NULL, ocl->num_devices, &ocl->device_id, NULL, NULL, &ocl->err);                          //context
    ocl->command_queue  = clCreateCommandQueue(ocl->context, ocl->device_id, CL_QUEUE_PROFILING_ENABLE, &ocl->err);                 //command queue
    ocl->err            = clGetDeviceInfo(ocl->device_id, CL_DEVICE_NAME, sizeof(ocl->device_str), &ocl->device_str, NULL);         //device info
    
    printf("%s\n", ocl->device_str);
    
    /*
     =============================
     program
     =============================
     */
    
    //name
    char prg_name[200];
    sprintf(prg_name,"%s/%s", ROOT_PRG, "prg.cl");

    printf("%s\n",prg_name);

    //file
    FILE* src_file = fopen(prg_name, "r");
    if(!src_file)
    {
        fprintf(stderr, "Failed to load kernel. check ROOT_PRG\n");
        exit(1);
    }

    //length
    fseek(src_file, 0, SEEK_END);
    size_t  prg_len =  ftell(src_file);
    rewind(src_file);

//    printf("%lu\n",prg_len);

    //source
    char *prg_src = (char*)malloc(prg_len);
    fread(prg_src, sizeof(char), prg_len, src_file);
    fclose(src_file);

//    printf("%s\n",prg_src);

    //create
    ocl->program = clCreateProgramWithSource(ocl->context, 1, (const char**)&prg_src, (const size_t*)&prg_len, &ocl->err);
    printf("prg %d\n",ocl->err);
    
    //clean
    free(prg_src);

    //build
    ocl->err = clBuildProgram(ocl->program, 1, &ocl->device_id, NULL, NULL, NULL);
    printf("bld %d\n",ocl->err);
    
    //unload compiler
    ocl->err = clUnloadPlatformCompiler(ocl->platform_id);
    
    /*
     =============================
     log
     =============================
     */

    //log
    size_t log_size = 0;
    
    //log size
    clGetProgramBuildInfo(ocl->program, ocl->device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

    //allocate
    char *log = (char*)malloc(log_size);

    //log text
    clGetProgramBuildInfo(ocl->program, ocl->device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

    //print
    printf("%s\n", log);

    //clear
    free(log);
    
    /*
     =============================
     kernels
     =============================
     */

    ocl->vtx_init = clCreateKernel(ocl->program, "vtx_init", &ocl->err);
    ocl->vtx_assm = clCreateKernel(ocl->program, "vtx_assm", &ocl->err);
    ocl->vtx_bnd1 = clCreateKernel(ocl->program, "vtx_bnd1", &ocl->err);
    ocl->vtx_elim = clCreateKernel(ocl->program, "vtx_elim", &ocl->err);

    /*
     =============================
     memory
     =============================
     */
    
    //CL_MEM_READ_WRITE/CL_MEM_HOST_READ_ONLY/CL_MEM_HOST_NO_ACCESS / CL_MEM_ALLOC_HOST_PTR
    
    //buf
    ocl->vtx_xx.dev = clCreateBuffer(ocl->context, CL_MEM_HOST_READ_ONLY, msh->nv_tot*sizeof(cl_float4), NULL, &ocl->err);
    ocl->vtx_uu.dev = clCreateBuffer(ocl->context, CL_MEM_HOST_READ_ONLY, msh->nv_tot*sizeof(cl_float4), NULL, &ocl->err);
    ocl->vtx_vv.dev = clCreateBuffer(ocl->context, CL_MEM_HOST_READ_ONLY, msh->nv_tot*sizeof(cl_float4), NULL, &ocl->err);
    ocl->vtx_aa.dev = clCreateBuffer(ocl->context, CL_MEM_HOST_READ_ONLY, msh->nv_tot*sizeof(cl_float4), NULL, &ocl->err);
    ocl->vtx_ff.dev = clCreateBuffer(ocl->context, CL_MEM_HOST_READ_ONLY, msh->nv_tot*sizeof(cl_float4), NULL, &ocl->err);
    
    //coo
    ocl->mtx_A.ii.dev = clCreateBuffer(ocl->context, CL_MEM_HOST_READ_ONLY, 27*msh->nv_tot*sizeof(cl_int16),   NULL, &ocl->err);
    ocl->mtx_A.jj.dev = clCreateBuffer(ocl->context, CL_MEM_HOST_READ_ONLY, 27*msh->nv_tot*sizeof(cl_int16),   NULL, &ocl->err);
    ocl->mtx_A.vv.dev = clCreateBuffer(ocl->context, CL_MEM_HOST_READ_ONLY, 27*msh->nv_tot*sizeof(cl_float16), NULL, &ocl->err);

    
    /*
     =============================
     arguments
     =============================
     */

    ocl->err = clSetKernelArg(ocl->vtx_init,  0, sizeof(cl_float4), (void*)&msh->dx);
    ocl->err = clSetKernelArg(ocl->vtx_init,  1, sizeof(cl_mem),    (void*)&ocl->vtx_xx.dev);
    ocl->err = clSetKernelArg(ocl->vtx_init,  2, sizeof(cl_mem),    (void*)&ocl->vtx_uu.dev);
    ocl->err = clSetKernelArg(ocl->vtx_init,  3, sizeof(cl_mem),    (void*)&ocl->vtx_ff.dev);
    ocl->err = clSetKernelArg(ocl->vtx_init,  4, sizeof(cl_mem),    (void*)&ocl->mtx_A.ii.dev);
    ocl->err = clSetKernelArg(ocl->vtx_init,  5, sizeof(cl_mem),    (void*)&ocl->mtx_A.jj.dev);
    ocl->err = clSetKernelArg(ocl->vtx_init,  6, sizeof(cl_mem),    (void*)&ocl->mtx_A.vv.dev);

    ocl->err = clSetKernelArg(ocl->vtx_assm,  0, sizeof(cl_float3), (void*)&msh->dx);
    ocl->err = clSetKernelArg(ocl->vtx_assm,  1, sizeof(cl_float8), (void*)&msh->mat_prm);
    ocl->err = clSetKernelArg(ocl->vtx_assm,  2, sizeof(cl_mem),    (void*)&ocl->vtx_uu.dev);
    ocl->err = clSetKernelArg(ocl->vtx_assm,  3, sizeof(cl_mem),    (void*)&ocl->vtx_ff.dev);
    ocl->err = clSetKernelArg(ocl->vtx_assm,  4, sizeof(cl_mem),    (void*)&ocl->mtx_A.vv.dev);
    
    ocl->err = clSetKernelArg(ocl->vtx_bnd1,  0, sizeof(cl_mem),    (void*)&ocl->vtx_ff.dev);
    ocl->err = clSetKernelArg(ocl->vtx_bnd1,  1, sizeof(cl_mem),    (void*)&ocl->mtx_A.vv.dev);
    
    ocl->err = clSetKernelArg(ocl->vtx_elim,  0, sizeof(cl_mem),    (void*)&ocl->vtx_ff.dev);
    ocl->err = clSetKernelArg(ocl->vtx_elim,  1, sizeof(cl_mem),    (void*)&ocl->mtx_A.vv.dev);
}


//final
void ocl_final(struct msh_obj *msh, struct ocl_obj *ocl)
{
    ocl->err = clFlush(ocl->command_queue);
    ocl->err = clFinish(ocl->command_queue);
    
    //kernels
    ocl->err = clReleaseKernel(ocl->vtx_init);
    ocl->err = clReleaseKernel(ocl->vtx_assm);
    ocl->err = clReleaseKernel(ocl->vtx_bnd1);
    ocl->err = clReleaseKernel(ocl->vtx_elim);
    
    //memory
    ocl->err = clReleaseMemObject(ocl->vtx_xx.dev);
    ocl->err = clReleaseMemObject(ocl->vtx_uu.dev);
    ocl->err = clReleaseMemObject(ocl->vtx_vv.dev);
    ocl->err = clReleaseMemObject(ocl->vtx_aa.dev);
    ocl->err = clReleaseMemObject(ocl->vtx_ff.dev);
    
    ocl->err = clReleaseMemObject(ocl->mtx_A.ii.dev);
    ocl->err = clReleaseMemObject(ocl->mtx_A.jj.dev);
    ocl->err = clReleaseMemObject(ocl->mtx_A.vv.dev);
    
    ocl->err = clReleaseProgram(ocl->program);
    ocl->err = clReleaseCommandQueue(ocl->command_queue);
    ocl->err = clReleaseContext(ocl->context);
    
    return;
}


#endif /* ocl_h */