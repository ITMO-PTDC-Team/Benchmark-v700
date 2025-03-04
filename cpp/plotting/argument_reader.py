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

DEFAULT_OUTPUT_DIR_NAME = "plotter-output"

def parse_json(file_path):
    try:
        with open(file_path, 'r') as file:
            data = json.load(file)
        
        if "benchmark-params" not in data:
            print("No benchmark-params key in JSON.")
            return

        for benchmark in data["benchmark-params"]:
            print(f"Benchmark: {benchmark['folder']}")

            input_file = benchmark['json-file-input']
            output_file = benchmark['json-file-output']

            for test in benchmark["tests"]:
                print(f"TEST: {test['name']}")
                
                ds = test['data-structure']['name']
                if "keys" in test:
                    keys = test["keys"]

                    x_axis_name = keys['name']
                    x_axis_keys = map(int, keys['values'])
                    
                    # SCRIPT RUNNING
                    print("RUNNING")
                    modify_and_run_second_json(input_file, output_file, ds, x_axis_name, x_axis_keys, {})
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

def modify_and_run_second_json(input_file, 
                               output_file, 
                               ds,
                               x_axis_name,
                               x_axis_keys,
                               additional):
    try:
        bench_path = Path.cwd().parent
        print(bench_path)

        with open(Path.cwd().parent / json_example / input_file, 'r') as file:
            data = json.load(file)

        for x_key in x_axis_keys:
            # TEMP JSON
            data["test"]["numThreads"] = x_key
            temp_file = f"temp_config_{x_key}_{x_axis_name}.json"
            with open(temp_file, 'w') as temp:
                json.dump(data, temp, indent=4)


            allocator = "libmimalloc"
            ld_preload = f"LD_PRELOAD= ../lib/{allocator}.so"

            run_command = f"{ld_preload} ./bin/{ds}.debra -json-file {temp_file} -result-file {result-file} "

            for argument, value in additional: 
                run_command += f"-{argument} {value}"

            bench_path = Path.cwd().parent
            print(bench_path)
            
            try:
                env = os.environ.copy()
                # env["LD_PRELOAD"] = ld_preload
                cp = subprocess.run(
                    run_command.split(),
                    cwd=str(bench_path / "json_example"),
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