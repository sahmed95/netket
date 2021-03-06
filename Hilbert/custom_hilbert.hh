// Copyright 2018 The Simons Foundation, Inc. - All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <Eigen/Dense>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>

#ifndef NETKET_CUSTOM_HILBERT_HH
#define NETKET_CUSTOM_HILBERT_HH

namespace netket{

using namespace std;
using namespace Eigen;

/**
  User-Define Hilbert space
*/

class CustomHilbert : public AbstractHilbert{

  std::vector<double> local_;

  int nstates_;

  int size_;

public:

  CustomHilbert(const json & pars){

    if(FieldExists(pars["Hilbert"],"QuantumNumbers")){
      local_=pars["Hilbert"]["QuantumNumbers"].get<std::vector<double>>();
    }
    else{
      cerr<<"QuantumNumbers are not defined"<<endl;
    }

    if(FieldExists(pars["Hilbert"],"Size")){
      size_=pars["Hilbert"]["Size"];
      if(size_<=0){
        cerr<<"Hilbert Size parameter must be positive"<<endl;
        std::abort();
      }
    }
    else{
      cerr<<"Hilbert space extent is not defined"<<endl;
    }

    nstates_=local_.size();
  }

  bool IsDiscrete()const{
    return true;
  }

  int LocalSize()const{
    return nstates_;
  }

  int Size()const{
    return size_;
  }

  vector<double> LocalStates()const{
    return local_;
  }

  void RandomVals(VectorXd & state,netket::default_random_engine & rgen)const{
    std::uniform_int_distribution<int> distribution(0,nstates_-1);

    assert(state.size()==size_);

    //unconstrained random
    for(int i=0;i<state.size();i++){
      state(i)=local_[distribution(rgen)];
    }

  }

  void UpdateConf(VectorXd & v,const vector<int>  & tochange,
    const vector<double> & newconf)const{

    assert(v.size()==size_);

    int i=0;
    for(auto sf: tochange){
      v(sf)=newconf[i];
      i++;
    }
  }


};

}
#endif
