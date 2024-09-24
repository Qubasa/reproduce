import os
import shlex
import subprocess
from pathlib import Path
from typing import TYPE_CHECKING

import pytest

if TYPE_CHECKING:
    pass
import hashlib


def write_script(cmd: list[str], new_env: dict[str, str], name: str) -> None:
    # Create the bash script content
    script_content = "#!/bin/bash\n"
    for key, value in new_env.items():
        if '"' in value:
            value = value.replace('"', '\\"')
        if "'" in value:
            value = value.replace("'", "\\'")
        if "`" in value:
            value = value.replace("`", "\\`")
        script_content += f'export {key}="{value}"\n'
    strace_cmd = ["strace", "-f", "-o", "file-access.log", "-e", "trace=file", *cmd]
    script_content += shlex.join(strace_cmd)

    # Write the bash script to a file
    script_filename = name
    with open(script_filename, "w") as script_file:
        script_file.write(script_content)

    print(f"You can find the script at {os.getcwd()}/{script_filename}")
    # Make the script executable
    os.chmod(script_filename, 0o755)


def checksum(file_path: Path, hash_algorithm: str = "sha256") -> str:
    # Create a hash object
    hash_func = hashlib.new(hash_algorithm)

    # Open the file in binary mode
    with open(file_path, "rb") as f:
        # Read the file in chunks
        while chunk := f.read(8192):
            hash_func.update(chunk)

    # Return the hexadecimal digest of the hash
    return hash_func.hexdigest()


def test_gpg_gen(
    monkeypatch: pytest.MonkeyPatch,
    temporary_home: Path,
) -> None:
    gnupghome = temporary_home / "gpg"
    gnupghome.mkdir(mode=0o700)
    monkeypatch.setenv("GNUPGHOME", str(gnupghome))
    monkeypatch.setenv("PASSWORD_STORE_DIR", str(temporary_home / "pass"))
    # Add /home/lhebendanz/Projects/clan-core/pkgs/clan-cli/tests to path
    os.environ["PATH"] += ":" + str(Path(__file__).parent)
    gpg_key_spec = temporary_home / "gpg_key_spec"
    gpg_key_spec.write_text(
        """
        Key-Type: 1
        Key-Length: 1024
        Name-Real: Root Superuser
        Name-Email: test@local
        Expire-Date: 0
        %no-protection
    """
    )

    cmd = ["ssh-keygen", "-t", "ed25519", "-f", "testkey", "-N", ""]
    subprocess.run(
        cmd,
        check=True,
    )
    sha256 = checksum(Path("testkey"))
    # assert sha256 == "2da3b2e850419580535cf1fa992d50917f631a4841b2f4a2a97f461e4efc95c9"
    return
    cmd = ["gpg", "--batch", "--gen-key", str(gpg_key_spec)]
    print(shlex.join(cmd))
    print("CWD: ", Path.cwd())
    new_env = os.environ.copy()
    write_script(cmd, new_env, "gpg-gen.sh")
    breakpoint()
    subprocess.run(
        cmd,
        check=True,
    )
    privdir = gnupghome / "private-keys-v1.d"

    print("=====================================")
    for key in privdir.glob("*.key"):
        # checksum key file
        key_checksum = key.read_text()
        lines = key_checksum.splitlines()
        date = lines[0].split(" ")[1]
        # assert date == "20240611T110408" or date == "20240611T110359"
        print(lines[0])
        print(lines[1:])
        sha256 = checksum(key)
        print(f"sha256 checksum: {sha256}")
        # assert sha256 == "88006c5e845658b542370a323cbd2eee3b409f38aaf3fdf06e1b491e0736df35" or sha256 == "62fc6c9d1764838e55720817a196290b2b5b7143f6a8799edcd90727de5c75b1"
