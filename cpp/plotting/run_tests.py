import pytest
import os
import json
import tempfile
from pathlib import Path
import shutil
from unittest.mock import patch, MagicMock
import matplotlib
import argparse
from plotter import run, PlotterJsonExtractor, ResultJsonExtractor, IterationsJsonAggregator

matplotlib.use('Agg')

@pytest.fixture(scope="module")
def test_dirs():
    base_dir = Path("tests")
    dirs = {
        "break": base_dir / "tests_break",
        "no_run": base_dir / "tests_no_run",
        "complete": base_dir / "tests_complete",
    }
    
    yield dirs

# Helper functions
def create_test_json(path, content):
    with open(path, 'w') as f:
        json.dump(content, f)

# --------------------------
# tests_break - Tests that should fail on bad input
# --------------------------
def test_missing_required_fields(test_dirs, capsys):
    """Test that missing required fields prints error message"""
    test_file = test_dirs["break"] / "missing_fields.json"
    
    plotter = PlotterJsonExtractor(path=test_file, no_run="false")
    plotter.extract()
    
    captured = capsys.readouterr()
    
    assert "Please, set up the \"folder\" parameter." in captured.out

# --------------------------
# tests_no_run - Tests for correct plotting without running benchmarks
# --------------------------
def test_plotting_without_running(test_dirs):
    """Test that plotting works with pre-generated data"""
    test_file = test_dirs["no_run"] / "plot_config.json"
    output_dir = test_dirs["no_run"] / "output"
    output_dir.mkdir(exist_ok=True)
    
    # Create test config
    create_test_json(test_file, {
        "folder": str(output_dir),
        "json-file-input": str(test_dirs["no_run"] / "bench_config.json"),
        "key_title": "threads",
        "keys_title": ["1", "2", "4"],
        "competitors": [
            {"name": "ds1", "display-name": "DS1"},
            {"name": "ds2", "display-name": "DS2"}
        ],
        "keys": [
            {"name": "test.numThreads", "values": ["1", "2", "4"]}
        ],
        "aggregate_stat": "ops",
        "y_label": "Operations",
        "y_scale": "log"
    })
    
    # Create pre-generated result files
    for ds in ["ds1", "ds2"]:
        for threads in ["1", "2", "4"]:
            result_file = output_dir / f"{ds}_threads_{threads}_1.json"
            create_test_json(result_file, {"ops": int(threads) * 1000})
    
    # Create a mock args object
    args = argparse.Namespace(
        file=str(test_file),
        title="Test Plot",
        pathg=str(output_dir / "plot.png"),
        no_run=True
    )
    
    # Test plotting
    run(args)
    
    assert (output_dir / "plot.png").exists()

# --------------------------
# tests_complete - Full integration tests
# --------------------------
def test_full_workflow(test_dirs):
    """Test complete workflow from config to plot"""
    test_file = test_dirs["complete"] / "full_config.json"
    output_dir = test_dirs["complete"] / "full_output"
    output_dir.mkdir(exist_ok=True)
    
    # Create test config
    create_test_json(test_file, {
        "folder": str(output_dir),
        "json-file-input": str(test_dirs["complete"] / "bench_config.json"),
        "key_title": "threads",
        "keys_title": ["1", "2"],
        "competitors": [
            {"name": "aksenov_splaylist_64", "display-name": "aksenov_splaylist"}
        ],
        "keys": [
            {
                "name": "test.threadLoopBuilders.0.quantity",
                "values": [1, 2]
            },
            {
                "name": "test.threadLoopBuilders.1.quantity",
                "values": [1, 2]
            },
            {
                "name": "test.numThreads",
                "values": [2, 4]
            }
        ],
        "iterations": 2,
        "aggregate_stat": "average_num_operations_total",
        "y_label": "Operations",
        "y_scale": "log"
    })
    
    # Create args object
    args = argparse.Namespace(
        file=str(test_file),
        title="Full Test",
        pathg=str(output_dir / "full_plot.png"),
        no_run=False
    )
    
    # Test complete run
    run(args)
    
    # Verify outputs
    assert (output_dir / "full_plot.png").exists()
    assert (output_dir / "aksenov_splaylist_64_threads_1_aggregated.json").exists()
    assert (output_dir / "aksenov_splaylist_64_threads_2_aggregated.json").exists()