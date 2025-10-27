import matplotlib
matplotlib.use('Agg')  # Use non-interactive backend
import matplotlib.pyplot as plt
import numpy as np
import sys

try:
    # Load data
    data = np.loadtxt('data/RD/h_noisy_interp/1761519372_Secondary_Cases.csv', delimiter=',')
    
    # Handle 1D array (single human)
    if data.ndim == 1:
        data = data.reshape(1, -1)
    
    print(f'Data shape: {data.shape}')
    
    # Create boxplot
    plt.figure(figsize=(8, 6))
    plt.boxplot(data.T)
    plt.xlabel('Human Agent ID')
    plt.ylabel('Secondary Cases')
    plt.title('Secondary Cases by ID (n=10 trials)')
    
    num_humans = data.shape[0]
    plt.xticks(range(1, num_humans + 1), range(num_humans))
    
    plt.savefig('data/RD/h_noisy_interp/1761519372_Secondary_Cases.png', dpi=300, bbox_inches='tight')
    print('Plot saved: data/RD/h_noisy_interp/1761519372_Secondary_Cases.png')
    
except Exception as e:
    print(f'Error: {e}', file=sys.stderr)
    sys.exit(1)
