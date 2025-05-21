# PLOTTING

Use [plotter.py](plotting/plotter.py) to benchmark and then plot results. If you want to look into results as a table, consider [exp_table_builder.py](plotting/exp_table_builder.py) and [all_exp_table_builder.py](plotting/all_exp_table_builder.py).

Before launching the scripts - install required dependecies via command:

```shell
pip install --upgrade -r requirements.txt
```

To get more info about laucnhing the scripts use `-h` option.

## Setting the JSON parameters

Before launching the script, you should set up the parameters for it. 

| Parameter            | Description                                  | Example Value                     |
|----------------------|----------------------------------------------|-----------------------------------|
| `folder`             | Output directory for plots and results       | `"tests-plots"`                   |
| `json-file-input`    | Name of the input file for benchmark         | `"test.json"`                     |
| `iterations`         | Number of test iterations to average between | `5`                               |
| `competitors`        | Data structures to compare                   | `aksenov_splaylist_64`            |
| `keys`               | Changeable input file parameters to          | See below                         |
| `key_title`          | X-axis label                                 | `"NumberofThreads"`               |
| `keys_title`         | X-axis tick labels                           | `["2", "4", "8", "16"]`           |


### Keys

Each element in the list corresponding to a value, that would be substitued in the `input` JSON file by the `keys.name` path during the runs of a benchmark. The number of times benchmark would be launched equals to `len(keys_title) * len(competitor)`. 

Please note, that `len(keys_title)` should be equal to the `len(keys.values)` for all values.

`test.threadLoopBuilders.0.quantity` with values: `[1, 2, 4, 8]` means that for the first run, the value `test.threadLoopBuilders[0].quantity` in the `test.json` file would be set to `1`.

## Running

Example JSON file for script settings: [plotter_example.json](plotting/plotter_example.json)

The script support following flags:

| Parameter            | Description                                                                      | Example Value                     |
|----------------------|----------------------------------------------------------------------------------|-----------------------------------|
| `--file, -f`         | Path to the file containing information about launch params                      | `"plotter_example.json"`          |        
| `--title, -t`        | Title of the resulting graph                                                     | `"aksenov_srivastava_comp"`       |
| `--pathg, -pg`       | Path for the resulting graph                                                     | `./tests-plots/graph`                               |        
| `-no-run`            | Store-only flag for benchmark running before plot. Use to omit run               | `-----`                           |

plotter.py: (uniform and x/y distributions):
```shell
python3 argument_reader.py -f plotter_example.json
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
