import pytest

@pytest.mark.usefixtures('setup_test')
def test_parallel_download(utils):
    pkg1 = utils.config['sglversion_pkgname']
    pkg2 = utils.config['sglversion2_pkgname']
    utils.erase_package(pkg1)
    utils.erase_package(pkg2)
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', '-v', pkg1, pkg2])
    assert ret['retval'] == 0
    found = [l for l in ret['stdout'] if '%' in l and (pkg1 in l or pkg2 in l)]
    assert len(found) >= 2
