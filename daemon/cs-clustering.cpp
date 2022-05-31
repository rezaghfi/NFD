#include "cs-clustering.hpp"
#include <iostream>
#include <bits/stdc++.h>
#include <cmath>

namespace nfd{

namespace cs{

void createClustring(int clustersNodeNum[], int clustersNum, int clusterHead, int clusterNode[][]){
  double k[clustersNum];
  //read file and create clustring
  double c[clustersNum][clustersNodeNum], p[clustersNum][clustersNodeNum], ram[clustersNum][clustersNodeNum];
  double fc[clustersNum][clustersNodeNum];
  double ct[clustersNum][clustersNodeNum], pt[clustersNum][clustersNodeNum], ramt[clustersNum][clustersNodeNum];
  double cnorm[clustersNum][clustersNodeNum], pnorm[clustersNum][clustersNodeNum], ramnorm[clustersNum][clustersNodeNum];
   for(int i=0; i<clustersNum; i++){
    for(int j=0; j <clustersNodeNum[i]; j++){
      cnorm[i][j] += c[i][j];
      pnorm[i][j] +=p[i][j];
      ramnorm[i][j] += ram[i][j];
    }
  }
  // SET F FOR rounter in any cluster for once
  for(int i=0; i<clustersNum; i++){
    k[i] = (-1)/log(clustersNodeNum);
    for(int j=0; j <clustersNodeNum[i]; j++){
      cnorm[i][j] = c[i][j]/cnorm[i][j]; 
      pnorm[i][j] = c[i][j]/pnorm[i][j]; 
      ramnorm[i][j] = ram[i][j]/ramnorm[i][j]; 
      fc[i][j] = (cnorm[i][j]*(1-k[i]*ct[i][j])+pnorm[i][j]*(1-k[i]*pt[i][j])+ramnorm[i][j]*(1-(k[i]*ramt[i][j])))/((1-k[i]*ct[i][j])+(1-k[i]*pt[i][j])+(1-k[i]*ramt[i][j]));
    }
  }
  
}

int placement(int routerNum, double k[],){
  
  double p[routerNum];
  
  for(int i=0; i<routerNum;i++){
    p[i] = nnorm * (1-k[i]*nt) + hnorm * (1-k[i]*ht) _cnorm*(1-k[i]*ct)+fnorm*(1-k[i]*ft)
    if(p[i]>p[i+1])
      temp = i
    else temp = i+1
  }
  //sort(p, p+routerNum, greater<int>());
  return temp;
}
}
}
