import abc
import argparse
from pathlib import Path
import multiprocessing
import subprocess
import os
import re
from collections import defaultdict
import numpy
import logging
import matplotlib.pyplot as plt
import seaborn as sns
import json
import sys
import plotter
import copy

DEFAULT_OUTPUT_DIR_NAME = "plotter-output"

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
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.out_folder = "folder"
        self.input_path = "test.json"
        self.allocator = "libmimalloc"
        self.iterations = 1
        self.ylabel = "average_num_operations_total"

        self.xtitle = "NumberofThreads"
        self.xvalues = []

        self.ds = []
        self.settings = defaultdict(dict)

    def extract(self):
        try:
            with open(self.path, 'r') as file:
                data = json.load(file)
            
            # necessary
            if "folder" not in data:
                print("Please, set up the \"folder\" parameter.")
                return
            self.out_folder = data['folder']

            if "json-file-input" not in data:
                print("Please, set up the \"json-file-input\" parameter.")
                return
            self.input_path = data['json-file-input']

            if "key_title" not in data:
                print("Please, set up the \"key_title\" parameter.")
                return
            self.xtitle = data['key_title']

            if "keys_title" not in data:
                print("Please, set up the \"keys_title\" parameter.")
                return
            self.xvalues = data['keys_title']

            # optional
            if "iterations" in data:
                self.iterations = data['iterations']

            if "allocator" in data:
                self.allocator = data['allocator']

            if "ylabel" in data:
                self.ylabel = data['ylabel']

            for structs in data['competitors']:
                ds_name = structs['name']
                self.ds.append(ds_name)

                for cur_keys in data['keys']:
                    assert(len(cur_keys['values']) == len(self.xvalues))
                    self.settings[cur_keys['name']] = cur_keys['values']
        except FileNotFoundError:
            print(f"Could not open '{file_path}'")
            sys.exit(1)
        except json.JSONDecodeError:
            print(f"Could not open '{file_path}'")
            sys.exit(1)
        except KeyError as e:
            print(f"No key in JSON: {e}")
        except Exception as e:
            print(f"Error: {e}")

    def run_extractor(self):
        for ds_name in self.ds:
            print("Running for " + ds_name)
            for iter_num in range(len(self.xvalues)):
                keys = []
                for key, values in self.settings.items():
                    keys.append(values[iter_num])
                paths = list(self.settings.keys())
                modify_and_run_second_json(self.out_folder, 
                                        self.input_path, 
                                        self.iterations, 
                                        self.allocator,
                                        ds_name, 
                                        keys, 
                                        paths, 
                                        str(self.xtitle) + self.xvalues[iter_num])

class ResultJsonExtractor(JsonStatExtractor):
    def __init__(self, label, **kwargs):
        super().__init__(**kwargs)
        self.ylabel = label

        self.average_num_operations = 1
        # can add more parameters

    def extract(self):
        try:
            with open(self.path, 'r', encoding='utf-8') as file:
                data_temp = json.load(file)
            if 'average_num_operations_total' in data_temp:
                self.average_num_operations = data_temp['average_num_operations_total']
            else:
                print(f"No key")
        except json.JSONDecodeError:
            print(f"Error while reading {filename}")
        except Exception as e:
            print(f"Error during working with {filename}: {e}") 
        except FileNotFoundError:
            print(f"Could not open '{file_path}'")
            sys.exit(1)
        except json.JSONDecodeError:
            print(f"Could not open '{file_path}'")
            sys.exit(1)
        except KeyError as e:
            print(f"No key in JSON: {e}")
        except Exception as e:
            print(f"Error: {e}")
    
    def run_extractor(self):
        return


def parse_json(file_path):
    plotter_initial = PlotterJsonExtractor(path=file_path)
    plotter_initial.extract()
    plotter_initial.run_extractor()
    
    fig, ax = plt.subplots()
    fig.suptitle("Result")
    ax.set_xlabel(plotter_initial.xtitle)
    # ax.xaxis.labelpad = 2
    ax.set_xscale("log")
    ax.set_ylabel(plotter_initial.ylabel)

    for ds in plotter_initial.ds:
        yvalues = []
        for val in plotter_initial.xvalues:
            path_to_result = f"{plotter_initial.out_folder}/{ds}_{plotter_initial.xtitle}{val}.json"
            plotter_final = ResultJsonExtractor(label=plotter_initial.ylabel, path=path_to_result)
            plotter_final.extract()
            yvalues.append(plotter_final.average_num_operations)
        ax.plot(plotter_initial.xvalues, yvalues, label = ds)

    ax.grid(True)
    ax.legend()
    fig.tight_layout()
    fig.savefig(f"Result_graph.png")
    plt.close(fig)

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
                               iters,
                               ds,
                               keys,
                               path_to_keys,
                               title):
    try:
        bench_path = Path.cwd().parent

        with open(params, 'r') as file:
            data = json.load(file)

        temp_data = copy.copy(data)

        for key, key_path in zip(keys, path_to_keys):
            # TEMP JSON
            set_value(temp_data, key_path, key)

        temp_file = os.path.join(folder, f"temp_config_{ds}_{title}.json")

        if not os.path.exists(folder):
            os.makedirs(folder)

        # logger
        log_file = bench_path / "plotting" / folder / f"log_{ds}_{title}.txt"
        task_logger = plotter.create_logger("logger_prefix", log_file)

        with open(temp_file, 'w') as temp:
            json.dump(temp_data, temp, indent=4)
 
        ld_preload = f"../lib/{allocator}.so"

        inp = f"../plotting/{temp_file}"
        out = f"../plotting/{folder}/{ds}_{title}.json"
        run_command = f"./bin/{ds}.debra -json-file {inp} -result-file {out}"

        # for argument, value in additional: 
        #    run_command += f"-{argument} {value}"

        try:
            env = os.environ.copy()
            env["LD_PRELOAD"] = ld_preload
            cp = subprocess.run(
                run_command.split(),
                cwd=str(bench_path / "microbench"),
                env=env,
                timeout=100000,
                check=True,
                capture_output=True,
                text=True
            )
            if cp.stderr:
                task_logger.info(f"stderr: {cp.stderr}")
                log = cp.stdout
        except subprocess.CalledProcessError as exc:
            task_logger.error(
               f"ProcessError while running command: {exc}")
            log = exc.stdout
        except subprocess.TimeoutExpired as exc:
            task_logger.error(
               f"TimeoutExpired while running command: {exc}")
            log = exc.stdout

    except FileNotFoundError:
        print(f"Could not open '{input_file}'")
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Could not open '{input_file}'")
        sys.exit(1)
    except KeyError as e:
        print(f"No key in JSON: {e}")
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        print(f"Could not launch script: {e}")
        sys.exit(1)
    except Exception as e:  
        print(f"Error: {e}")
        sys.exit(1)

def main():
    if len(sys.argv) != 2:
        print("Wrong number of arguments")
        return

    file_path = sys.argv[1]
    parse_json(file_path)

if __name__ == "__main__":
    main()