#
# Copyright (C) 2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import time
import shutil
import pytest

@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    utils.edit_config({'throttle': '1024'}, repo='photon-test')
    yield
    teardown_test(utils)

def teardown_test(utils):
    cache = utils.tdnf_config.get('main', 'cachedir')
    shutil.rmtree(cache, ignore_errors=True)
    utils.edit_config({'throttle': None}, repo='photon-test')


def _run_makecache(utils, parallel):
    utils.run(['tdnf', 'clean', 'all'])
    start = time.time()
    ret = utils.run(['tdnf', f'--setopt=parallelDownloads={parallel}', 'makecache'])
    duration = time.time() - start
    assert ret['retval'] == 0
    return duration


def test_makecache_ignores_parallel(utils):
    dur_seq = _run_makecache(utils, 1)
    dur_high = _run_makecache(utils, 8)
    # Metadata downloads should remain sequential regardless of the setting
    assert dur_high >= dur_seq * 0.9
