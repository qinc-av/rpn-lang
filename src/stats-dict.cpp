/***************************************************
 * file: qinc/rpn-lang/src/stats-dict.cpp
 *
 * @file    stats-dict.cpp
 * @author  Eric L. Hernes
 * @born_on   Sunday, March 29, 2026
 * @copyright (C) Copyright Eric L. Hernes 2026
 * @copyright (C) Copyright Q, Inc. 2026
 *
 * @brief   Statistics words: MEAN, VARIANCE, STDDEV, LINFIT, CORRELATION, OLS.
 *
 * Basic stats (MEAN, VARIANCE, STDDEV, LINFIT, CORRELATION) operate on
 * stack::Vector.  OLS uses stack::Matrix (design matrix X) and stack::Vector
 * (dependent variable y) and returns a StJson result object.
 *
 */

#include "../rpn-matrix.h"

// ---------------------------------------------------------------------------
// Local validators
// ---------------------------------------------------------------------------
namespace stats_validator {
  const rpn::StrictTypeValidator d1_vector({typeid(::stack::Vector).hash_code()}, "d1_vector");
  const rpn::StrictTypeValidator d2_vector_vector({typeid(::stack::Vector).hash_code(),
                                                   typeid(::stack::Vector).hash_code()}, "d2_vector_vector");
  // OLS: ( matrix vec -- json )  TOS=vector, NOS=matrix → _types[0]=Vector, _types[1]=Matrix
  const rpn::StrictTypeValidator d2_matrix_vector({typeid(::stack::Vector).hash_code(),
                                                   typeid(::stack::Matrix).hash_code()}, "d2_matrix_vector");
}

#define STATS_WDEF(validator, fn) { stats_validator::validator, NATIVE_WORD_FN(stats, fn), nullptr }

// ---------------------------------------------------------------------------
// MEAN  ( vec -- mean )
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(stats, mean) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(::stack::Vector, *sv);
  rpn.stack.push_double(v.vec().mean());
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// VARIANCE  ( vec -- var )  sample variance (N-1 denominator)
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(stats, variance) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(::stack::Vector, *sv);
  if (v.size() < 2) return rpn::WordDefinition::Result::param_error;
  double mu = v.vec().mean();
  double var = (v.vec().array() - mu).square().sum() / (v.size() - 1);
  rpn.stack.push_double(var);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// STDDEV  ( vec -- stddev )  sample standard deviation
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(stats, stddev) {
  auto sv = rpn.stack.pop();
  const auto &v = PEEK_CAST(::stack::Vector, *sv);
  if (v.size() < 2) return rpn::WordDefinition::Result::param_error;
  double mu = v.vec().mean();
  double var = (v.vec().array() - mu).square().sum() / (v.size() - 1);
  rpn.stack.push_double(std::sqrt(var));
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// CORRELATION  ( vec_x vec_y -- r )  Pearson correlation coefficient
// NOS = x, TOS = y
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(stats, correlation) {
  auto sy = rpn.stack.pop();   // TOS: y
  auto sx = rpn.stack.pop();   // NOS: x
  const auto &xv = PEEK_CAST(::stack::Vector, *sx);
  const auto &yv = PEEK_CAST(::stack::Vector, *sy);
  if (xv.size() != yv.size() || xv.size() < 2) return rpn::WordDefinition::Result::param_error;
  double mx = xv.vec().mean(), my = yv.vec().mean();
  Eigen::VectorXd xc = xv.vec().array() - mx;
  Eigen::VectorXd yc = yv.vec().array() - my;
  double denom = std::sqrt(xc.squaredNorm() * yc.squaredNorm());
  rpn.stack.push_double(denom > 0.0 ? xc.dot(yc) / denom : 0.0);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// LINFIT  ( vec_x vec_y -- slope intercept )  simple OLS: y = intercept + slope*x
// NOS = x, TOS = y; result: NOS = intercept, TOS = slope
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(stats, linfit) {
  auto sy = rpn.stack.pop();   // TOS: y
  auto sx = rpn.stack.pop();   // NOS: x
  const auto &xv = PEEK_CAST(::stack::Vector, *sx);
  const auto &yv = PEEK_CAST(::stack::Vector, *sy);
  if (xv.size() != yv.size() || xv.size() < 2) return rpn::WordDefinition::Result::param_error;
  int n = (int)xv.size();
  Eigen::MatrixXd X(n, 2);
  X.col(0) = Eigen::VectorXd::Ones(n);
  X.col(1) = xv.vec();
  Eigen::VectorXd beta = (X.transpose() * X).inverse() * (X.transpose() * yv.vec());
  rpn.stack.push_double(beta(0));   // intercept (NOS)
  rpn.stack.push_double(beta(1));   // slope     (TOS)
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// OLS  ( matrix_X vec_y -- json )
// X is N×K design matrix (caller adds intercept column if desired).
// y is N-element vector.
// Result JSON: {beta, bse, bt, yhat, resid, ess, rss, tss, rsq, adjrsq, ser, df}
// ---------------------------------------------------------------------------
NATIVE_WORD_DECL(stats, ols) {
  auto sy = rpn.stack.pop();   // TOS: y
  auto sX = rpn.stack.pop();   // NOS: X
  const auto &yv = PEEK_CAST(::stack::Vector, *sy);
  const auto &Xm = PEEK_CAST(::stack::Matrix, *sX);
  int N = (int)yv.size(), K = (int)Xm.cols();
  if ((int)Xm.rows() != N) return rpn::WordDefinition::Result::param_error;
  if (N <= K) return rpn::WordDefinition::Result::param_error;  // need N > K for df > 0

  const Eigen::MatrixXd &X = Xm.mat();
  const Eigen::VectorXd &y = yv.vec();

  Eigen::MatrixXd XtXi = (X.transpose() * X).inverse();
  Eigen::VectorXd beta  = XtXi * (X.transpose() * y);

  double ybar = y.mean();
  Eigen::VectorXd yhat  = X * beta;
  Eigen::VectorXd resid = y - yhat;

  double ess = resid.squaredNorm();
  double rss = (yhat.array() - ybar).square().sum();
  double tss = (y.array() - ybar).square().sum();
  double rsq = (tss > 0.0) ? rss / tss : 0.0;
  double df  = N - K;
  double adjrsq = 1.0 - (1.0 - rsq) * (N - 1) / df;
  double s2  = ess / df;
  double ser = std::sqrt(s2);

  Eigen::VectorXd bse(K), bt(K);
  for (int i = 0; i < K; i++) {
    bse(i) = std::sqrt(XtXi(i, i) * s2);
    bt(i)  = (bse(i) > 0.0) ? beta(i) / bse(i) : 0.0;
  }

  stack::Object result;
  result.add_value("beta",   ::stack::Vector(beta));
  result.add_value("bse",    ::stack::Vector(bse));
  result.add_value("bt",     ::stack::Vector(bt));
  result.add_value("yhat",   ::stack::Vector(yhat));
  result.add_value("resid",  ::stack::Vector(resid));
  result.add_value("ess",    stack::Double(ess));
  result.add_value("rss",    stack::Double(rss));
  result.add_value("tss",    stack::Double(tss));
  result.add_value("rsq",    stack::Double(rsq));
  result.add_value("adjrsq", stack::Double(adjrsq));
  result.add_value("ser",    stack::Double(ser));
  result.add_value("df",     stack::Double(df));
  rpn.stack.push(result);
  return rpn::WordDefinition::Result::ok;
}

// ---------------------------------------------------------------------------
// addStatsWords
// ---------------------------------------------------------------------------
void
rpn::Interp::addStatsWords() {
  setWordCategory("stats");

  addDefinition("MEAN",        STATS_WDEF(d1_vector,        mean));
  addDefinition("VARIANCE",    STATS_WDEF(d1_vector,        variance));
  addDefinition("STDDEV",      STATS_WDEF(d1_vector,        stddev));
  addDefinition("CORRELATION", STATS_WDEF(d2_vector_vector, correlation));
  addDefinition("LINFIT",      STATS_WDEF(d2_vector_vector, linfit));
  addDefinition("OLS",         STATS_WDEF(d2_matrix_vector, ols));

  addWordMetadata("MEAN",        "Arithmetic mean of a vector.");
  addWordMetadata("VARIANCE",    "Sample variance of a vector (N-1 denominator).");
  addWordMetadata("STDDEV",      "Sample standard deviation of a vector.");
  addWordMetadata("CORRELATION", "Pearson correlation coefficient. `vec_x vec_y CORRELATION`.");
  addWordMetadata("LINFIT",      "Simple linear regression. `vec_x vec_y LINFIT` → intercept slope.");
  addWordMetadata("OLS",         "OLS regression. `matrix_X vec_y OLS` → JSON {beta,bse,bt,yhat,resid,ess,rss,tss,rsq,adjrsq,ser,df}.");

  setWordCategory("");
}

/* end of qinc/rpn-lang/src/stats-dict.cpp */
