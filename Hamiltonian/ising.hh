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

#ifndef NETKET_ISING1D_HH
#define NETKET_ISING1D_HH

#include <iostream>
#include <Eigen/Dense>
#include <random>
#include <complex>
#include <vector>
#include <mpi.h>

namespace netket{

using namespace std;
using namespace Eigen;

/**
  Transverse field Ising model on an arbitrary graph.
*/
template<class G> class Ising: public AbstractHamiltonian {

  const int nspins_;
  double h_;
  double J_;

  const G & graph_;

  /**
    List of bonds for the interaction part.
  */
  std::vector<std::vector<int>> bonds_;

  int mynode_;

  /**
    Hilbert space descriptor for this hamiltonian.
  */
  Hilbert hilbert_;

public:

  /**
    Contructor with explicit parameters.
    @param G is a graph from which the number of spins and the bonds are obtained.
    @param h is the transverse field coupled to sigma_x.
    @param J is the interaction constant for the sigma_z(i)*sigma_z(j) part.
  */
  Ising(const G & graph,double h,double J=1):
       nspins_(graph.Nsites()),h_(h),J_(J),graph_(graph){

    Init();
  }

  /**
    Json constructor.
    @param G is a graph from which the number of spins and the bonds are obtained.
    @param pars is a json list of parameters. The default value of J is 1.0
  */
  Ising(const G & graph,const json & pars):
    nspins_(graph.Nsites()),
    h_(FieldVal(pars["Hamiltonian"],"h")),
    J_(FieldOrDefaultVal(pars["Hamiltonian"],"J",1.0)),
    graph_(graph){

    Init();
  }

  void Init(){
    GenerateBonds();

    MPI_Comm_rank(MPI_COMM_WORLD, &mynode_);

    //Specifying the hilbert space
    json hil;
    hil["Hilbert"]["Name"]="Spin";
    hil["Hilbert"]["Nspins"]=nspins_;
    hil["Hilbert"]["S"]=0.5;

    hilbert_.Init(hil);

    if(mynode_==0){
      cout<<"# Transverse-Field Ising model created "<<endl;
      cout<<"# h = "<<h_<<endl;
      cout<<"# J = "<<J_<<endl;
    }
  }

  /**
    Member function generating the bonds on the lattice.
    bonds[i][k] contains the k-th bond for site i.
  */
  void GenerateBonds(){
    auto adj=graph_.AdjacencyList();

    bonds_.resize(nspins_);

    for(int i=0;i<nspins_;i++){
      for (auto s : adj[i]){
        if(s>i){
          bonds_[i].push_back(s);
        }
      }
    }
  }

  /**
  Member function finding the connected elements of the Hamiltonian.
  Starting from a given visible state v, it finds all other visible states v'
  such that the hamiltonian matrix element H(v,v') is different from zero.
  In general there will be several different connected visible units satisfying
  this condition, and they are denoted here v'(k), for k=0,1...N_connected.
  @param v a constant reference to the visible configuration.
  @param mel(k) is modified to contain matrix elements H(v,v'(k)).
  @param connector(k) for each k contains a list of sites that should be changed
  to obtain v'(k) starting from v.
  @param newconfs(k) is a vector containing the new values of the visible units on the
  affected sites, such that: v'(k,connectors(k,j))=newconfs(k,j). For the other
  sites v'(k)=v, i.e. they are equal to the starting visible configuration.
  */
  void FindConn(const VectorXd & v,vector<std::complex<double>> & mel,
    vector<vector<int>> & connectors,vector<vector<double>> & newconfs){

    connectors.clear();
    connectors.resize(nspins_+1);
    newconfs.clear();
    newconfs.resize(nspins_+1);
    mel.resize(nspins_+1);

    mel[0]=0;
    connectors[0].resize(0);
    newconfs[0].resize(0);

    for(int i=0;i<nspins_;i++){
      //spin flips
      mel[i+1]=-h_;
      connectors[i+1].push_back(i);
      newconfs[i+1].push_back(-v(i));

      //interaction part
      for(auto bond : bonds_[i]){
        mel[0]-=J_*v(i)*v(bond);
      }
    }

  }

  const Hilbert & GetHilbert()const{
    return hilbert_;
  }

};


}

#endif
