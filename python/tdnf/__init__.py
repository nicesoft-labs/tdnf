# Copyright (C) 2019 VMware
# Modified for NiceOS to keep backward compatibility with legacy create_repo_conf calls.
#
# SPDX-License-Identifier: GPL-2.0-only
#
# Описание:
#   Этот модуль предоставляет:
#     - create_repo_conf(): совместима как с НОВОЙ сигнатурой (reposdir, name, baseurl, **options),
#                           так и со СТАРОЙ (mapping, reposdir=...),
#     - класс Tdnf: тонкая обёртка над CLI `tdnf` (с поддержкой Docker-режима).
#
#   Цель: «подогнать» поведение под isoBuilder.py / generate_initrd.py,
#   чтобы не менять их логику и при этом сохранить актуальный интерфейс.

from tdnf._tdnf import *  # noqa: F401,F403

import os
import subprocess
from typing import Iterable, Optional, Sequence, Tuple, Dict, Any


def _write_repo_file(
    reposdir: str,
    section_name: str,
    baseurl: str,
    display_name: Optional[str] = None,
    **options: Any,
) -> str:
    """
    Вспомогательная функция: создаёт один *.repo файл.

    Параметры:
      - reposdir: каталог для записи *.repo
      - section_name: имя секции и файла (section_name.repo)
      - baseurl: значение baseurl=
      - display_name: человекочитаемое имя (если None — берём section_name)
      - **options: остальные пары ключ=значение, попадают в файл как строки "key=value"

    Возвращает путь к созданному файлу.
    """
    os.makedirs(reposdir, exist_ok=True)
    repo_path = os.path.join(reposdir, f"{section_name}.repo")

    # enabled по умолчанию = 1, можно переопределить через options["enabled"]
    enabled = options.pop("enabled", 1)

    # Если display_name не задан — используем имя секции
    if display_name is None:
        display_name = section_name

    lines = [
        f"[{section_name}]",
        f"name={display_name}",
        f"baseurl={baseurl}",
        f"enabled={enabled}",
    ]

    for key, value in options.items():
        lines.append(f"{key}={value}")

    with open(repo_path, "w", encoding="utf-8") as repo_file:
        repo_file.write("\n".join(lines) + "\n")

    return repo_path


def create_repo_conf(*args: Any, **kwargs: Any) -> str:
    """
    Совмещённая версия create_repo_conf — понимает два интерфейса:

    1) НОВАЯ СИГНАТУРА (рекомендуется):
       create_repo_conf(reposdir: str, name: str, baseurl: str, **options) -> str
       где **options может содержать:
           - name: человекочитаемое имя (пишется в поле 'name=' файла .repo)
           - enabled, gpgcheck, skip_if_unavailable и т.п.

    2) СТАРАЯ СИГНАТУРА (legacy, как в старом NiceOS-коде):
       create_repo_conf(mapping: Dict[str, Dict[str, Any]], reposdir="/path") -> str
       где mapping = {
         "repoName": {"baseurl": "...", "enabled": 1, "gpgcheck": 0, "name": "...", ...},
         ...
       }
       Создаются все репозитории из mapping.
       Возвращается путь к ПЕРВОМУ созданному .repo (строка).
    """
    # --- Legacy-детектор: первый аргумент — dict, а reposdir передан (именованно или позиционно) ---
    if args and isinstance(args[0], dict) and ("reposdir" in kwargs or (len(args) >= 2 and isinstance(args[1], str))):
        mapping: Dict[str, Dict[str, Any]] = args[0]
        # reposdir может прийти как именованный параметр или вторым позиционным
        reposdir: Optional[str] = kwargs.get("reposdir")
        if reposdir is None and len(args) >= 2 and isinstance(args[1], str):
            reposdir = args[1]
        if reposdir is None:
            raise TypeError("legacy create_repo_conf: 'reposdir' must be provided (kwargs or positional)")

        first_path: Optional[str] = None

        for section_name, opts in mapping.items():
            if not isinstance(opts, dict):
                raise TypeError(f"legacy create_repo_conf: value for '{section_name}' must be a dict")
            if "baseurl" not in opts:
                raise TypeError(f"legacy create_repo_conf: repo '{section_name}' is missing required 'baseurl'")

            # Копию opts модифицируем локально
            local_opts = dict(opts)
            baseurl = str(local_opts.pop("baseurl"))

            # ВАЖНО: вынимаем 'name' (человекочитаемое) из опций,
            # чтобы не передать его одновременно и позиционно (как section_name), и именованно
            display_name = local_opts.pop("name", section_name)

            repo_path = _write_repo_file(reposdir, section_name, baseurl, display_name=display_name, **local_opts)
            if first_path is None:
                first_path = repo_path

        # Если mapping пуст — вернём путь по умолчанию (никто обычно не использует возвращаемое значение)
        return first_path or os.path.join(reposdir, "default.repo")

    # --- Новый интерфейс: ожидаем минимум 3 позиционных аргумента ---
    if len(args) < 3:
        # Попытка достать по именам (редкий случай)
        reposdir = kwargs.get("reposdir")
        name = kwargs.get("name")
        baseurl = kwargs.get("baseurl")
        if not (reposdir and name and baseurl):
            raise TypeError(
                "create_repo_conf expects either (mapping, reposdir=...) or (reposdir, name, baseurl, **options)"
            )
        args = (reposdir, name, baseurl)

    reposdir = str(args[0])
    section_name = str(args[1])
    baseurl = str(args[2])

    # Разбираем опции: если пришёл 'name' — это человекочитаемое имя
    options = dict(kwargs)
    display_name = options.pop("name", section_name)

    # На всякий случай уберём возможный дублирующийся 'reposdir' из options
    options.pop("reposdir", None)

    return _write_repo_file(reposdir, section_name, baseurl, display_name=display_name, **options)


class Tdnf:
    """
    Лёгкая обёртка над CLI `tdnf`.

    Совместима с вашим кодом:
      - logger: объект логгера (имеет .debug/.info и т.п.)
      - releasever: передаётся в --releasever
      - reposdir:   передаётся в --setopt=reposdir=...
      - docker_image: при наличии команда запускается в контейнере,
                      при отсутствии образа откатывается на хостовый `tdnf`
      - installroot: передаётся в --installroot

    Метод run() возвращает (returncode, stdout+stderr).
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

    def _build_cmd(self, args: Sequence[str]) -> Sequence[str]:
        """Конструирует базовую команду tdnf (без Docker-обёртки)."""
        cmd = ["tdnf", "-y"]
        if self.releasever:
            cmd.extend(["--releasever", self.releasever])
        if self.reposdir:
            cmd.append(f"--setopt=reposdir={self.reposdir}")
        if self.installroot:
            cmd.extend(["--installroot", self.installroot])
        cmd.extend(args)
        return cmd

    def _wrap_docker(self, base_cmd: Sequence[str]) -> Tuple[Sequence[str], bool]:
        """Если указан docker_image — оборачиваем команду в docker run."""
        if not self.docker_image:
            return base_cmd, False

        docker_cmd = ["docker", "run", "--rm"]
        if self.reposdir:
            docker_cmd.extend(["-v", f"{self.reposdir}:/etc/yum.repos.d"])
        if self.installroot:
            docker_cmd.extend(["-v", f"{self.installroot}:{self.installroot}"])
        docker_cmd.append(self.docker_image)
        docker_cmd.extend(base_cmd)
        return docker_cmd, True

    def run(
        self,
        args: Sequence[str],
        env: Optional[dict] = None,
        directories: Optional[Iterable[str]] = None,
    ) -> Tuple[int, str]:
        """
        Выполнить tdnf с указанными аргументами.
        Возвращает (returncode, combined_output).
        """

        # Создаём каталоги, если просили
        if directories:
            for directory in directories:
                os.makedirs(directory, exist_ok=True)

        # Базовая команда
        host_cmd = self._build_cmd(args)
        cmd, docker_used = self._wrap_docker(host_cmd)

        if self.logger:
            self.logger.debug("Running tdnf command: %s", " ".join(cmd))

        proc = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            env=env,
        )

        output = (proc.stdout or "") + (proc.stderr or "")

        # Если пытались использовать docker-образ, но его нет — откатываемся на хост
        missing_image = ("Unable to find image" in output) or ("manifest unknown" in output)
        if docker_used and proc.returncode != 0 and missing_image:
            if self.logger:
                self.logger.info(
                    "Docker image %s not found; falling back to host tdnf",
                    self.docker_image,
                )
            cmd = host_cmd
            if self.logger:
                self.logger.debug("Running tdnf command: %s", " ".join(cmd))
            proc = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                env=env,
            )
            output = (proc.stdout or "") + (proc.stderr or "")

        if self.logger:
            # DEBUG — чтобы не зашумлять INFO-лог
            self.logger.debug(output)

        return proc.returncode, output
