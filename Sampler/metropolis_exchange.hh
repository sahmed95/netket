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

#ifndef NETKET_METROPOLISEXCHANGE_HH
#define NETKET_METROPOLISEXCHANGE_HH

#include <iostream>
#include <Eigen/Dense>
#include <random>
#include <mpi.h>

namespace netket{

using namespace std;
using namespace Eigen;

//Metropolis sampling generating local exchanges
template<class WfType> class MetropolisExchange: public AbstractSampler<WfType>{

  WfType & psi_;

  const Hilbert & hilbert_;

  //number of visible units
  const int nv_;

  netket::default_random_engine rgen_;

  //states of visible units
  VectorXd v_;

  VectorXd accept_;
  VectorXd moves_;

  int mynode_;
  int totalnodes_;

  //clusters to do updates
  std::vector<std::vector<int>> clusters_;

  //Look-up tables
  typename WfType::LookupType lt_;

public:

  template<class G> MetropolisExchange(G & graph,WfType & psi,int dmax=1):
       psi_(psi),hilbert_(psi.GetHilbert()),nv_(hilbert_.Size()){

    Init(graph,dmax);
  }

  //Json constructor
  MetropolisExchange(Graph & graph,WfType & psi,const json & pars):
    psi_(psi),hilbert_(psi.GetHilbert()),nv_(hilbert_.Size()){

    int dmax=FieldOrDefaultVal(pars["Sampler"],"Dmax",1);
    Init(graph,dmax);

  }

  template<class G> void Init(G & graph,int dmax){
    v_.resize(nv_);

    MPI_Comm_size(MPI_COMM_WORLD, &totalnodes_);
    MPI_Comm_rank(MPI_COMM_WORLD, &mynode_);

    accept_.resize(1);
    moves_.resize(1);

    GenerateClusters(graph,dmax);

    Seed();

    Reset(true);

    if(mynode_==0){
      cout<<"# Metropolis Exchange sampler is ready "<<endl;
      cout<<"# "<<dmax<<" is the maximum distance for exchanges"<<endl;
    }
  }

  template<class G> void GenerateClusters(G & graph,int dmax){
    auto dist=graph.Distances();

    assert(dist.size()==nv_);

    for(int i=0;i<nv_;i++){
      for(int j=0;j<nv_;j++){
        if(dist[i][j]<=dmax && i!=j){
          clusters_.push_back({i,j});
        }
      }
    }
  }

  void Seed(int baseseed=0){
    std::random_device rd;
    vector<int> seeds(totalnodes_);

    if(mynode_==0){
      for(int i=0;i<totalnodes_;i++){
        seeds[i]=rd()+baseseed;
      }
    }

    SendToAll(seeds);

    rgen_.seed(seeds[mynode_]);
  }


  void Reset(bool initrandom=false){
    if(initrandom){
      if(initrandom){
        hilbert_.RandomVals(v_,rgen_);
      }
    }

    psi_.InitLookup(v_,lt_);

    accept_=VectorXd::Zero(1);
    moves_=VectorXd::Zero(1);
  }

  void Sweep(){

    vector<int> tochange(2);
    std::uniform_real_distribution<double> distu;
    std::uniform_int_distribution<int> distcl(0,clusters_.size()-1);

    vector<double> newconf(2);

    for(int i=0;i<nv_;i++){

      int rcl=distcl(rgen_);
      assert(rcl<clusters_.size());
      int si=clusters_[rcl][0];
      int sj=clusters_[rcl][1];

      assert(si<nv_ && sj<nv_);

      if(std::abs(v_(si)-v_(sj))>std::numeric_limits<double>::epsilon()){

        tochange=clusters_[rcl];
        newconf[0]=v_(sj);
        newconf[1]=v_(si);

        double ratio=std::norm(std::exp(psi_.LogValDiff(v_,tochange,newconf,lt_)));

        if(ratio>distu(rgen_)){
          accept_[0]+=1;
          psi_.UpdateLookup(v_,tochange,newconf,lt_);
          hilbert_.UpdateConf(v_,tochange,newconf);
        }
      }
      moves_[0]+=1;
    }

  }


  VectorXd Visible(){
    return v_;
  }

  void SetVisible(const VectorXd & v){
    v_=v;
  }


  WfType & Psi(){
    return psi_;
  }


  Hilbert & HilbSpace()const{
    return hilbert_;
  }

  VectorXd Acceptance()const{
    VectorXd acc=accept_;
    for(int i=0;i<1;i++){
      acc(i)/=moves_(i);
    }
    return acc;
  }

};


}

#endif
