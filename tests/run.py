"""Run the actual control/servo C sources against a minimal HAL simulation."""
from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
compiler = shutil.which("gcc")
if not compiler:
    raise SystemExit("GCC is required for host tests; Keil remains the firmware compiler.")

with tempfile.TemporaryDirectory(prefix="manbo-firmware-test-") as temp:
    executable = Path(temp) / "test_firmware.exe"
    subprocess.run([
        compiler, "-std=c99", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-I", str(root / "tests/fakes"), "-I", str(root / "源码/MyDriver"),
        str(root / "tests/test_firmware.c"),
        str(root / "源码/MyDriver/RobotControl.c"),
        str(root / "源码/MyDriver/Servo.c"),
        "-o", str(executable),
    ], check=True)
    subprocess.run([str(executable)], check=True)
