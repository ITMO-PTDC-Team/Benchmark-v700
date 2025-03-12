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

def parse_json(file_path):
    try:
        with open(file_path, 'r') as file:
            data = json.load(file)
        
        if "folder" not in data:
            print("Please, set up the \"folder\" parameter.")
            return
        out_folder = data['folder']


        if "json-file-input" not in data:
            print("Please, set up the \"json-file-input\" parameter.")
            return
        param_json = data['json-file-input']

        iterations = 1
        if "iterations" in data:
            iterations = data['iterations']

        for structs in data['competitors']:
            ds = structs['name']
            print(f"Benchmark: {ds}")

            key_names = []
            for cur_keys in data['keys']:
                key_names.append(cur_keys['name'])

            for i in range(len(data['keys_title'])):
                print(f"TEST: {data['key_title']}")
                keys = []
                for cur_keys in data['keys']:
                    keys.append(cur_keys['values'][i])
                print("KEK")
                modify_and_run_second_json(out_folder, param_json, iterations, ds, keys, key_names, data['key_title'] + data['keys_title'][i])
                print("-" * 40)

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

def set_value(data, path, new_value):
    keys = path.split('.')
    print(new_value)
    current = data
    for key in keys[:-1]:
        try:
            current = current[int(key)]
        except ValueError:
            current = current[key]
    current[keys[-1]] = new_value


def modify_and_run_second_json(folder, 
                               params, 
                               iters,
                               ds,
                               keys,
                               path_to_keys,
                               title):
    try:
        bench_path = Path.cwd().parent
        print(bench_path)

        with open(params, 'r') as file:
            data = json.load(file)

        temp_data = copy.copy(data)

        for key, key_path in zip(keys, path_to_keys):
            # TEMP JSON
            set_value(temp_data, key_path, key)

        temp_file = os.path.join(folder, f"temp_config_{ds}_{title}.json")

        if not os.path.exists(folder):
            os.makedirs(folder)

        with open(temp_file, 'w') as temp:
            json.dump(temp_data, temp, indent=4)

        allocator = "libmimalloc"   
        ld_preload = f"LD_PRELOAD= ../lib/{allocator}.so"

        inp = f"../plotting/{temp_file}"
        out = f"../plotting/{temp_file[:-5]}/_benchrestul.json"
        # + temp_file[:-5] + "_benchresult.json"
        run_command = f"./bin/{ds}.debra -json-file {inp} -result-file {out}"

        # for argument, value in additional: 
        #    run_command += f"-{argument} {value}"
        
        # try:
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
        #     if cp.stderr:
        #         task_logger.info(f"stderr: {cp.stderr}")
        #     log = cp.stdout
        # except subprocess.CalledProcessError as exc:
        #     task_logger.error(
        #         f"ProcessError while running command: {exc}")
        #     log = exc.stdout
        # except subprocess.TimeoutExpired as exc:
        #     task_logger.error(
        #         f"TimeoutExpired while running command: {exc}")
        #     log = exc.stdout

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