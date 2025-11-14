Import("env")
import subprocess
import os
import shutil

# --- Dependency Check and Installation ---
# Use shutil.which to check if 'gcovr' executable is available in the PATH
if shutil.which("gcovr") is None:
    print("gcovr not found. Installing now...")
    try:
        # Run pip install if not found
        # '-q' for quiet output, 'check=True' to raise an error on non-zero exit code
        subprocess.run(["pip", "-q", "install", "gcovr"], check=True, capture_output=True, text=True)
        print("gcovr installed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error installing gcovr: {e.stderr}")
        # Use SCons Exit() function to stop the build on failure
        Exit(1)
else:
    print("gcovr found. Skipping installation.")


# --- Custom Target: gcovr_callback (Final Aggregation) ---
def gcovr_callback(*arg, **kwargs):
    """
    Executes gcovr to aggregate all trace files and generate the final HTML report.
    This should be run AFTER all tests have completed.
    """
    print("Executing gcovr: Aggregating reports...")

    # Ensure the output directory exists
    os.makedirs(".reports", exist_ok=True)

    # 1. Execute gcovr to combine ALL trace files (*.json) into a single report.
    # --add-tracefile .pio/tests/*.json: Reads all individual test results.
    # --html-details .reports/coverage.html: Outputs the final aggregated HTML report.
    # --root .: Correctly maps paths from the build directory to the source files.
    # --filter '.*': Ensures gcovr only processes files in the current root (project source).
    command = "gcovr --add-tracefile .pio/tests/*.json --html-details .reports/coverage.html --root . --filter '.*'"
    print(f"Executing: {command}")
    env.Execute(command)


env.AddCustomTarget("gcovr", None, gcovr_callback, title="gcovr", description="Executes gcovr to aggregate coverage data and generate an HTML report.")


# --- Post-Action: runNativeChecks (Per-Test Data Collection) ---
def runNativeChecks(source, target, env):
    test_name = env.get("PIOTEST_RUNNING_NAME", "")
    print(f"Running gcovr data collection for test: {test_name}")

    os.makedirs(".pio/tests/", exist_ok=True)

    build_dir = os.path.dirname(target[0].rstr())

    # FIX: Change to --json which accepts an output filename
    cppcheckargs = [
        "gcovr",
        build_dir,
        "--json",  # <--- CHANGED FROM --json-summary-pretty
        os.path.join(".pio/tests/", test_name + ".json"),
        "--root",
        ".",
        "--exclude-directories",
        ".*pio/libdeps/.*",
    ]

    command = " ".join(cppcheckargs)
    print(f"Executing: {command}")
    env.Execute(command)


# The post-action is attached to the main program/executable built for the test.
env.AddPostAction("$BUILD_DIR/$PROGNAME$PROGSUFFIX", runNativeChecks)
