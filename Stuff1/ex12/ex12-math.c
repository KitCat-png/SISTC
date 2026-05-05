#include <math.h>



void converte(double *dados_out, double *dados_in, long int nelem){ 
  double m, p;
  

  for(long int i=0; i < nelem; i = i+2){
    m = sqrt(dados_in[i]*dados_in[i] + dados_in[i+1]*dados_in[i+1]);
    p = atan2(dados_in[i+1], dados_in[i]);
    dados_out[i] = cos(p)*m ;
    dados_out[i+1] = sin(p)*m;
  }
} 

void rectangular2polar(double *dados_out, double *dados_in, long int nelem)
{
  for(long int i=0; i < nelem; i = i+2){
    dados_out[i] = sqrt(dados_in[i]*dados_in[i] + dados_in[i+1]*dados_in[i+1]);
    dados_out[i+1] = atan2(dados_in[i+1], dados_in[i]);
  }
}

void polar2rectangular(double *dados_out, double *dados_in, long int nelem)
{
  for(long int i=0; i < nelem; i = i+2){
    dados_out[i] = cos(dados_in[i+1])*dados_in[i];
    dados_out[i+1] = sin(dados_in[i+1])*dados_in[i];
  }
}

double mycos(double x) {
  
  if(x<0)
    return 2*x/M_PI+1; //2*(x+M_PI)/M_PI-1
  else
    return -2*x/M_PI+1; 
}

double mysin(double x) {
  

  if(x<-M_PI/2)
    return -2*x/M_PI-2;
  else
    if(x > M_PI/2)
      return -2*x/M_PI+2;
    else
      return 2*x/M_PI; 
}

