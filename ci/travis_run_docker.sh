#!/bin/bash
set -e #Exit on failure.
printf "travis_run_docker.sh:"

# Take ownership of the home directory
# Otherwise docker folder mapping can fuck things up
sudo chown -R "$(id -u -n)": ~/
sudo chmod -R a+rw .

if [[ -d "${HOME}/host_cargo/git" && -d "${HOME}/host_cargo/registry" ]]; then
	echo "importing host_cargo"
  cp -Rp "${HOME}/host_cargo/git" "${HOME}/.cargo/git"
  cp -Rp "${HOME}/host_cargo/registry" "${HOME}/.cargo/registry"
	cp -Rp "${HOME}/host_cargo/bin" "${HOME}/.cargo/bin"
fi

./build.sh
test -d target/doc && chmod a+rwX target/doc # travis cache process can't delete it otherwise

if [[ "SKIP_HOST_CARGO_EXPORT" != 'True' ]]; then
    if [[ -d "${HOME}/.cargo/git" && -d "${HOME}/.cargo/registry" ]]; then
        echo "exporting to host_cargo"
        cp -Rp "${HOME}/.cargo/git" "${HOME}/host_cargo/git"
        cp -Rp "${HOME}/.cargo/registry" "${HOME}/host_cargo/registry"
        cp -Rp "${HOME}/.cargo/bin" "${HOME}/host_cargo/bin"
        chmod a+rwX "${HOME}/host_cargo/"*
    fi
fi

if [[ "$COVERALLS" == 'true' ]]; then
  pwd
  echo "*******  See coverage **************"
  lcov --list coverage.info # debug before upload

  echo "******* Uploading to coveralls **************"
  coveralls-lcov "--repo-token=${COVERALLS_TOKEN}" coverage.info # uploads to coveralls

  #kcov --coveralls-id=$TRAVIS_JOB_ID --exclude-pattern=/.cargo target/kcov target/debug/<<<MY_PROJECT_NAME>>>-*

fi
