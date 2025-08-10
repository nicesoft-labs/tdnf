#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

from tdnf._tdnf import *

import os
import configparser


def create_repo_conf(path, repos):
    """Create a TDNF repository configuration file.

    Parameters
    ----------
    path : str
        Destination file path for the repository configuration.
    repos : dict
        Mapping of repository identifiers to dictionaries of options.

    Returns
    -------
    str
        The path to the generated configuration file.
    """

    parser = configparser.ConfigParser()
    for repo_id, options in repos.items():
        section = parser[repo_id] = {}
        for key, value in options.items():
            section[key] = str(value)

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as handle:
        parser.write(handle)

    return path
