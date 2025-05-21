# PLOTTING

Use [plotter.py](plotting/plotter.py) to benchmark and then plot results. If you want to look into results as a table, consider [exp_table_builder.py](plotting/exp_table_builder.py) and [all_exp_table_builder.py](plotting/all_exp_table_builder.py).

Before launching the scripts - install required dependecies via command:

```shell
pip install --upgrade -r requirements.txt
```

To get more info about laucnhing the scripts use `-h` option.

## Examples

plotter.py: (uniform and x/y distributions):
```shell
python3 plotter.py --stat find-throughput update-throughput total-throughput rq-throughput --nprocess 9 --ds redis_zset redis_sait redis_sabt redis_sabpt redis_salt --workload dist-uniform "skewed-sets -rs 0.3 -rp 0.7 -ws 0.3 -wp 0.7 -inter 0.3" "skewed-sets -rs 0.2 -rp 0.8 -ws 0.2 -wp 0.8 -inter 0.2" "skewed-sets -rs 0.1 -rp 0.9 -ws 0.1 -wp 0.9 -inter 0.1" "skewed-sets -rs 0.05 -rp 0.95 -ws 0.05 -wp 0.95 -inter 0.05" "skewed-sets -rs 0.01 -rp 0.99 -ws 0.01 -wp 0.99 -inter 0.01" --workload-name uniform 70-30 80-20 90-10 95-05 99-01 --insdelrq 0.0/0.0/1.0 0.3/0.3/0.4 0.2/0.2/0.6 0.0/0.0/0.0 0.3/0.3/0.0 0.2/0.2/0.0 -k 10000 100000 1000000 5000000 -ps 5000 50000 500000 2500000 --time 20000 --fig-size 6,6 --color blue green red purple orange -o plotter-output-root
```

plotter.py (zipf distribution):
```shell
python3 plotter.py --stat find-throughput update-throughput total-throughput rq-throughput --nprocess 3 --ds redis_zset redis_sait redis_sabt redis_sabpt redis_salt --workload "dist-zipf 1" --workload-name zipf-1 --insdelrq 0.0/0.0/1.0 0.3/0.3/0.4 0.2/0.2/0.6 0.0/0.0/0.0 0.3/0.3/0.0 0.2/0.2/0.0  --key 10000 100000 1000000 5000000 --prefill-size 5000 50000 500000 2500000 --prefill-sequential --time 20000 --fig-size 6,6 --color blue green red purple orange -o plotter-output-root --avg 5
```

exp_table_builder.py:
```shell
python3 exp_table_builder.py -pod plotter-output-root -s total_throughput -ds redis_zset redis_sait redis_sabt redis_sabpt redis_salt -w uniform 70-30 80-20 90-10 95-05 99-01 zipf-1 -b redis_zset -k 100000 -ops 0.0_0.0_0.0
```

all_exp_table_builder.py:
```shell
python3 all_exp_table_builder.py -etb "-pod plotter-output-root -s total_throughput -ds redis_zset redis_sait redis_sabt redis_sabpt redis_salt -w uniform 70-30 80-20 90-10 95-05 99-01 -b redis_zset" -k 10000 100000 1000000 5000000 -ops 0.0_0.0_0.0 0.0_0.0_1.0 0.2_0.2_0.0 0.2_0.2_0.6 0.3_0.3_0.0 0.3_0.3_0.4
```

## Troubleshooting

If some errors occur while launching because of OS, try this:

```shell
sudo sysctl kernel.perf_event_paranoid=1
```
