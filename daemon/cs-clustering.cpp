#include "cs-clustering.hpp"
#include <iostream>
#include <bits/stdc++.h>

namespace nfd{

namespace cs{
int nodeNum;
int clusterNodes[nodeNum];
int first = 0;
int routerSelect(int routerNum){
  int temp;
  double p[routerNum];
  double fc[routerNum];
  for(int i=0; i<routerNum;i++){
    fc[i] = (cnorm*(1-k*ct)+pnorm*(1-k*pt)+ramnorm*(1-(k*ramt)))/((1-k*ct)+(1-kpt)+(1-k*ramt));
    p[i] = nnorm * (1-k*nt) + hnorm * (1-k*ht) _cnorm*(1-k*ct)+fnorm*(1-k*ft)
    if(p[i]>p[i+1])
      temp = i
    else temp = i+1
  }
  //sort(p, p+routerNum, greater<int>());
  return temp;
}
}
}
