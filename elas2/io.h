//
//  io.h
//  frac2
//
//  Created by Toby Simpson on 12.01.24.
//

#ifndef io_h
#define io_h


#define ROOT_WRITE  "/Users/toby/Downloads/"

//write
void wrt_raw(void *ptr, size_t n, size_t bytes, char *file_name)
{
//    printf("%s\n",file_name);
    
    //name
    char file_path[250];
    sprintf(file_path, "%s%s.raw", ROOT_WRITE, file_name);

    //open
    FILE* file = fopen(file_path,"wb");
  
    //write
    fwrite(ptr, bytes, n, file);
    
    //close
    fclose(file);
    
    return;
}


//write
void wrt_vtk(struct msh_obj *msh, struct ocl_obj *ocl, int k)
{

    FILE* file1;
    char file1_name[250];
    
    //file name
    sprintf(file1_name, "%s%s.%03d.vtk", ROOT_WRITE, "grid1", k);
    
    //open
    file1 = fopen(file1_name,"w");
    
    //write
    fprintf(file1,"# vtk DataFile Version 3.0\n");
    fprintf(file1,"grid1\n");
    fprintf(file1,"ASCII\n");
    fprintf(file1,"DATASET STRUCTURED_GRID\n");
    fprintf(file1,"DIMENSIONS %d %d %d\n", msh->vtx_dim.x, msh->vtx_dim.y, msh->vtx_dim.z);
    
    /*
     ===================
     coords
     ===================
     */
    
    fprintf(file1,"\nPOINTS %d float\n", msh->nv_tot);

    for(int i=0; i<msh->nv_tot; i++)
    {
        fprintf(file1, "%e %e %e\n", ocl->vtx_xx.hst[i].x, ocl->vtx_xx.hst[i].y, ocl->vtx_xx.hst[i].z);
    }

    //point data flag
    fprintf(file1,"\nPOINT_DATA %d\n", msh->nv_tot);
    
    /*
     ===================
     uu
     ===================
     */
    
    fprintf(file1,"VECTORS Uu float\n");

    for(int i=0; i<msh->nv_tot; i++)
    {
        fprintf(file1, "%e %e %e\n", ocl->vtx_uu.hst[i].x, ocl->vtx_uu.hst[i].y, ocl->vtx_uu.hst[i].z);
    }
    
    /*
     ===================
     fu
     ===================
     */

    fprintf(file1,"VECTORS Fu float\n");

    for(int i=0; i<msh->nv_tot; i++)
    {
        fprintf(file1, "%e %e %e\n", ocl->vtx_ff.hst[i].x, ocl->vtx_ff.hst[i].y, ocl->vtx_ff.hst[i].z);
    }
    
    /*
     ===================
     uc
     ===================
     */
    
    fprintf(file1,"SCALARS Uc float 1\n");
    fprintf(file1,"LOOKUP_TABLE default\n");
    
    for(int i=0; i<msh->nv_tot; i++)
    {
        fprintf(file1, "%e\n", ocl->vtx_uu.hst[i].w);
    }

    /*
     ===================
     fc
     ===================
     */

    fprintf(file1,"SCALARS Fc float 1\n");
    fprintf(file1,"LOOKUP_TABLE default\n");
    
    for(int i=0; i<msh->nv_tot; i++)
    {
        fprintf(file1, "%e\n", ocl->vtx_ff.hst[i].w);
    }
    
    //clean up
    fclose(file1);

    return;
}



#endif /* io_h */
