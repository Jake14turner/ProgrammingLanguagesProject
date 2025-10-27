import matplotlib.pyplot as plt
import numpy as np

# Load data
data = np.loadtxt('data/RD/h_noisy_interp/1761519225_Human_Hazard___Sickness.csv', delimiter=',')

# Create boxplot
plt.figure(figsize=(8, 6))
plt.boxplot(data.T)
plt.xlabel('Human Agent ID')
plt.ylabel('Human Hazard @ Sickness')
plt.title('Human Hazard @ Sickness by ID (n=10 trials)')
num_humans = data.shape[0]
plt.xticks(range(1, num_humans + 1), range(num_humans))
plt.savefig('data/RD/h_noisy_interp/1761519225_Human_Hazard___Sickness.png', dpi=300, bbox_inches='tight')
print('Plot saved: data/RD/h_noisy_interp/1761519225_Human_Hazard___Sickness.png')
