# PLOTTING

Use [plotter.py](plotting/plotter.py) to benchmark and then plot results.

Before launching the scripts - install required dependencies via command:

```shell
pip install --upgrade -r requirements.txt
pip install --upgrade -r test_requirements.txt
```

## Setting the JSON parameters

Before launching the script, you should set up the parameters for it. 

| Parameter | Description | Example Value | Default Value | Required |
|-----------|-------------|---------------|---------------|----------|
| `folder` | Output directory for plots and results | `"tests-plots"` | - | **Yes** |
| `json-file-input` | Name of the input file for benchmark | `"test.json"` | - | **Yes** |
| `iterations` | Number of test iterations to average between | `5` | `1` | No |
| `timeout` | Time (in seconds) after which benchmark will shutdown forcibly. Defaulted to 10000 for testing purposes | `5` | `10000` | No |
| `competitors` | Data structures to compare (array of strings) | `["aksenov_splaylist_64"]` | - | **Yes** |
| `keys` | Changeable input file parameters to test (object with key-value pairs) | See below | - | **Yes** |
| `key_title` | X-axis label | `"NumberOfThreads"` | - | **Yes** |
| `keys_title` | X-axis tick labels (array of strings) | `["2", "4", "8", "16"]` | - | **Yes** |
| `allocator` | Memory allocator to use | `"libmimalloc"` | `""` | No |
| `compiled_path` | Path to compiled binaries | `"../build/"` | `"./bin/"` | No |
| `agg_stat` | Aggregation statistic for results | `"average_num_operations_total"` | `"average_num_operations_total"` | No |
| `ylabel` | Y-axis label for the plot | `"Avg operations"` | `"Average number of operations total"` | No |
| `yscale` | Y-axis scale type | `"log"` | `"linear"` | No |


### Competitors Configuration

The `competitors` parameter should be an array of objects, each containing:

```json
"competitors": [
    {
        "bin-name": "aksenov_splaylist_64.debra",
        "display-name": "Aksenov Splaylist",
        "data-structure-arguments": {
            // Optional: specific arguments for this data structure
            "id": "custom_id",
            "other_param": "value"
        }
    }
]
```

### Keys

Each element in the list corresponding to a value, that would be substituted in the `input` JSON file by the `keys.name` path during the runs of a benchmark. The number of times benchmark would be launched equals to `len(keys_title) * len(competitor)`. 

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
python3 plotter.py -f plotter_example.json
```

NOTE: the process, that script will launch, will work from `cpp/microbench` directory. Set up your parameters in JSON file accordingly (only affects `compiled-path` currently) according to this.

NOTE 2: The `logs` folder will be created under your plotting folder for potential error information during runs

## Troubleshooting

If some errors occur while launching because of OS, try this:

```shell
sudo sysctl kernel.perf_event_paranoid=1
```

# TESTING

```shell
python3 -m pytest run_tests.py -v
```

NOTE: For the final test, you must have `aksenov_splaylist_64.debra` built and located in `microbench/bin` directory.

By default, tests will clean their temporary output directories (with the exception of logs), but you can change that by setting `CLEAN_OUTPUT_DIRECTORY` environmental variable during launch to `False`.