#include <RcppEigen.h>
#include "data_manipulation.h"


using namespace Rcpp;
using namespace Eigen;

// [[Rcpp::depends(RcppEigen)]]
// [[Rcpp::depends(RcppProgress)]]


// [[Rcpp::export]]
List CanonCor(Eigen::MatrixXd mat1, Eigen::MatrixXd mat2){
  mat1 = mat1.rowwise() - mat1.colwise().mean();
  mat2 = mat2.rowwise() - mat2.colwise().mean();
  
  Eigen::MatrixXd mat1_cov = FastCov(mat1, false);
  Eigen::MatrixXd mat2_cov = FastCov(mat2, false);
  Eigen::MatrixXd mat12_cov = FastCovMats(mat1, mat2, false);
  
  Eigen::MatrixXd h1 = mat1_cov.inverse().sqrt();
  Eigen::MatrixXd h2 = mat2_cov.inverse().sqrt();
  
  Eigen::MatrixXd k = h1 * mat12_cov * h2;
  Eigen::JacobiSVD<Eigen::MatrixXd> svd(k, ComputeThinU | ComputeThinV);
  
  Eigen::MatrixXd a = h1 * svd.matrixU();
  Eigen::MatrixXd b = h2 * svd.matrixV();
  List cc(3);
  cc[0] = svd.singularValues();
  cc[1] = (a.transpose() * mat1.transpose()).transpose();
  cc[2] = (b.transpose() * mat2.transpose()).transpose();
  return(cc);
}

// [[Rcpp::export]]
List CalcPartialCCA(Eigen::MatrixXd mat1, Eigen::MatrixXd mat2, int k, Eigen::MatrixXd v_init) {
  Eigen::MatrixXd u_mat(mat1.cols(), k);
  Eigen::MatrixXd v_mat(mat2.cols(), k);
  Eigen::VectorXd d(k);
  
  for (int i = 0; i < k; ++i){
    Eigen::VectorXd v = v_init.col(i);
    Eigen::VectorXd u = ((mat2 * v).transpose() * mat1).transpose();
    u = u.array() / u.norm();
    v = ((mat1 * u).transpose() * mat2).transpose();
    v = v.array() / v.norm();
    u_mat.col(i) = u;
    v_mat.col(i) = v;
    d[i] = ((mat1 * u).array() * (mat2 * v).array()).sum();
    mat1 = FastRBind(mat1, sqrt(d[i]) * u.transpose().array());
    mat2 = FastRBind(mat2, -1 * sqrt(d[i]) * v.transpose().array());
  }
  List cc(3);
  cc[0] = u_mat;
  cc[1] = v_mat;
  cc[2] = d;
  return(cc);
}