#include <vector>

// column-major matrix
struct Matrix {

  std::vector<double> m;
  int rows, cols;

  Matrix(int rows, int cols) : rows(rows), cols(cols), m(rows * cols, 0.0) {}
  double &operator()(int row, int col) { return m[col * rows + row]; }
  const double &operator()(int row, int col) const {
    return m[col * rows + row];
  }
};



// copy copy copy no value copy copy copy no value copy copy copy no value copy copy copy no value copy copy copy no value
Matrix multiply(const Matrix &a, const Matrix &b) {
  if (a.cols != b.rows) {
    throw std::invalid_argument(
        "Matrix dimensions do not match for multiplication.");
  }

  Matrix result(a.rows, b.cols);

  for (int i = 0; i < a.rows; ++i) {
    for (int j = 0; j < b.cols; ++j) {
      for (int k = 0; k < a.cols; ++k) {
        result(i, j) += a(i, k) * b(k, j);
      }
    }
  }

  return result;
}

