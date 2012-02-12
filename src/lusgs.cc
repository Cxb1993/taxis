#include "fv.h"

//------------------------------------------------------------------------------
// Matrix-free LUSGS scheme
//------------------------------------------------------------------------------
void FiniteVolume::lusgs ()
{  
   unsigned int f = 0;
   PrimVar prim;
   int neighbour_cell;
   Flux flux_new, flux_old, summation_face; 
   Vector face_normal;
   const double gamma = param.material.gamma;
   const double prandtl = param.material.prandtl;

   // over relaxation factor; higher value improves stability
   // but slows down convergence; needs to be tuned
   const double omega = 1.5;
   
   // Forward sweep
   for (int i=0; i<grid.n_vertex; ++i)
   {   
      // initially summation over all faces initialized to zero.
      summation_face.zero ();
      
      // Diagonal scalar value for LUSGS
      // dt contains convective eigenvalue
      dt[i] = dt[i] / param.cfl + 0.5 * omega * dt[i];
      
      // Loop over neighboring cells
      for (unsigned int j=0; j<grid.vertex[i].nbr_vertex.size(); ++j)
      {
         f = grid.vertex[i].face[j];
         neighbour_cell = grid.vertex[i].nbr_vertex[j];

         if(param.material.model == Material::ns)
         {
            double mu = param.material.viscosity (primitive[i].temperature);
            double area = grid.face[f].area;
            double rho = param.material.Density(primitive[i]); // TODO: take average across face

            dt[i] += gamma * mu * area * area / (grid.dcarea[i] * rho * prandtl);
         } 


         if (neighbour_cell < i)
         {               
            face_normal = grid.face[f].normal;
	         if (grid.face[f].vertex[1] == i)
	            face_normal *= -1.0;
            
            param.material.euler_flux(primitive[neighbour_cell], 
                                      face_normal,
                                      flux_old);
            
            
            PrimVar prim_avg = (primitive[i] + primitive[neighbour_cell])*0.5;
	         double vel_normal  = prim_avg.velocity * face_normal;
	         double c  = param.material.sound_speed (prim_avg);
            double area = grid.face[f].area;
	         double lambda  = omega * (fabs(vel_normal) + c * area); 

            // viscous eigenvalue
            if(param.material.model == Material::ns)
            {
               Vector dr = grid.vertex[i].coord - grid.vertex[neighbour_cell].coord;
               double mu = param.material.viscosity (prim_avg.temperature);
               double density = param.material.Density (prim_avg);
               lambda += area * gamma * mu / (dr.norm() * density * prandtl);
            }
	         
            prim = param.material.con2prim(conserved_old[neighbour_cell]
                                           + residual[neighbour_cell]);
            param.material.euler_flux(prim, face_normal, flux_new);
            summation_face += (residual[neighbour_cell]*lambda
                               - (flux_new - flux_old))*(-0.5);
            
         }
         
      } 
            
      residual[i] += summation_face;
      residual[i] *= (-1.0/dt[i]); 
      // Now residual contains increment of conserved variable
   }
   
   // Backward Sweep
   for(int i=grid.n_vertex-1; i>=0; --i)
   {  
      // initially summation over all faces initialized to zero.
      summation_face.zero ();

      // Loop over neighboring cells
      for(unsigned int j=0; j<grid.vertex[i].nbr_vertex.size(); ++j)
      {  
         f = grid.vertex[i].face[j];
         neighbour_cell = grid.vertex[i].nbr_vertex[j];

         if (neighbour_cell > i)
         {	                
            face_normal = grid.face[f].normal;
            if (grid.face[f].vertex[1] == i)
               face_normal *= -1.0;
            	         
            param.material.euler_flux(primitive[neighbour_cell], 
                                      face_normal,
                                      flux_old);
            
            double area = grid.face[f].area;
            
            PrimVar prim_avg = (primitive[i] + primitive[neighbour_cell])*0.5;
            double vel_normal  = prim_avg.velocity * face_normal;
            double c  = param.material.sound_speed (prim_avg);
            double lambda  = omega * (fabs(vel_normal) + c * area);

            if(param.material.model == Material::ns)
            {
               Vector dr = grid.vertex[i].coord - grid.vertex[neighbour_cell].coord;
               double mu = param.material.viscosity (prim_avg.temperature);
               double density = param.material.Density (prim_avg);
               lambda += area * gamma * mu / (dr.norm() * density * prandtl);
            }

            
            prim = param.material.con2prim(conserved_old[neighbour_cell] 
                                           + residual[neighbour_cell]);
            param.material.euler_flux(prim, face_normal, flux_new);
            summation_face += (residual[neighbour_cell]*lambda -
                               (flux_new - flux_old))*(-0.5);
         }
      }
      residual[i] -= summation_face * (1.0/dt[i]);
   } 
   
}