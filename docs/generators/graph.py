import pandas as pd
from matplotlib import pyplot as plt


d = {'Сервер nginx': [0.35, 1.017, 1.592, 2.488, 2.878], 
'Сервер с 1-м процессом': [0.952,1.914,2.769,3.438,4.673],
'Сервер с 10-ю процессами': [1.107, 1.644, 2.333, 2.908, 3.426]}
df = pd.DataFrame(d, index=[2000, 4000, 6000, 8000, 10000])
df.plot(style=['-o', '-rx', '-gv'], use_index=True)
plt.xlabel('Количество запросов')
plt.ylabel('Общее время тестирования, с')
plt.savefig("../inc/img/graph-01.pdf", format="pdf", bbox_inches="tight")

d = {'Сервер nginx': [0.175, 0.254, 0.265, 0.311, 0.288], 
'Сервер с 1-м процессом': [0.476, 0.478, 0.461, 0.43, 0.467],
'Сервер с 10-ю процессами': [0.554, 0.411, 0.389, 0.364, 0.343]}
df = pd.DataFrame(d, index=[2000, 4000, 6000, 8000, 10000])
df.plot(style=['-o', '-rx', '-gv'], use_index=True)
plt.xlabel('Количество запросов')
plt.ylabel('Среднее время выполнения одного запроса, мс')
plt.savefig("../inc/img/graph-02.pdf", format="pdf", bbox_inches="tight")
