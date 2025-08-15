# Copyright (C) 2019 VMware
# Modified for NiceOS to keep backward compatibility with legacy create_repo_conf calls.
#
# SPDX-License-Identifier: GPL-2.0-only
#
# Описание:
#   Этот модуль предоставляет:
#     - create_repo_conf(): совместима как с НОВОЙ сигнатурой (reposdir, name, baseurl, **options),
#                           так и со СТАРОЙ (mapping, reposdir=...),
#     - класс Tdnf: тонкая обёртка над CLI `tdnf` (с поддержкой Docker-режима),
#                   повторяет поведение вашего варианта.
#
#   Цель: «подогнать» поведение под isoBuilder.py / generate_initrd.py,
#   чтобы не менять их логику и в то же время сохранять актуальный интерфейс.

from tdnf._tdnf import *  # noqa: F401,F403 - оставляем как есть, если потребуются низкоуровневые биндинги

import os
import subprocess
from typing import Iterable, Optional, Sequence, Tuple, Dict, Any, Union


def _write_repo_file(reposdir: str, name: str, baseurl: str, **options: Any) -> str:
    """
    Вспомогательная функция: создаёт один *.repo файл.
    Параметры:
      - reposdir: каталог для записи *.repo
      - name: имя секции и файла (name.repo)
      - baseurl: значение baseurl=
      - **options: остальные пары ключ=значение, попадают в файл как строки "key=value"
    Возвращает путь к созданному файлу.
    """
    os.makedirs(reposdir, exist_ok=True)
    repo_path = os.path.join(reposdir, f"{name}.repo")

    # "name=" в файле берём из options.name если задан, иначе используем секцию name
    human_name = options.pop("name", name)

    lines = [
        f"[{name}]",
        f"name={human_name}",
        f"baseurl={baseurl}",
        "enabled=1",
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

      2) СТАРАЯ СИГНАТУРА (legacy, как в старом NiceOS-коде):
         create_repo_conf(mapping: Dict[str, Dict[str, Any]], reposdir="/path") -> str
         где mapping = { "repoName": {"baseurl": "...", "enabled": 1, "gpgcheck": 0, ...}, ... }
         Возвращает путь к ПЕРВОМУ созданному .repo (строка).
         Если в mapping несколько репозиториев — остальные тоже будут созданы.

    ЗАМЕЧАНИЯ:
      - В legacy-режиме "name" в файле .repo берётся из поля options["name"] если задан,
        иначе совпадает с именем секции (ключом словаря).
      - Возвращаем строку (путь к первому файлу), чтобы сохранить совместимость с вызывающим кодом,
        который обычно не использует это значение.
    """
    # Детектор старого интерфейса:
    # первый позиционный аргумент — словарь, а в kwargs есть 'reposdir'
    if args and isinstance(args[0], dict) and ("reposdir" in kwargs or (len(args) >= 2 and isinstance(args[1], str))):
        mapping: Dict[str, Dict[str, Any]] = args[0]
        # reposdir может прийти как именованный параметр (типичный случай) или вторым позиционным
        reposdir: Optional[str] = kwargs.get("reposdir")
        if reposdir is None and len(args) >= 2 and isinstance(args[1], str):
            reposdir = args[1]
        if reposdir is None:
            raise TypeError("legacy create_repo_conf: 'reposdir' must be provided (kwargs or positional)")

        first_path: Optional[str] = None
        # Проходим по всем репозиториям из mapping
        for repo_name, data in mapping.items():
            if not isinstance(data, dict):
                raise TypeError(f"legacy create_repo_conf: value for '{repo_name}' must be a dict")
            if "baseurl" not in data:
                raise TypeError(f"legacy create_repo_conf: repo '{repo_name}' is missing required 'baseurl'")

            # Копируем словарь, чтобы не менять исходный
            opts = dict(data)
            baseurl = str(opts.pop("baseurl"))
            # enabled по умолчанию 1 — но _write_repo_file уже пишет enabled=1,
            # поэтому если в opts явно передали enabled, мы его добавим как есть (можно переопределить).
            repo_path = _write_repo_file(reposdir, repo_name, baseurl, **opts)
            if first_path is None:
                first_path = repo_path

        # На всякий случай — если mapping пустой:
        return first_path or os.path.join(reposdir, "default.repo")

    # Новый интерфейс: ожидаем минимум 3 позиционных аргумента
    if len(args) < 3:
        # Попробуем достать по именам (на случай экзотического вызова)
        reposdir = kwargs.get("reposdir")
        name = kwargs.get("name")
        baseurl = kwargs.get("baseurl")
        if not (reposdir and name and baseurl):
            raise TypeError(
                "create_repo_conf expects either (mapping, reposdir=...) or (reposdir, name, baseurl, **options)"
            )
        # Сформируем позиционно и продолжим
        args = (reposdir, name, baseurl)

    reposdir = str(args[0])
    name = str(args[1])
    baseurl = str(args[2])
    options = dict(kwargs)
    # Убедимся, что не протащили лишний 'reposdir' в options
    if "reposdir" in options:
        options.pop("reposdir")
    return _write_repo_file(reposdir, name, baseurl, **options)


class Tdnf:
    """
    Лёгкая обёртка над CLI `tdnf`.

    Совместима с вашим кодом:
      - logger: объект логгера (имеет .debug/.info и т.п.)
      - releasever: передаётся в --releasever
      - reposdir:   передаётся в --setopt=reposdir=...
      - docker_image: если задан — команда запускается в контейнере,
                      при отсутствии образа происходит откат на хостовый `tdnf`
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
            # Не info, чтобы не зашумлять; но при отладке полезно видеть весь вывод
            self.logger.debug(output)

        return proc.returncode, output
