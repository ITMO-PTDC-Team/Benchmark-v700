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

CLEAN_DIRECTORY_AFTER = os.getenv("CLEAN_DIRECTORY_AFTER", "True")

def clean_directory(output_dir):
    for filename in os.listdir(output_dir):
        if not os.path.isdir(output_dir / filename):
            os.remove(output_dir / filename)

def check_error(args, capsys, error_string): 
    with pytest.raises(SystemExit) as excinfo:
        run(args)
    
    # Check the exit code is 1
    assert excinfo.value.code == 1
    
    # Check the error message
    captured = capsys.readouterr()
    assert error_string in captured.out

# Test fixture
@pytest.fixture(scope="module")
def test_dirs():
    base_dir = Path("tests")
    dirs = {
        "break": base_dir / "tests_break",
        "no_run": base_dir / "tests_no_run",
        "complete": base_dir / "tests_complete",
    }
    
    yield dirs

# --------------------------
# tests_break - Tests that should fail on bad input
# --------------------------
def test_missing_required_fields(test_dirs, capsys):
    """Test that missing required fields prints error message"""
    test_dict = {
        "missing_folder.json":     "Unexpected error: Please, set up the \"folder\" parameter.",
        "missing_input.json":      "Unexpected error: Please, set up the \"json-file-input\" parameter.",
        "missing_key_title.json":  "Unexpected error: Please, set up the \"key_title\" parameter.",
        "missing_keys_title.json": "Unexpected error: Please, set up the \"keys_title\" parameter.",
    }
    

    for file_path, msg in test_dict.items():
        test_file = test_dirs["break"] / file_path

        args = argparse.Namespace(
            file=str(test_file),
            title="Test Plot",
            pathg=str(test_dirs["break"] / "plot.png"),
            no_run=False
        )
        
        check_error(args, capsys, msg)

# --------------------------
# tests_no_run - Tests for correct plotting without running benchmarks
# --------------------------
def test_no_run_no_files(test_dirs, capsys):
    """Test that plotting works with pre-generated data"""
    test_file = test_dirs["no_run"] / "plot_config.json"
    output_dir = test_dirs["no_run"] / "output"

    if (not os.path.exists(output_dir)):
        os.makedirs(output_dir)

    clean_directory(output_dir)

    args = argparse.Namespace(
        file=str(test_file),
        title="Test Plot",
        pathg=str(output_dir / "plot.png"),
        no_run=True
    )

    check_error(args, capsys, "File not found")

    if (CLEAN_DIRECTORY_AFTER == "True"):
        clean_directory(output_dir)   

def test_no_run_delete_files(test_dirs, capsys):
    """Test plotting workflow with file deletion simulation"""
    test_file = test_dirs["no_run"] / "plot_config.json"
    output_dir = test_dirs["no_run"] / "output"
    
    if (not os.path.exists(output_dir)):
        os.makedirs(output_dir)

    clean_directory(output_dir)
            
    # mock functions
    def mock_run_extractor(self):
        for ds in ["ds1", "ds2"]:
            for threads in ["1", "2", "4"]:
                result_file = output_dir / f"{ds}_threads_{threads}_1.json"
                with open(result_file, 'w') as f:
                    json.dump({"ops": int(threads) * 1000}, f)
                
                agg_file = output_dir / f"{ds}_threads_{threads}_aggregated.json"
                with open(agg_file, 'w') as f:
                    json.dump({"ops": int(threads) * 1000}, f)

    def mock_extract_extractor(self):
        return True
    
    with patch('plotter.IterationsJsonAggregator.run_extractor', mock_run_extractor), \
         patch('plotter.IterationsJsonAggregator.extract', mock_extract_extractor):
        
        from plotter import run
        args_before = MagicMock(
            file=str(test_file),
            title="Test Plot",
            pathg=str(output_dir / "plot.png"),
            no_run=True
        )
        
        run(args_before)
        
        assert (output_dir / "plot.png").exists()
    
        assert (output_dir / "ds1_threads_1_1.json").exists()
        assert (output_dir / "ds1_threads_1_aggregated.json").exists()
        assert (output_dir / "ds2_threads_4_1.json").exists()
        assert (output_dir / "ds2_threads_4_aggregated.json").exists()

    for filename in os.listdir(output_dir):
        if 'aggregated' in filename or filename == 'plot.png':
            os.remove(output_dir / filename)

    args_after = argparse.Namespace(
        file=str(test_file),
        title="Test Plot",
        pathg=str(output_dir / "plot.png"),
        no_run=True
    )

    run(args_after)
    assert (output_dir / "plot.png").exists()

    if (CLEAN_DIRECTORY_AFTER == "True"):
        clean_directory(output_dir)  

# --------------------------
# tests_complete - Full integration tests
# --------------------------
def test_full_workflow(test_dirs):
    """Test complete workflow from config to plot"""
    test_file = test_dirs["complete"] / "full_config.json"
    output_dir = test_dirs["complete"] / "output"

    if (not os.path.exists(output_dir)):
        os.makedirs(output_dir)

    clean_directory(output_dir)

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
    assert (output_dir / "aksenov_splaylist_64.debra_threads_1_aggregated.json").exists()
    assert (output_dir / "aksenov_splaylist_64.debra_threads_2_aggregated.json").exists()

    if (CLEAN_DIRECTORY_AFTER == "True"):
        clean_directory(output_dir)  