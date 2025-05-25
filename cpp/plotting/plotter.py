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

DEFAULT_OUTPUT_DIR_NAME = "plotter-output"
DEFAULT_TIMEOUT = 90

COLOR_PALETTE = sns.color_palette()
DS = ["redis_zset", "redist_sabt", "redis_sait", "redis_salt"]

LOG_FILE = "log.txt"

DEFAULT_FIG_SIZE = "15,8"
LAYOUT = "constrained"
FIG_FORMAT = "png"

OUT_FORMAT = "txt"


### Supported stats ###

# Operations stats:
TOTAL_FIND = "total-find"
TOTAL_RQ = "total-rq"
TOTAL_INSERTS = "total-inserts"
TOTAL_DELETES = "total-deletes"
TOTAL_UPDATES = "total-updates"
TOTAL_OPS = "total-ops"

OPERATIONS_STATS = {TOTAL_FIND, TOTAL_RQ, TOTAL_INSERTS, TOTAL_DELETES, TOTAL_UPDATES, TOTAL_OPS}

# Throughput stats:
THROUGHPUT_FIND = "find-throughput"
THROUGHPUT_RQ = "rq-throughput"
THROUGHPUT_UPDATE = "update-throughput"
THROUGHPUT_TOTAL = "total-throughput"

THROUGHPUT_STATS = {THROUGHPUT_FIND, THROUGHPUT_RQ, THROUGHPUT_UPDATE, THROUGHPUT_TOTAL}

# Depth stats:
TOTAL_KEY_DEPTH = "total-key-depth"
KEY_DEPTH = "key-depth"

DEPTH_STATS = {TOTAL_KEY_DEPTH, KEY_DEPTH}

# Iter stats:
TOTAL_KEY_SEARCH = "total-key-search"
ITER_STATS = {TOTAL_KEY_SEARCH}

ONLY_INS_DEL_STATS = {TOTAL_INSERTS, TOTAL_DELETES, TOTAL_UPDATES, THROUGHPUT_UPDATE}


### Supported prefill strategies ###
PREFILL_STRATEGY_INSERT = "insert"
PREFILL_STRATEGY_MIXED = "mixed"

PREFILL_STRATEGY_MAPPER = {
    PREFILL_STRATEGY_INSERT: "-prefill-insert",
    PREFILL_STRATEGY_MIXED: "-prefill-mixed"
}


def to_file_name(stat):
    return "_".join(stat.lower().replace("/", " ").split())


def get_label_by_stat(stat):
    if stat in OPERATIONS_STATS:
        return "operations"
    elif stat in THROUGHPUT_STATS:
        return "operations per second"
    elif stat in DEPTH_STATS:
        return "depth"
    elif stat in ITER_STATS:
        return "iters"
    else:
        return "stat"


class StatAggregator(abc.ABC):
    def __init__(self, **kwargs):
        self.ds = kwargs["ds"]
        assert self.ds is not None

        self.color = kwargs["color"]
        assert self.color is not None

        self.logger = kwargs["logger"]
        assert self.logger is not None

    @abc.abstractmethod
    def extract(self, keys_cnt, log):
        """Extract stat from log."""

    @abc.abstractmethod
    def aggregate(self):
        """Aggregate all extracted stat."""

    @abc.abstractmethod
    def plot(self, ax):
        """Plot aggregated stat to specific ax."""

    @abc.abstractmethod
    def out(self, output_dir):
        """Create file in the output_dir about all extracted stat"""


class AvgStatAggregator(StatAggregator):
    def __init__(self, stat, **kwargs):
        super().__init__(**kwargs)
        self.stat = stat
        self.vals_by_key = defaultdict(list)

    def get_extract_stat_name(self):
        return self.stat

    def extract_value(self, log):
        if log is None:
            return None
        m = re.search(f"{self.get_extract_stat_name()}=([\d\.]+)", log)
        return None if m is None else float(m.group(1))

    def extract(self, keys_cnt, log):
        value = self.extract_value(log)
        if value is None:
            self.logger.warning(f"keys_cnt={keys_cnt} - extract None")
        self.vals_by_key[keys_cnt].append(numpy.inf if value is None else value)

    def aggregate(self):
        self.keys = []
        self.avg_vals = []
        for keys_cnt, values in sorted(self.vals_by_key.items()):
            self.keys.append(keys_cnt)
            self.avg_vals.append(numpy.average(values))

    def plot(self, ax):
        ax.plot(self.keys, self.avg_vals, color=self.color, label=self.ds)

    def out(self, output_dir):
        out_file = output_dir / f"{to_file_name(self.stat)}.{OUT_FORMAT}"
        with open(out_file, "w") as ouf:
            for key, avg_val in zip(self.keys, self.avg_vals):
                ouf.write(f"key={key}\n")
                ouf.write(f"values={self.vals_by_key[key]}\n")
                ouf.write(f"avg_val={avg_val}\n\n")


# Operations aggregators:

class TotalFindAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("total_find", **kwargs)

class TotalRqAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("total_rq", **kwargs)

class TotalInsertsAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("total_inserts", **kwargs)


class TotalDeletesAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("total_deletes", **kwargs)

class TotalUpdatesAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("total_updates", **kwargs)


class TotalOpsAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("total_ops", **kwargs)


# Throughput aggregators:

class ThroughputFindAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("find_throughput", **kwargs)

class ThroughputRqAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("rq_throughput", **kwargs)

class ThroughputUpdateAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("update_throughput", **kwargs)


class ThroughputTotalAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("total_throughput", **kwargs)


# Depth aggregators:

class TotalKeyDepthAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("total key depth", **kwargs)
    
    def get_extract_stat_name(self):
        return "TOTAL_AVG_DEPTH"


# TODO
class KeyDepthAggregator(StatAggregator):
    pass


# Iters aggregators:

class TotalKeySearchAggregator(AvgStatAggregator):
    def __init__(self, **kwargs):
        super().__init__("total key search", **kwargs)
    
    def get_extract_stat_name(self):
        return "AVG_SEARCH_ITERS"


AGGREGATOR_BY_STAT = {
    TOTAL_FIND: TotalFindAggregator,
    TOTAL_RQ: TotalRqAggregator,
    TOTAL_INSERTS: TotalInsertsAggregator,
    TOTAL_DELETES: TotalDeletesAggregator,
    TOTAL_UPDATES: TotalUpdatesAggregator,
    TOTAL_OPS: TotalOpsAggregator,

    THROUGHPUT_FIND: ThroughputFindAggregator,
    THROUGHPUT_RQ: ThroughputRqAggregator,
    THROUGHPUT_UPDATE: ThroughputUpdateAggregator,
    THROUGHPUT_TOTAL: ThroughputTotalAggregator,

    TOTAL_KEY_DEPTH: TotalKeyDepthAggregator,
    KEY_DEPTH: KeyDepthAggregator,

    TOTAL_KEY_SEARCH: TotalKeySearchAggregator
}


def plot_avg_all(stat, title, ylabel, aggregators, output_dir, fig_size):
    fig, ax = plt.subplots(figsize=fig_size, layout=LAYOUT)
    fig.suptitle(title)
    for aggregator in aggregators:
        aggregator.plot(ax)
    ax.set_xlabel("keys set size")
    ax.set_xscale("log")
    ax.set_ylabel(ylabel)
    ax.grid(True)
    ax.legend()
    fig.savefig(output_dir / f"{to_file_name(stat)}.{FIG_FORMAT}")
    plt.close(fig)


def plot_all(stat, title, aggregators, output_dir, fig_size):
    if not aggregators:
        return
    ylabel = get_label_by_stat(stat)
    first = next(iter(aggregators))
    if isinstance(first, AvgStatAggregator):
        plot_avg_all(stat, title, ylabel, aggregators, output_dir, fig_size)
    elif isinstance(first, KeyDepthAggregator):
        # TODO
        pass


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


def task(top_dir, ip, dp, rp, fp, workload, workload_name, stats, args):
    log_file = top_dir / LOG_FILE
    logger_prefix = f"{to_file_name(workload_name)} - {ip}/{dp}/{fp}"

    stats_info = defaultdict(dict)
    for stat in stats:
        for ind, ds in enumerate(args.ds):
            aggregator_logger = create_logger(f"{logger_prefix} - {stat} - {ds}", log_file)
            stats_info[stat][ds] = AGGREGATOR_BY_STAT[stat](ds=ds, color=args.color[ind], logger=aggregator_logger)

    task_logger = create_logger(logger_prefix, log_file)

    for ds in args.ds:
        for key, prefill_size in zip(args.key, args.prefill_size):
            for _ in range(args.avg):
                run_command = f"./bin/{ds}.debra -{workload} -insdel {ip} {dp} -rq {rp} "\
                              f"-k {key} -prefillsize {prefill_size} -t {args.time} "\
                              f"-nwork {args.nwork} -nprefill {args.nprefill} {PREFILL_STRATEGY_MAPPER[args.prefill_strategy]}"
                if args.non_shuffle:
                    run_command += " -non-shuffle"
                if args.prefill_sequential:
                    run_command += " -prefill-sequential"

                ld_preload = f"../lib/{args.allocator}.so"
                task_logger.info(f"Running LD_PRELOAD={ld_preload} {run_command}")

                try:
                    env = os.environ.copy()
                    env["LD_PRELOAD"] = ld_preload
                    cp = subprocess.run(
                        run_command.split(),
                        cwd=str(args.setbench_dir / "microbench"),
                        env=env,
                        timeout=args.timeout,
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

                log = str(log)

                for stat in stats:
                    stats_info[stat][ds].extract(key, log)

        for stat in stats:
            stats_info[stat][ds].aggregate()

    for stat, aggregators_dict in stats_info.items():
        title = f"stat: {stat} | workload: {workload_name} | i={ip} d={dp} rq={rp} f={fp}"
        plot_all(stat, title, aggregators_dict.values(), top_dir, args.fig_size)

    ds_dict = {}
    for ds in args.ds:
        ds_dir = top_dir / to_file_name(ds)
        ds_dir.mkdir(exist_ok=True)
        ds_dict[ds] = ds_dir

    for aggregators_dict in stats_info.values():
        for ds, aggregators in aggregators_dict.items():
            aggregators.out(ds_dict[ds])


def get_colors(ds):
    colors = []
    for i, ds_in in enumerate(ds):
        index = i
        for j, ds_out in enumerate(DS):
            if ds_out in ds_in:
                index = j
                break
        colors.append(COLOR_PALETTE[index])
    return colors


def start(args):
    args.output_dir.mkdir(exist_ok=True)

    if not args.color:
        args.color = get_colors(args.ds)

    tasks = []
    for insdelrq in args.insdelrq:
        ip, dp, rp = map(float, insdelrq.split("/"))
        fp = round(1 - ip - dp - rp, 2)
        for workload, workload_name in zip(args.workload, args.workload_name):
            top_dir = args.output_dir / to_file_name(workload_name) / to_file_name(insdelrq)
            top_dir.mkdir(parents=True, exist_ok=True)
            stats = [stat for stat in args.stat if not ((rp + fp == 1) and stat in ONLY_INS_DEL_STATS)]
            tasks.append((top_dir, ip, dp, rp, fp, workload, workload_name, stats, args))

    print("START PLOTTING")

    with multiprocessing.Pool(args.nprocess) as pool:
        pool.starmap(task, tasks)

    print("END PLOTTING")


def check_args(args):
    if not (1 <= args.nprocess <= multiprocessing.cpu_count()):
        raise ValueError("nprocess must be in [1; cpu_count]")
    if any(map(lambda k: k <= 0, args.key)):
        raise ValueError("all keys must be > 0")
    if any(map(lambda ps: ps < 0, args.prefill_size)):
        raise ValueError("all prefill-sizes must be >= 0")
    if len(args.prefill_size) != len(args.key):
        raise ValueError("must be: len(prefill_size) == len(key)")
    if args.avg <= 0:
        raise ValueError("avg must be > 0")
    if args.timeout <= 0:
        raise ValueError("timeout must be > 0")
    if args.time <= 0:
        raise ValueError("time must be > 0")
    if args.nwork <= 0:
        raise ValueError("nwork must be > 0")
    if args.nprefill <= 0:
        raise ValueError("nprefill must be > 0")
    if args.color and len(args.color) != len(args.ds):
        raise ValueError(
            "if color specified then must be: len(color) == len(ds)")
    if not args.color and len(COLOR_PALETTE) < len(args.ds):
        raise ValueError(
            "not enough colors to draw all ds: specify colors by yourself")
    if args.workload_name and len(args.workload_name) != len(args.workload):
        raise ValueError(
            "if workload_name specified then must be: len(workload_name) == len(workload)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
        Script for plotting setbench benchmarks' results.

        Can launch benchmarks in multiple threads (--nprocess) to produce result faster.
    """)
    plotter_group = parser.add_argument_group("plotter args")
    plotter_group.add_argument("--stat", nargs="+", required=True, choices=[*AGGREGATOR_BY_STAT], help="Stats to plot.")
    plotter_group.add_argument("-o", "--output-dir", type=Path, default=Path.cwd() / DEFAULT_OUTPUT_DIR_NAME, help="Directory where results will be stored")
    plotter_group.add_argument("-s", "--setbench-dir", type=Path, default=Path.cwd().parent, help="Directory where setbench is located")
    plotter_group.add_argument("--nprocess", type=int, default=1, help="Specifies how many processes will be spawned to run benchmarks. Recommendation: nproccess * nwork <= cpu_count")
    plotter_group.add_argument("-c", "--color", nargs="*", action="extend", help="Colors for data structures. Supports all colors which can render matplotlib. If not specified, using DEFAULT_COLORS")
    plotter_group.add_argument("--avg", type=int, default=3, help="Number used for averaging results.")
    plotter_group.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT, help="Timeout in seconds for waiting results of each benchmark.")
    plotter_group.add_argument("--fig-size", type=str, default=DEFAULT_FIG_SIZE, help="figsize of plots")

    setbench_group = parser.add_argument_group("setbench args")
    setbench_group.add_argument("--ds", nargs="+", required=True, action="extend", help="Data structures to benchmark")
    setbench_group.add_argument("--workload", nargs="+", required=True, action="extend", help="Workloads' params to benchmark")
    setbench_group.add_argument("--workload-name", nargs="*", action="extend", help="Specifies workloads' names (used together with --workload)")
    setbench_group.add_argument("--insdelrq", nargs="+", required=True, help="inserts + deletes + rq + finds = 1. If insdelrq==0.3/0.3/0.3 then inserts == 0.3, deletes == 0.3, rq == 0.3 and finds == 0.1")
    setbench_group.add_argument("-k", "--key", nargs="+", required=True, type=int, action="extend", help="Stands for -k setbench arg")
    setbench_group.add_argument("-ps", "--prefill-size", nargs="+", required=True, type=int, action="extend", help="Stands for -prefillsize setbench arg")
    setbench_group.add_argument("-t", "--time", required=True, type=int, help="How long to run each benchmark? (ms) Stands for -t setbench arg")
    setbench_group.add_argument("--nwork", type=int, default=1, help="How many threads will do operations on each data structure at the same time? Stands for -nwork setbench arg")
    setbench_group.add_argument("--nprefill", type=int, default=1, help="How many threads will prefill each data structure? Stands for -nprefill setbench arg")
    setbench_group.add_argument("--prefill-strategy", choices=[*PREFILL_STRATEGY_MAPPER], default=PREFILL_STRATEGY_INSERT, help="Strategy to use to prefill each DS")
    setbench_group.add_argument("--non-shuffle", action="store_true", help="-non-shuffle setbench arg")
    setbench_group.add_argument("--prefill-sequential", action="store_true", help="-prefill-sequential setbench arg")
    setbench_group.add_argument("--allocator", default="libjemalloc", help="Allocator used while benchmarking")

    args = parser.parse_args()
    args.fig_size = eval(args.fig_size)

    check_args(args)
    start(args)
