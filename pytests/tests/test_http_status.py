import os
import shutil
import pytest

WORKDIR = '/root/httpstatus/workdir'
REPOFILENAME = 'httpstatus.repo'
REPONAME = 'httpstatus-repo'

@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    if os.path.isdir(WORKDIR):
        shutil.rmtree(WORKDIR)
    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    if os.path.isfile(filename):
        os.remove(filename)


def test_http_404_error(utils):
    workdir = WORKDIR
    utils.makedirs(workdir)
    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    baseurl = "http://localhost:8080/doesntexist"
    utils.create_repoconf(filename, baseurl, REPONAME)

    ret = utils.run(['tdnf', '--disablerepo=*', '--enablerepo={}'.format(REPONAME), 'makecache'], cwd=workdir)
    assert ret['retval'] == 2503 + 404
