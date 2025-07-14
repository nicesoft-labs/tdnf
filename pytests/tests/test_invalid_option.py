# Copyright (C) 2024 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.

import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


def test_invalid_option(utils):
    ret = utils.run(['tdnf', '--invalid'])
    assert ret['retval'] != 0
    found = False
    for line in ret['stderr']:
        if 'No such option' in line:
            found = True
            break
    assert found
