# Models 
This file explains the implementation of various models that we're using in the project.

## 1 . Almgren-Chriss Market Impact Model
The Almgren-Chriss model is used to estimate the market impact of a trade, which represents the price change caused by executing a large order. It decomposes the market impact into permanent and temporary components, reflecting the long-term and short-term effects of the trade on the market price.
Mathematical Formulation
The Almgren-Chriss model estimates the market impact $I$ as the sum of permanent and temporary impacts:
$$I = I_{\text{permanent}} + I_{\text{temporary}}$$

### Permanent Impact
The permanent impact reflects the lasting effect of the trade on the market price due to the information it conveys. It is modeled as a linear function of the trade size relative to the total market volume:
$$I_{\text{permanent}} = \gamma \cdot \left( \frac{Q}{V} \right) \cdot S$$

$ Q $: Trade quantity `(quantity_)`, the size of the order in BTC.  
$ V $: Total volume `(total_volume)`, the sum of quantities in the bid and ask sides of the order book.  
$ S $: Mid-price `(mid_price)`, the average of the best bid and ask prices.  
$ \gamma $: A constant parameter `(gamma = 1.0)`, representing the sensitivity of the price to the trade size relative to volume. This value is hardcoded but can be tuned based on historical data.  
```cpp
double permanent_impact = gamma * (quantity_ / total_volume) * mid_price;
```
### Temporary Impact
The temporary impact captures the short-term price distortion due to liquidity constraints, which dissipates after the trade. It is modeled as a power-law function of the trade rate:
$$ I_{\text{temporary}} = \eta \cdot \left( \frac{Q}{T} \right)^{\beta} \cdot S $$

$ Q $ Trade quantity `(quantity_)`.  
$ T $ Time horizon `(T = 240.0)`, representing the time (in minutes) over which the trade is executed. This is a hardcoded estimate but can be adjusted based on trading strategy.  
$ \eta $: A constant parameter `(eta = 0.05)`, representing the temporary impact coefficient. This value is hardcoded but can be calibrated.  
$ \beta $: A power-law exponent `(beta = 0.5)`, typically between 0 and 1, reflecting the non-linear impact of trade rate. A value of 0.5 implies a square-root relationship, common in market impact models.  
$ S $ : Mid-price `(mid_price)`.  
```cpp
double temporary_impact = eta * std::pow(quantity_ / T, beta) * mid_price;
expected_market_impact_ = permanent_impact + temporary_impact;
```
$$ S = \frac{\text{best bid} + \text{best ask}}{2} $$
Hardcoded values: $ \gamma = 1.0 $, $ \eta = 0.05 $, $T=240.0 $, $\beta = 0.5$.  
Can be tuned using historical data.

## 2. Regression Models for Slippage Estimation
Slippage, the difference between the expected and actual execution price of a trade, is estimated using regression models. Two approaches are implemented: a linear regression and a quantile regression, with the choice determined by the `use_quantile_regression_` flag.

### Linear Regression Model
The linear regression model predicts slippage as a function of the mid-price using a simple linear relationship:
$$ \text{slippage} = \beta_0 + \beta_1 \cdot S $$

$ S $: Mid-price (mid_price), the independent variable.  
$ \beta_0 $: Intercept (regression_coefficients_(0)).  
$ \beta_1 $: Slope (regression_coefficients_(1)), representing the sensitivity of slippage to the mid-price.  

The coefficients $ \beta_0 $ and $ \beta_1 $ are estimated using ordinary least squares (OLS) regression:
$$ \beta = (X^T X)^{-1} X^T y $$

$ X $: Design matrix with dimensions
$ N \times 2 $, where $ N $ is the number of valid data points. Each row is $ [1, S_i] $, where $ S_i $ is the mid-price.  
$ y $: Vector of slippage values `(historical_data_[i].second)`.  
$ \beta $ : Coefficient vector $ [\beta_0, \beta_1] $.  
```cpp
historical_data_.pop_front();
historical_data_.push_back({mid_price, expected_slippage_});
if (++update_count_ % 10 == 0) {
    if (use_quantile_regression_) {
        computeQuantileRegression();
    } else {
        computeRegression();
    }
}

computeRegression() {
    Eigen::MatrixXd X(historical_data_.size(), 2);
    Eigen::VectorXd y(historical_data_.size());
    size_t valid_count = 0;
    for (size_t i = 0; i < historical_data_.size(); ++i) {
        if (historical_data_[i].first != 0.0) {
            X(valid_count, 0) = 1.0;
            X(valid_count, 1) = historical_data_[i].first;
            y(valid_count) = historical_data_[i].second;
            ++valid_count;
        }
    }
    if (valid_count >= 10) {
        X.conservativeResize(valid_count, Eigen::NoChange);
        y.conservativeResize(valid_count);
        regression_coefficients_ = (X.transpose() * X).ldlt().solve(X.transpose() * y);
    }
    if (valid_count >= 10) {
        std::cout << "Linear Regression - Valid points: " << valid_count
                  << ", Intercept: " << regression_coefficients_(0)
                  << ", Slope: " << regression_coefficients_(1) << std::endl;
    } else {
        std::cout << "Linear Regression - Not enough valid points: " << valid_count << std::endl;
    }
}
```
### Quantile Regression Model
Mathematical Formulation
Quantile regression estimates the conditional quantile of slippage given the mid-price, providing a more robust estimate against outliers compared to linear regression. It models the $ \tau $-th quantile of slippage:
$$ Q_{\text{slippage}}(\tau | S) = \beta_0 + \beta_1 \cdot S$$

$ \tau $: Quantile parameter (quantile = 0.5 by default), representing the median when $ \tau = 0.5 $.  
$ S $: Mid-price.  
$ \beta_0, \beta_1 $: Coefficients to be estimated.  

Quantile regression minimizes the following loss function:
$$ \text{Loss} = \sum_{i: y_i \geq \hat{y}_i} \tau |y_i - \hat{y}_i| + \sum_{i: y_i \lt \hat{y}_i} (1 - \tau) |y_i - \hat{y}_i|$$

$ y_i $: Observed slippage (historical_data_[i].second).  
$ \hat{y}_i $: Predicted slippage 
($ \beta_0 + \beta_1 \cdot S_i $).  
Using gradient descent to minimize the quantile loss function.
```cpp
computeQuantileRegression(double quantile) {
    Eigen::MatrixXd X(historical_data_.size(), 2);
    Eigen::VectorXd y(historical_data_.size());
    size_t valid_count = 0;
    for (size_t i = 0; i < historical_data_.size(); ++i) {
        if (historical_data_[i].first != 0.0) {
            X(valid_count, 0) = 1.0;
            X(valid_count, 1) = historical_data_[i].first;
            y(valid_count) = historical_data_[i].second;
            ++valid_count;
        }
    }
    if (valid_count < 10) {
        std::cout << "Quantile Regression - Not enough valid points: " << valid_count << std::endl;
        return;
    }
    X.conservativeResize(valid_count, Eigen::NoChange);
    y.conservativeResize(valid_count);

    Eigen::VectorXd beta(2);
    beta.setZero();
    double learning_rate = 0.0001;
    int max_iterations = 1000;
    for (int iter = 0; iter < max_iterations; ++iter) {
        Eigen::VectorXd gradient(2);
        gradient.setZero();
        for (size_t i = 0; i < valid_count; ++i) {
            double prediction = beta(0) + beta(1) * X(i, 1);
            double error = y(i) - prediction;
            double indicator = (error < 0) ? (quantile - 1.0) : quantile;
            gradient(0) += -indicator;
            gradient(1) += -indicator * X(i, 1);
        }
        beta -= learning_rate * gradient;
        if (gradient.norm() < 1e-5) {
            break;
        }
    }
    regression_coefficients_ = beta;
    std::cout << "Quantile Regression (tau=" << quantile << ") - Valid points: " << valid_count
              << ", Intercept: " << regression_coefficients_(0)
              << ", Slope: " << regression_coefficients_(1) << std::endl;
}
```
 

## 3. Maker/Taker Proportion Prediction
The maker/taker proportion represents the fraction of trades executed as maker orders (providing liquidity) versus taker orders (consuming liquidity). This proportion affects fees, as maker fees are typically lower than taker fees.
Currently, the implementation uses a hardcoded value:
$\text{maker\_taker\_proportion\_} = 0.5$

This assumes that 50% of the trades are executed as maker orders and 50% as taker orders.
The value is not dynamically calculated based on market conditions or historical data.