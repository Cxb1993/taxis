#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include "writer.h"

using namespace std;

//------------------------------------------------------------------------------
// Add primitive variables defined at vertices
//------------------------------------------------------------------------------
void Writer::attach_data (vector<PrimVar>& data)
{
   assert (!has_primitive);
   vertex_primitive = &data;
   has_primitive = true;
}

//------------------------------------------------------------------------------
// Specify which variables to write
//------------------------------------------------------------------------------
void Writer::attach_variables (const vector<string>& variables)
{
   if(variables.size() > 0)
      assert (has_primitive);

   for(unsigned int i=0; i<variables.size(); ++i)
      if(variables[i]=="mach")
         write_mach = true;
      else if(variables[i]=="density")
         write_density = true;
      else if(variables[i]=="vorticity")
         write_vorticity = true;
      else
      {
         cout << "Writer: unknown variable " << variables[i] << endl;
         abort ();
      }
}

//------------------------------------------------------------------------------
// Add gradient values; currently only velocity gradients added
//------------------------------------------------------------------------------
void Writer::attach_gradient (std::vector<Vector>& dU_,
                              std::vector<Vector>& dV_,
                              std::vector<Vector>& dW_)
{
   assert (!has_gradient);
   dU = &dU_;
   dV = &dV_;
   dW = &dW_;
   has_gradient = true;
}

//------------------------------------------------------------------------------
// Call output function for saving solution to file
//------------------------------------------------------------------------------
void Writer::output (string format, int counter, double elapsed_time)
{
   string filename = "sol";
   if     (counter <= 9)    filename = "sol000";
   else if(counter <= 99)   filename = "sol00";
   else if(counter <= 999)  filename = "sol0";
   else if(counter <= 9999) filename = "sol";
   else
   {
      cout << "Writer::output: counter is too large !!!\n";
      exit(0);
   }
   stringstream ss;
   ss << counter;
   filename += ss.str();

   if(format == "vtk")
   {
      filename += ".vtk";
      output_vtk (filename);
   }
   else if(format == "tec")
   {
      filename += ".plt";
      output_tec (elapsed_time, filename);
   }
   cout << "Saving solution into file " << filename << endl;
}

//------------------------------------------------------------------------------
// Write data to vtk file
//------------------------------------------------------------------------------
void Writer::output_vtk (string filename)
{
   ofstream vtk;
   vtk.open (filename.c_str());

   vtk << "# vtk DataFile Version 3.0" << endl;
   vtk << "flo3d" << endl;
   vtk << "ASCII" << endl;
   vtk << "DATASET UNSTRUCTURED_GRID" << endl;
   vtk << "POINTS  " << grid->n_vertex << "  float" << endl;

   for(unsigned int i=0; i<grid->n_vertex; ++i)
      vtk << grid->vertex[i].coord.x << " " 
          << grid->vertex[i].coord.y << " " 
          << grid->vertex[i].coord.z << endl;

   vtk << "CELLS  " << grid->n_cell << " " << 4 * grid->n_cell << endl;
   for(unsigned int i=0; i<grid->n_cell; ++i)
      vtk << 3 << "  " 
          << grid->cell[i].vertex[0] << " "
          << grid->cell[i].vertex[1] << " "
          << grid->cell[i].vertex[2] << endl;

   vtk << "CELL_TYPES  " << grid->n_cell << endl;
   for(unsigned int i=0; i<grid->n_cell; ++i)
      vtk << 5 << endl;

   // Write vertex data
   if(has_primitive) 
      vtk << "POINT_DATA  " << grid->n_vertex << endl;

   // If vertex primitive data is available, write to file
   if (has_primitive)
   {
      vtk << "SCALARS pressure float 1" << endl;
      vtk << "LOOKUP_TABLE default" << endl;
      for(unsigned int i=0; i<grid->n_vertex; ++i)
         vtk << (*vertex_primitive)[i].pressure << endl;

      vtk << "SCALARS temperature float 1" << endl;
      vtk << "LOOKUP_TABLE default" << endl;
      for(unsigned int i=0; i<grid->n_vertex; ++i)
         vtk << (*vertex_primitive)[i].temperature << endl;

      vtk << "VECTORS velocity float" << endl;
      for(unsigned int i=0; i<grid->n_vertex; ++i)
         vtk << (*vertex_primitive)[i].velocity.x << "  "
             << (*vertex_primitive)[i].velocity.y << "  "
             << (*vertex_primitive)[i].velocity.z
             << endl;
   }


   // Write mach number
   if(write_mach)
   {
      vtk << "SCALARS mach float 1" << endl;
      vtk << "LOOKUP_TABLE default" << endl;
      for(unsigned int i=0; i<grid->n_vertex; ++i)
      {
         double mach = material->Mach ( (*vertex_primitive)[i] );
         vtk << mach << endl;
      }
   }

   // Write density
   if(write_density)
   {
      vtk << "SCALARS density float 1" << endl;
      vtk << "LOOKUP_TABLE default" << endl;
      for(unsigned int i=0; i<grid->n_vertex; ++i)
      {
         double density = material->Density ((*vertex_primitive)[i]);
         vtk << density << endl;
      }
   }

   // write vorticity
   if(write_vorticity)
   {
      // Check if gradient information is available
      assert(has_gradient);

      vtk << "SCALARS vorticity float 1" << endl;
      vtk << "LOOKUP_TABLE default" << endl;
      for(unsigned int i=0; i<grid->n_vertex; ++i)
      {
         double vorticity = (*dV)[i].x - (*dU)[i].y;
         vtk << vorticity << endl;
      }
   }

   vtk.close ();
}

//------------------------------------------------------------------------------
// Write data to tecplot file
//------------------------------------------------------------------------------
void Writer::output_tec (double time, string filename)
{
   // Save grid only the first time this function is called
   static bool write_grid = true;
   if(write_grid)
   {
      ofstream tec;
      tec.open ("grid.plt");
   
      tec << "FILETYPE = GRID" << endl;
      tec << "VARIABLES = \"X\" \"Y\"" << endl;
   
      tec << "ZONE  DATAPACKING=BLOCK, NODES=" << grid->n_vertex 
         << ", ELEMENTS=" << grid->n_cell << ", ZONETYPE=FETRIANGLE" << endl;
   
      for(unsigned int i=0; i<grid->n_vertex; ++i)
         tec << grid->vertex[i].coord.x << endl;
   
      for(unsigned int i=0; i<grid->n_vertex; ++i)
         tec << grid->vertex[i].coord.y << endl;      

      // Triangles
      for(unsigned int i=0; i<grid->n_cell; ++i)
         tec << 1+grid->cell[i].vertex[0] << " "
             << 1+grid->cell[i].vertex[1] << " "
             << 1+grid->cell[i].vertex[2] << endl;
   
      write_grid = false;
   }

   // Save solution into different file
   ofstream tec;
   tec.open (filename.c_str());
   
   tec << "FILETYPE = SOLUTION" << endl;
   tec << "VARIABLES = \"T\" \"U\" \"V\" \"P\"";
   if(write_mach)
      tec << " \"Mach\"";
   if(write_density)
      tec << " \"Density\"";
   if(write_vorticity)
      tec << " \"Vorticity\"";
   tec << endl;
   
   tec << "ZONE STRANDID=1, SOLUTIONTIME=" << time 
       << ", DATAPACKING=BLOCK, NODES=" << grid->n_vertex 
       << ", ELEMENTS=" << grid->n_cell << ", ZONETYPE=FETRIANGLE" << endl;
   
   // If vertex primitive data is available, write to file
   if (has_primitive)
   {
      for(unsigned int i=0; i<grid->n_vertex; ++i)
         tec << (*vertex_primitive)[i].pressure << endl;

      for(unsigned int i=0; i<grid->n_vertex; ++i)
         tec << (*vertex_primitive)[i].temperature << endl;
      
      for(unsigned int i=0; i<grid->n_vertex; ++i)
         tec << (*vertex_primitive)[i].velocity.x << endl;
      
      for(unsigned int i=0; i<grid->n_vertex; ++i)
         tec << (*vertex_primitive)[i].velocity.y << endl;
   }
   
   
   // Write mach number
   if(write_mach)
   {
      for(unsigned int i=0; i<grid->n_vertex; ++i)
      {
         double mach = material->Mach ( (*vertex_primitive)[i] );
         tec << mach << endl;
      }
   }
   
   // Write density
   if(write_density)
   {
      for(unsigned int i=0; i<grid->n_vertex; ++i)
      {
         double density = material->Density ((*vertex_primitive)[i]);
         tec << density << endl;
      }
   }
   
   // write vorticity
   if(write_vorticity)
   {
      // Check if gradient information is available
      assert(has_gradient);
      
      for(unsigned int i=0; i<grid->n_vertex; ++i)
      {
         double vorticity = (*dV)[i].x - (*dU)[i].y;
         tec << vorticity << endl;
      }
   }
   
   tec.close ();
}

//------------------------------------------------------------------------------
// Write solution for restarting
//------------------------------------------------------------------------------
void Writer::output_restart (int iter)
{
   assert (has_primitive);

   cout << "Saving restart file restart.dat\n";

   ofstream fo;
   fo.open ("restart.dat");
   assert (fo.is_open());

   for(unsigned int i=0; i<grid->n_vertex; ++i)
      fo << scientific
         << (*vertex_primitive)[i].temperature << "  "
         << (*vertex_primitive)[i].velocity.x  << "  "
         << (*vertex_primitive)[i].velocity.y  << "  "
         << (*vertex_primitive)[i].velocity.z  << "  "
         << (*vertex_primitive)[i].pressure    << endl;

   fo << iter << endl;
   fo.close ();
}
