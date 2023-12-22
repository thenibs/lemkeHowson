#include <iostream>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <numeric>
#include <boost/rational.hpp>
using namespace std;
typedef boost::rational<int> Rational;

// Simple matrix class to contain Rational entries
class Matrix {
private:
    vector<vector<Rational>> data;

public:
    // default constructor
    Matrix(int numRows, int numCols) : data(numRows, vector<Rational>(numCols, 0)) {}

    // Constructor that initializes a matrix with given dimensions and values
    Matrix(int numRows, int numCols, const vector<Rational>& values) {
        if (values.size() != static_cast<long unsigned int>(numRows * numCols)) {
            throw invalid_argument("The size of values does not match the specified dimensions");
        }

        data.resize(numRows);
        for (int i = 0; i < numRows; i++) {
            data[i].resize(numCols);
            for (int j = 0; j < numCols; j++) {
                data[i][j] = values[i * numCols + j];
            }
        }
    }

    // set Matrix[i][j]
    void set(int row, int col, const Rational& value) {
        if (static_cast<long unsigned int>(row) < 0 || static_cast<long unsigned int>(row) >= data.size()
                                            || col < 0 || static_cast<long unsigned int>(col) >= data[0].size()) {
            throw out_of_range("Matrix indices are out of range");
        }
        data[row][col] = value;
    }

    // get Matrix[i][j]
    Rational get(int row, int col) const {
        if (static_cast<long unsigned int>(row) < 0 || static_cast<long unsigned int>(row) >= data.size()
                                            || col < 0 || static_cast<long unsigned int>(col) >= data[0].size()) {
            throw out_of_range("Matrix indices are out of range");
        }
        return data[row][col];
    }

    // get dims
    int getRows() const { return data.size(); }
    int getCols() const { return data[0].size(); }

    // output override
    friend ostream& operator<<(ostream& os, const Matrix& matrix) {
        int numRows = matrix.getRows();
        int numCols = matrix.getCols();

        int fieldWidth = 6;
        os << "[ ";
        for (int i = 0; i < numRows; i++) {
            if (i != 0) {
                os << "  ";
            }
            for (int j = 0; j < numCols; j++) {
                if (matrix.get(i, j).denominator() == 1) {
                    os << right << setw((fieldWidth + to_string(matrix.get(i, j).numerator()).length()) / 2) << matrix.get(i, j).numerator() << " ";
                } else {
                    string val = to_string(matrix.get(i, j).numerator()) + "/" + to_string(matrix.get(i, j).denominator());
                    os << right << setw((fieldWidth + val.length()) / 2) << val << " ";
                }
            }
            if (i < numRows-1) {
                os << endl;
            }
        }
        os << "]" << endl;

        return os;
    }
};


/*
    Normalize the matrices by removing all negative values by finding 
    the minimum negative value and increasing all values based on that minimum
*/
void normalizeMatrices(Matrix& m1, Matrix& m2) {
    // Find the lowest value in both matrices
    Rational lowestVal = m1.get(0, 0);
    for (int i = 0; i < m1.getRows(); i++) {
        for (int j = 0; j < m1.getCols(); j++) {
            lowestVal = min(lowestVal, m1.get(i, j));
            lowestVal = min(lowestVal, m2.get(i, j));
        }
    }

    // Calculate the constant to add
    Rational val = lowestVal > 0 ? 0 : abs(lowestVal) + 1;

    // update matrices based on val
    for (int i = 0; i < m1.getRows(); i++) {
        for (int j = 0; j < m1.getCols(); j++) {
            m1.set(i, j, m1.get(i, j) + val);
            m2.set(i, j, m2.get(i, j) + val);
        }
    }
}


/*
    Initializes the tableaux based on 2 equal dimension matrices
    Adds 2 columns for the basis variables while pivoting
*/
Matrix initTableaux(const Matrix& m1, const Matrix& m2) {
    if (m1.getRows() != m2.getRows() || m1.getCols() != m2.getCols()) {
        throw invalid_argument("Matrices must have the same dimensions");
    }

    int totalStrats = m1.getRows() + m1.getCols();
    Matrix tableaux(totalStrats, totalStrats + 2);

    // Init the first extra column (basis variable) to negative sequence
    for (int i = 0; i < totalStrats; i++) {
        tableaux.set(i, 0, -i-1);
    }

    // Init the second column (init value of basis variables) to 1s
    for (int i = 0; i < totalStrats; i++) {
        tableaux.set(i, 1, 1);
    }

    // Fill in tableaux with values from the two matrices
    // Values from m1
    for (int i = 0; i < m1.getRows(); i++) {
        for (int j = 0; j < m1.getCols(); j++) {
            tableaux.set(i, m1.getRows() + j + 2, -m1.get(i, j));
        }
    }

    // Values from m2
    for (int i = 0; i < m1.getCols(); i++) {
        for (int j = 0; j < m1.getRows(); j++) {
            tableaux.set(m1.getRows() + i, j + 2, -m2.get(j, i));
        }
    }

    return tableaux;
}


/*
    Performs a pivot operation on a given tableaux to find a leaving variable
*/
int pivot(Matrix& tableaux, int p1SCount, int enteringVar) {
    // check for valid variables and number of strats
    if (abs(enteringVar) <= 0 || abs(enteringVar) > tableaux.getRows()) {
        throw invalid_argument("Selected variable index is invalid.");
    }
    if (p1SCount <= 0 || p1SCount >= tableaux.getRows()) {
        throw invalid_argument("Invalid number of strategies for player 1.");
    }

    // Adjust for the additional column in tableaux
    auto varToCol = [&](int var) -> int {
        return 1 + abs(var);
    };

    // get set of row numbers for given variable
    auto getRowNums = [&](int var) -> vector<int> {
        vector<int> rows;
        if ((-p1SCount <= var && var < 0) || (var > p1SCount)) {
            for (int i = 0; i < p1SCount; ++i) rows.push_back(i);
        } else {
            for (int i = p1SCount; i < tableaux.getRows(); ++i) rows.push_back(i);
        }
        return rows;
    };

    // find minimum ratio for next pivoting
    int leavingVar = 0;
    Rational minRatio = Rational(INT_MAX);
    int leavingVarRow = -1;
    for (int i : getRowNums(enteringVar)) {
        if (tableaux.get(i, varToCol(enteringVar)) < 0) {
            Rational ratio = (-tableaux.get(i, 1), tableaux.get(i, varToCol(enteringVar)));
            if (minRatio == Rational(INT_MAX) || ratio < minRatio) {
                minRatio = ratio;
                leavingVar = tableaux.get(i, 0).numerator();
                leavingVarRow = i;
            }
        }
    }

    if (leavingVarRow == -1) {
        throw runtime_error("No valid pivot found.");
    }

    // Update the pivot row
    tableaux.set(leavingVarRow, 0, enteringVar);
    tableaux.set(leavingVarRow, varToCol(leavingVar), -1);
    Rational pivotElement = tableaux.get(leavingVarRow, varToCol(enteringVar));
    for (int j = 1; j < tableaux.getCols(); ++j) {
        tableaux.set(leavingVarRow, j, tableaux.get(leavingVarRow, j) / abs(pivotElement));
    }
    tableaux.set(leavingVarRow, varToCol(enteringVar), 0);

    // Update other rows
    for (int i : getRowNums(enteringVar)) {
        if (tableaux.get(i, varToCol(enteringVar)) != 0) {
            for (int j = 1; j < tableaux.getCols(); j++) {
                Rational multiplier = tableaux.get(i, varToCol(enteringVar));
                tableaux.set(i, j, tableaux.get(i, j) + multiplier * tableaux.get(leavingVarRow, j));
            }
            tableaux.set(i, varToCol(enteringVar), 0);
        }
    }

    return leavingVar;
}


/*
    Extracts equilibrium strategies from the tableaux
*/
void getEQ(const Matrix& tableaux, int p1SCount, vector<Rational>& eq1, vector<Rational>& eq2) {
    vector<int> firstColNums(tableaux.getRows());
    for (int i = 0; i < tableaux.getRows(); ++i) {
        firstColNums[i] = abs(tableaux.get(i, 0).numerator());
    }

    for (int i = 1; i <= tableaux.getRows(); ++i) {
        if (find(firstColNums.begin(), firstColNums.end(), i) == firstColNums.end()) {
            throw invalid_argument("Invalid indices in the first column of the tableaux.");
        }
    }

    // Get the proper probabilities within interval, otherwise 0
    vector<Rational> eqs(tableaux.getRows(), Rational(0));
    for (int i = 0; i < tableaux.getRows(); ++i) {
        int strat = tableaux.get(i, 0).numerator();
        Rational prob = tableaux.get(i, 1);
        eqs[abs(strat) - 1] = (strat < 0 || prob < 0) ? Rational(0) : prob;
    }

    vector<Rational> player1Strategy(eqs.begin(), eqs.begin() + p1SCount);
    vector<Rational> player2Strategy(eqs.begin() + p1SCount, eqs.end());

    eq1 = player1Strategy;
    eq2 = player2Strategy;
}


/*
    Normalizes the equilbrium strategy vectors
*/
void normalizeEQ(vector<Rational>& player1Strategy, vector<Rational>& player2Strategy) {
    auto normalizeStrategy = [](vector<Rational>& strategy) {
        Rational probSum = accumulate(strategy.begin(), strategy.end(), Rational(0));
        for (auto& prob : strategy) {
            prob *= probSum.denominator();
            prob /= probSum.numerator();
        }
    };

    normalizeStrategy(player1Strategy);
    normalizeStrategy(player2Strategy);
}


/*
    Requires equal size vectors and a non-degenerate game
    Algorithm Steps:
        1. Preprocessing: Normalize matrices
        2. Initialization of the tableaux
        3. Repeated pivoting
        4. Recover Nash equilibrium from final tableaux and normalize
*/
void lemkeHowson(const Matrix& m1OG, const Matrix& m2OG, vector<Rational>& eq1, vector<Rational>& eq2) {
    // Step 1
    Matrix m1 = m1OG;
    Matrix m2 = m2OG;
    normalizeMatrices(m1,m2);

    // Step 2
    Matrix tableaux = initTableaux(m1,m2);

    // Step 3
    // pivot until pivoted var is the same as init var 1
    int pivotVar = pivot(tableaux, m1.getRows(), 1);
    while (abs(pivotVar) != 1) {
        pivotVar = pivot(tableaux, m1.getRows(), -pivotVar);
    }

    // Step 4
    getEQ(tableaux, m1.getRows(), eq1, eq2);
    normalizeEQ(eq1, eq2);
}


int main() {
    // Read dimensions of the matrices
    int rows, cols;
    cin >> rows >> cols;

    // Read values for the first matrix
    vector<Rational> values1;
    for (int i = 0; i < rows * cols; ++i) {
        int numerator;
        cin >> numerator;
        values1.push_back(Rational(numerator, 1));
    }

    // Read values for the second matrix
    vector<Rational> values2;
    for (int i = 0; i < rows * cols; ++i) {
        int numerator;
        cin >> numerator;
        values2.push_back(Rational(numerator, 1));
    }

    // Construct Matrix objects
    Matrix m1(rows, cols, values1);
    Matrix m2(rows, cols, values2);

    cout << "Player 1 Matrix\n" << m1 << "\nPlayer 2 Matrix\n" << m2 << endl;

    // store result in eq1, eq2
    vector<Rational> eq1, eq2;
    lemkeHowson(m1, m2, eq1, eq2);

    // output results
    cout << "Equilibrium for Player 1" << endl;
    for (auto val : eq1) {
        if (val.denominator() == 1) {
            cout << val.numerator() << " ";
        } else {
            cout << val << " ";
        } 
    }
    cout << "\n\nEquilibrium for Player 2" << endl;
    for (auto val : eq2) {
        if (val.denominator() == 1) {
            cout << val.numerator() << " ";
        } else {
            cout << val << " ";
        } 
    }
    cout << endl;

    return 0;
}
