import matplotlib.pyplot as plt

fig, ax = plt.subplots()
for i in range(len(v)-1):
    tmp=[x for x in v[i] for _ in (0, 1)]
    for ii in range(len(tmp)):
        ax.plot([i,i+1], [tmp[ii], v[i+1][ii]], color='black', label='Price Path')
        ax.axhline(v[i+1][ii], color='grey', linestyle='dotted', linewidth=0.5, label='Outcomes')

ax.axhline(strike, color='red', linestyle='dashed', linewidth=0.8, label='Strike')
ax.set_title('Binomial Model')
ax.set_xlabel('Steps')
ax.set_ylabel('Asset Price')

handles, labels = plt.gca().get_legend_handles_labels()
lgnd = dict(zip(labels, handles))
ax.legend(lgnd.values(), lgnd.keys())
plt.show()