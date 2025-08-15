#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

from tdnf._tdnf import *


import os
import subprocess
from typing import Iterable, Optional, Sequence, Tuple


def create_repo_conf(reposdir: str, name: str, baseurl: str, **options) -> str:
    """Create a minimal yum repo configuration file.

    The file is written to ``reposdir`` and the path to the created file is
    returned. Additional key/value pairs may be supplied via ``options`` and
    are written as is to the configuration file.
    """

    os.makedirs(reposdir, exist_ok=True)
    repo_path = os.path.join(reposdir, f"{name}.repo")

    lines = [
        f"[{name}]",
        f"name={name}",
        f"baseurl={baseurl}",
        "enabled=1",
    ]

    for key, value in options.items():
        lines.append(f"{key}={value}")

    with open(repo_path, "w", encoding="utf-8") as repo_file:
        repo_file.write("\n".join(lines) + "\n")

    return repo_path


class Tdnf:
    """Light‑weight wrapper around the ``tdnf`` command line tool.

    This class mimics the historical interface that was removed when the
    ``_tdnf`` bindings were introduced.  It is intentionally simple and makes no
    assumptions about the calling environment; it merely constructs a command
    line and executes it via :func:`subprocess.run`.
    """

    def __init__(
        self,
        logger=None,
        releasever: Optional[str] = None,
        reposdir: Optional[str] = None,
        docker_image: Optional[str] = None,
        installroot: Optional[str] = None,
    ) -> None:
        self.logger = logger
        self.releasever = releasever
        self.reposdir = reposdir
        self.docker_image = docker_image
        self.installroot = installroot

    def run(
        self,
        args: Sequence[str],
        env: Optional[dict] = None,
        directories: Optional[Iterable[str]] = None,
    ) -> Tuple[int, str]:
        """Execute ``tdnf`` with the supplied arguments.

        Parameters
        ----------
        args:
            Sequence of arguments passed to ``tdnf``.
        env:
            Optional environment overrides.
        directories:
            Optional iterable of directories that should be created prior to
            executing the command.  Non‑existing directories are created with
            :func:`os.makedirs` using ``exist_ok=True``.

        Returns
        -------
        tuple
            ``(returncode, combined_output)`` where ``combined_output`` is the
            concatenation of stdout and stderr from the command execution.
        """

        if directories:
            for directory in directories:
                os.makedirs(directory, exist_ok=True)

        cmd = ["tdnf", "-y"]
        if self.releasever:
            cmd.extend(["--releasever", self.releasever])
        if self.reposdir:
            cmd.append(f"--setopt=reposdir={self.reposdir}")
        if self.installroot:
            cmd.extend(["--installroot", self.installroot])
        cmd.extend(args)

        docker_used = False

        if self.docker_image:
            docker_cmd = ["docker", "run", "--rm"]
            if self.reposdir:
                docker_cmd.extend(["-v", f"{self.reposdir}:/etc/yum.repos.d"])
            if self.installroot:
                docker_cmd.extend(["-v", f"{self.installroot}:{self.installroot}"])
            docker_cmd.append(self.docker_image)
            docker_cmd.extend(cmd)
            cmd = docker_cmd
            docker_used = True

        if self.logger:
            self.logger.debug("Running tdnf command: %s", " ".join(cmd))

        proc = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            env=env,
        )

        output = proc.stdout + proc.stderr

        # If a docker image was requested but not found, fall back to the host
        # tdnf so that callers can continue without container images.
        missing_image = (
            "Unable to find image" in output or "manifest unknown" in output
        )
        if docker_used and proc.returncode != 0 and missing_image:
            if self.logger:
                self.logger.info(
                    "Docker image %s not found; falling back to host tdnf",
                    self.docker_image,
                )
            cmd = ["tdnf", "-y"]
            if self.releasever:
                cmd.extend(["--releasever", self.releasever])
            if self.reposdir:
                cmd.append(f"--setopt=reposdir={self.reposdir}")
            if self.installroot:
                cmd.extend(["--installroot", self.installroot])
            cmd.extend(args)
            if self.logger:
                self.logger.debug("Running tdnf command: %s", " ".join(cmd))
            proc = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                env=env,
            )
            output = proc.stdout + proc.stderr

        if self.logger:
            self.logger.debug(output)

        return proc.returncode, output
