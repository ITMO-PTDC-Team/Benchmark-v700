# PLOTTING

Use [argument_reader.py](plotting/argument_reader.py) to benchmark and then plot results.

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

```shell
python3 argument_reader.py -f plotter_example.json
```

NOTE: the process, that script will launch, will work from `cpp/microbench` directory. Set up your parameters in JSON file accordingly (only affects `compiled-path` currently) according to this.

## Troubleshooting

If some errors occur while launching because of OS, try this:

```shell
sudo sysctl kernel.perf_event_paranoid=1
```
