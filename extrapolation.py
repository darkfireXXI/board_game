import matplotlib.pyplot as plt
import numpy as np

data_with_border = {
    1: 1,  # 1e0
    2: 23,  # 1e1
    3: 20_389,  # 1e4
    4: 207_016_289,  # 1e8
    # 5: 215_000_000_000_000_000,
}

data_no_border = {
    1: 1,  # 1e0
    2: 23,  # 1e1
    3: 22_050,  # 1e4
    4: 234_000_000,  # 1e8
}

data = {
    "Board With Border": data_with_border,
    "Board No Border": data_no_border,
}

for data_set, data in data.items():
    x_data = np.array(list(data.keys()))
    y_data = np.array(list(data.values()))

    log_y = np.log(y_data)

    b_fit, log_a_fit = np.polyfit(x_data, log_y, 1)
    a_fit = np.exp(log_a_fit)

    x_predict = 10
    y_predict = a_fit * np.exp(b_fit * x_predict)
    print(f"Predicted value at x = 10: {y_predict:.2f}")

    x_fit = np.linspace(1, 10, 100)
    y_fit = a_fit * np.exp(b_fit * x_fit)

    plt.scatter(x_data, y_data, label=data_set + " Empirical", color="red", s=11)
    label = f"Fitted: y = {a_fit:.2e} * exp({b_fit:.2f}x)"
    plt.plot(x_fit, y_fit, label=data_set + " Extrapolated", color="black", linestyle="--", linewidth=0.66)

    plt.yscale("log")

plt.xlabel("Board Dimension")
plt.ylabel("No. of Permutations")
plt.title("Board Permutations: Exponential Fit Using Log-Linear Method")
plt.legend()
plt.grid(True)
plt.show()
