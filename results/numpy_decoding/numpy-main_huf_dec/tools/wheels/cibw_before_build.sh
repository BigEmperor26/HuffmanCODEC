set -xe

PROJECT_DIR="$1"
PLATFORM=$(PYTHONPATH=tools python -c "import openblas_support; print(openblas_support.get_plat())")

# Update license
if [[ $RUNNER_OS == "Linux" ]] ; then
    cat $PROJECT_DIR/tools/wheels/LICENSE_linux.txt >> $PROJECT_DIR/LICENSE.txt
elif [[ $RUNNER_OS == "macOS" ]]; then
    cat $PROJECT_DIR/tools/wheels/LICENSE_osx.txt >> $PROJECT_DIR/LICENSE.txt
elif [[ $RUNNER_OS == "Windows" ]]; then
    cat $PROJECT_DIR/tools/wheels/LICENSE_win32.txt >> $PROJECT_DIR/LICENSE.txt
fi

# Install Openblas
if [[ $RUNNER_OS == "Linux" || $RUNNER_OS == "macOS" ]] ; then
    basedir=$(python tools/openblas_support.py)
    cp -r $basedir/lib/* /usr/local/lib
    cp $basedir/include/* /usr/local/include
    if [[ $RUNNER_OS == "macOS" && $PLATFORM == "macosx-arm64" ]]; then
        sudo mkdir -p /opt/arm64-builds/lib /opt/arm64-builds/include
        sudo chown -R $USER /opt/arm64-builds
        cp -r $basedir/lib/* /opt/arm64-builds/lib
        cp $basedir/include/* /opt/arm64-builds/include
    fi
elif [[ $RUNNER_OS == "Windows" ]]; then
    PYTHONPATH=tools python -c "import openblas_support; openblas_support.make_init('numpy')"
    target=$(python tools/openblas_support.py)
    ls /tmp
    mkdir -p openblas
    # bash on windows does not like cp -r $target/* openblas
    for f in $(ls $target); do
        cp -r $target/$f openblas
    done
    ls openblas
fi

if [[ $RUNNER_OS == "macOS" ]]; then
    # Install same version of gfortran as the openblas-libs builds
    if [[ $PLATFORM == "macosx-arm64" ]]; then
        PLAT="arm64"
    fi
    source $PROJECT_DIR/tools/wheels/gfortran_utils.sh
    install_gfortran
    # Try a newer version of delocate that knows about /usr/local/lib
    pip install git+https://github.com/isuruf/delocate@search_usr_local#egg=delocate
fi
