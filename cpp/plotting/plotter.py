import abc
import argparse
from pathlib import Path
import subprocess
import os
from collections import defaultdict
import matplotlib.pyplot as plt
import json
import sys
import copy
import logging
import glob


DEFAULT_OUTPUT_DIR_NAME = "plotter-output"


def create_logger(name, log_file):
    logger = logging.getLogger(name)
    logger.setLevel(logging.DEBUG)

    handler = logging.FileHandler(log_file)
    handler.setLevel(logging.DEBUG)

    formatter = logging.Formatter(
        "%(asctime)s - %(levelname)s - %(name)s - %(message)s")
    handler.setFormatter(formatter)

    logger.addHandler(handler)
    return logger


def handle_file_error(exc: Exception, filepath: str = ""):
    if isinstance(exc, json.JSONDecodeError):
        print(f"Error: Invalid JSON or file read failed - '{filepath}'")
    elif isinstance(exc, FileNotFoundError):
        print(f"Error: File not found - '{filepath}'")
    elif isinstance(exc, KeyError):
        print(f"Error: Missing key in JSON - {exc}")
    else:
        print(f"Unexpected error: {exc}")
    
    sys.exit(1)


class JsonStatExtractor(abc.ABC):
    def __init__(self, **kwargs):
        self.path = kwargs["path"]
        assert self.path is not None

    @abc.abstractmethod
    def extract(self):
        """Extract stats from json by 'path'."""

    @abc.abstractmethod
    def run_extractor(self):
        """Run the custom extractor."""


class PlotterJsonExtractor(JsonStatExtractor):
    def __init__(self, no_run, **kwargs):
        super().__init__(**kwargs)
        self.out_folder = "folder"
        self.allocator = ""
        self.compiled_path = "./bin/"
        self.iterations = 1
        self.agg_stat = "average_num_operations_total"
        self.ylabel = "Average number of operations total"
        self.yscale = "linear"
        self.timeout = 100000

        self.xtitle = "NumberOfThreads"
        self.xvalues = []

        self.ds = []
        self.ds_args = []
        self.display_ds = []
        self.settings = defaultdict(dict)
        self.no_run = no_run

    def extract(self):
        try:
            with open(self.path, 'r') as file:
                data = json.load(file)
            
            # necessary
            if "folder" not in data:
                raise Exception("Please, set up the \"folder\" parameter.")
            self.out_folder = data['folder']

            if "json-file-input" not in data:
                raise Exception("Please, set up the \"json-file-input\" parameter.")
            self.input_path = data['json-file-input']

            if "key_title" not in data:
                raise Exception("Please, set up the \"key_title\" parameter.")
            self.xtitle = data['key_title']

            if "keys_title" not in data:
                raise Exception("Please, set up the \"keys_title\" parameter.")
            self.xvalues = data['keys_title']

            # optional
            if "iterations" in data:
                self.iterations = data['iterations']

            if "timeout" in data:
                self.timeout = data['timeout']

            if "allocator" in data:
                self.allocator = data['allocator']

            if "compiled-path" in data:
                self.compiled_path = data['compiled-path']

            if "aggregate_stat" in data:
                self.agg_stat = data['aggregate_stat']

            if "y_label" in data:
                self.ylabel = data['y_label']

            if "y_scale" in data:
                self.yscale = data['y_scale']

            for structs in data['competitors']:
                ds_name = structs['bin-name']
                self.ds.append(ds_name)
                self.ds_args.append(structs['data-structure-arguments'] if 'data-structure-arguments' in structs else {})
                self.display_ds.append(structs['display-name'] if 'display-name' in structs else ds_name)
                    
                for cur_keys in data['keys']:
                    assert(len(cur_keys['values']) == len(self.xvalues))
                    self.settings[cur_keys['name']] = cur_keys['values']
        except Exception as e:
            handle_file_error(e, self.path)

    def run_extractor(self):
        if not (self.no_run):
            folder_path = f"../plotting/{self.out_folder}/"
            if os.path.exists(folder_path):
                files = glob.glob(os.path.join(folder_path, "*")) 
                for f in files:
                    try:
                        if os.path.isfile(f):
                            os.remove(f)  
                    except Exception as e:
                        print(f"Error during file deletion {f}: {e}")
            # create folder
            if not os.path.exists(self.out_folder):
                os.makedirs(self.out_folder)
            log_folder = f"{self.out_folder}/logs/"
            if not os.path.exists(log_folder):
                os.makedirs(log_folder)
            # populate folder with temporary configs
            with open(self.input_path, 'r') as file:
                original_data = json.load(file)     
            for iter_num in range(len(self.xvalues)):
                temp_data = copy.copy(original_data)
                for key_path, values in self.settings.items():
                    set_value(temp_data, key_path, values[iter_num])
                title = str(self.xtitle) + '_' + self.xvalues[iter_num]
                temp_file = os.path.join(self.out_folder, f"temp_config_{title}.json")
                with open(temp_file, 'w') as temp:
                    json.dump(temp_data, temp, indent=4)
        for ds_name, ds_args in zip(self.ds, self.ds_args):
            for iter_num in range(len(self.xvalues)):
                title = str(self.xtitle) + '_' + self.xvalues[iter_num]
                suffix = "_" + ds_args["id"] if "id" in ds_args else ""
                modify_and_run_second_json(
                    self.out_folder, 
                    self.input_path, 
                    self.allocator,
                    self.compiled_path, 
                    self.iterations,
                    self.timeout,
                    self.agg_stat,
                    self.no_run,
                    ds_name,
                    title,
                    suffix
                )


class ResultJsonExtractor(JsonStatExtractor):
    def __init__(self, agg_label, **kwargs):
        super().__init__(**kwargs)
        self.aggregate_label = agg_label
        self.agg_stat = 1
        # can add more parameters

    def extract(self):
        try:
            with open(self.path, 'r', encoding='utf-8') as file:
                data_temp = json.load(file)
            if self.aggregate_label in data_temp:
                self.agg_stat = data_temp[self.aggregate_label]
            else:
                print(f"No key")
        except Exception as e:
            handle_file_error(e, self.path)
    
    def run_extractor(self):
        return


class IterationsJsonAggregator(JsonStatExtractor):
    def __init__(self, file_name, iters, stats, **kwargs):
        super().__init__(**kwargs)
        self.file_name = file_name
        self.iters = iters
        self.stats = stats
        self.aggregated = {}

    def extract(self):
        for stat in self.stats:
            agg_stat = 0
            for iter_num in range(1, self.iters + 1):
                try:
                    current_path = f"{self.path}/{self.file_name}_{iter_num}.json"
                    with open(current_path, 'r', encoding='utf-8') as file:
                        data_temp = json.load(file)
                    if stat in data_temp:
                        agg_stat += data_temp[stat]
                    else:
                        print(f"No key")
                except Exception as e:
                    handle_file_error(e, current_path)
            self.aggregated[stat] = agg_stat / self.iters
    
    def run_extractor(self):
        agg_path = f"{self.path}/{self.file_name}_aggregated.json"
        try:
            with open(agg_path, 'w') as outfile:
                json.dump(self.aggregated, outfile)
        except Exception as e:
            print(f"Error: {e}")
        return


def set_value(data, path, new_value):
    keys = path.split('.')
    current = data
    for key in keys[:-1]:
        try:
            current = current[int(key)]
        except ValueError:
            current = current[key]
    current[keys[-1]] = new_value


def modify_and_run_second_json(folder, 
                               params, 
                               allocator,
                               compiled_path,
                               iters,
                               timeout,
                               agg_stat,
                               no_run,
                               ds,
                               title,
                               suffix):
    try:
        log_file = Path.cwd().parent / "plotting" / folder / "logs" / f"log_{ds}_{title}{suffix}.txt"
        task_logger = create_logger("logger_prefix", log_file)
        ld_preload = f"../lib/{allocator}.so" if allocator != "" else ""
        inp = f"../plotting/{folder}/temp_config_{title}.json"
        file_name = f"{ds}_{title}{suffix}"
        if not (no_run):
            print("Running for " + ds + " with suffix " + title + suffix)

            for iter_num in range(1, iters + 1):
                out = f"../plotting/{folder}/{file_name}_{iter_num}.json"
                run_command = f"{compiled_path}{ds} -json-file {inp} -result-file {out}"
                # TODO:
                # for argument, value in additional: 
                #    run_command += f"-{argument} {value}"

                try:
                    env = os.environ.copy()
                    env["LD_PRELOAD"] = ld_preload
                    cp = subprocess.run(
                        run_command.split(),
                        cwd=str(Path.cwd().parent / "microbench"),
                        env=env,
                        timeout=timeout,
                        check=True,
                        capture_output=True,
                        text=True
                    )
                    if cp.stderr:
                        print(f"Error stream is not empty {cp.stderr}")
                        task_logger.info(f"stderr: {cp.stderr}")
                except subprocess.CalledProcessError as exc:
                    print(f"Error whilst launching subprocess {exc}")
                    task_logger.error(f"ProcessError while running command: {exc}")
                except subprocess.TimeoutExpired as exc:
                    print(f"Timeout in subprocess {exc}")
                    task_logger.error(f"TimeoutExpired while running command: {exc}")
                except Exception as e:
                    handle_file_error(e, out)
        
        # Aggregate
        aggregator = IterationsJsonAggregator(file_name=file_name, iters=iters, stats=[agg_stat], path=folder)
        aggregator.extract()
        aggregator.run_extractor()

    except Exception as e:
        handle_file_error(e, params)

def run(args):
    plotter_initial = PlotterJsonExtractor(path=args.file, no_run=args.no_run)
    plotter_initial.extract()
    plotter_initial.run_extractor()
    
    fig, ax = plt.subplots()
    fig.suptitle(args.title if args.title is not None else "Result graph")
    ax.set_xlabel(plotter_initial.xtitle)
    # ax.xaxis.labelpad = 2
    ax.set_yscale(plotter_initial.yscale)
    ax.set_ylabel(plotter_initial.ylabel)

    for ds, ds_args, name in zip(plotter_initial.ds, plotter_initial.ds_args, plotter_initial.display_ds):
        yvalues = []
        for val in plotter_initial.xvalues:
            suffix = '_' + ds_args["id"] if "id" in ds_args else ""
            path_to_result = f"{plotter_initial.out_folder}/{ds}_{plotter_initial.xtitle}_{val}{suffix}_aggregated.json"
            plotter_final = ResultJsonExtractor(agg_label=plotter_initial.agg_stat, path=path_to_result)
            plotter_final.extract()
            yvalues.append(plotter_final.agg_stat)
        ax.plot(plotter_initial.xvalues, yvalues, label = name)

    ax.grid(True)
    ax.legend()
    fig.tight_layout()
    fig.savefig(args.pathg if args.pathg is not None else f"Result_graph.png")
    plt.close(fig)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
        Script for plotting setbench benchmarks' results.
    """)
    parser.add_argument('-f', '--file', type=str, required=True, help='File with benchmark parameters. See README')
    parser.add_argument('-t', '--title', type=str, help='Name for the resulting graph')
    parser.add_argument('-pg', '--pathg', type=str, help='Path for graph')
    parser.add_argument('-no-run', action='store_true', help='Flag for benchmark running before plot. Use flag to only plot. Uses paths and structures from parameter file')

    args = parser.parse_args()

    run(args)
