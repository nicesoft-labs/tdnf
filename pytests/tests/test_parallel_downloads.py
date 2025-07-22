import os
import time
import shutil
import pytest

PKG1 = 'tdnf-test-one'
PKG2 = 'tdnf-test-two'

@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    utils.edit_config({'throttle': '1024'}, repo='photon-test')
    yield
    teardown_test(utils)

def teardown_test(utils):
    cache = utils.tdnf_config.get('main', 'cachedir')
    shutil.rmtree(cache, ignore_errors=True)
    utils.edit_config({'throttle': None}, repo='photon-test')
    utils.erase_package(PKG1)
    utils.erase_package(PKG2)


def test_parallel_downloads(utils):
    cache = utils.tdnf_config.get('main', 'cachedir')
    shutil.rmtree(cache, ignore_errors=True)
    os.makedirs(cache, exist_ok=True)

    start = time.time()
    ret = utils.run(['tdnf', '--setopt=parallelDownloads=1', 'install', '-y', PKG1, PKG2])
    dur_seq = time.time() - start
    assert ret['retval'] == 0
    utils.run(['rm', '-rf', cache])
    utils.erase_package(PKG1)
    utils.erase_package(PKG2)

    start = time.time()
    ret = utils.run(['tdnf', '--setopt=parallelDownloads=2', 'install', '-y', PKG1, PKG2])
    dur_par = time.time() - start
    assert ret['retval'] == 0
    assert dur_par < dur_seq
