from matplotlib import pyplot as plt

with open("data") as file:
    data = [i.replace("\n", "").split(" ") for i in file]

X = []
Y = []
for item in data:
    X.append(int(item[0]))
    Y.append(float(item[1]))

plt.axis([0, 55, 0, 3])
plt.plot(X, Y)
plt.show()