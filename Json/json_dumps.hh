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

#ifndef NETKET_JSON_DUMPS_HH
#define NETKET_JSON_DUMPS_HH

#include <Eigen/Dense>
#include <vector>
#include <complex>

namespace Eigen{

  template<class T> void to_json(json &j,const Matrix<T, Dynamic, 1> & v){
    std::vector<T> temp(v.size());
    for(int i=0;i<v.size();i++){
      temp[i]=v(i);
    }
    j=json(temp);
  }

  template<class T> void from_json(const json& j, Matrix<T, Dynamic, 1> & v) {
    std::vector<T> temp=j.get<std::vector<T>>();
    v.resize(temp.size());
    for(int i=0;i<temp.size();i++){
      v(i)=temp[i];
    }
  }

  template<class T> void to_json(json &j,const Matrix<T, Dynamic, Dynamic> & v){
    std::vector<std::vector<T>> temp(v.rows());
    for(int i=0;i<v.rows();i++){
      temp[i].resize(v.cols());
      for(int j=0;j<v.cols();j++){
        temp[i][j]=v(i,j);
      }
    }
    j=json(temp);
  }

  template<class T> void from_json(const json& j, Matrix<T, Dynamic, Dynamic> & v) {
    std::vector<std::vector<T>> temp=j.get<std::vector<std::vector<T>>>();

    if(temp[0].size()==0){
      std::cerr<<"Error while loading Eigen Matrix from Json"<<std::endl;
      std::abort();
    }

    v.resize(temp.size(),temp[0].size());
    for(int i=0;i<temp.size();i++){
      for(int j=0;j<temp[i].size();j++){
        if(temp[i].size()!=temp[0].size()){
          std::cerr<<"Error while loading Eigen Matrix from Json"<<std::endl;
          std::abort();
        }
        v(i,j)=temp[i][j];
      }
    }
  }

}

namespace std{

  void to_json(json& j, const std::complex<double>& p) {
    j = json{p.real(), p.imag()};
  }

  void from_json(const json& j, std::complex<double>& p) {
    if(j.is_array()){
      p=std::complex<double>(j[0].get<double>(),j[1].get<double>());
    }
    else{
      p=std::complex<double>(j.get<double>(),0.);
    }
  }

}

#endif
