import matplotlib.pyplot as plt

fig, ax = plt.subplots()

x = []
y = []
labels = []
for i in range(len(vega_tree)):
    for ii in range(len(vega_tree[i])):
        x.append(i+1)
        y.append(value_tree[i][ii])
        labels.append(vega_tree[i][ii])

scatter = ax.scatter(x, y, c=labels, cmap='coolwarm', s=300, edgecolors='black')
plt.colorbar(scatter, label='Label Value')

for i in range(len(x)):
    ax.text(x[i], y[i], f'{labels[i]:.2f}', color='black', ha='center', va='center', fontsize=8)

ax.set_xlabel('Steps')
ax.set_ylabel('Asset Value')
ax.set_title('Binomial Model - Vega Plot')
plt.show()