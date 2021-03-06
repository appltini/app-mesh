## Build

App Mesh was implemented by modern C++ and require g++ version higher than 4.8.5, there are 2 ways to setup App Mesh build environment.

### Option 1: Setup build environment on local host (support Debian and Fedora distribution)
The general way build all dependencies on local Linux host, this will build all the dependencies and need some time to finish.
```shell
$ git clone --depth=1 https://github.com/laoshanxi/app-mesh.git
$ sudo sh app-mesh/script/openssl_update.sh
$ sudo sh app-mesh/autogen.sh
# The above process will create 'dep' directory, this dir can be deleted
```

After environment was setup with above steps, use bellow steps to build App Mesh.
```shell
$ cd app-mesh
$ mkdir build; cd build; cmake ..; make; make pack; make test
```

### Option 2: Build from docker image
The simple way is use docker container which already have compiler and dependencies, pull image `laoshanxi/appmesh:build_centos7` for the build docker image and use bellow steps to build App Mesh directly. `make test ARGS="-V"` is used to run Unit Test after `make`.
```shell
$ cd app-mesh
$ docker run --rm -v $(pwd):$(pwd) -w $(pwd) laoshanxi/appmesh:build_centos7 sh -c "mkdir build;cd build;cmake ..;make;make pack;make test ARGS="-V""
```

BTW, This docker image was built with bellow steps
https://github.com/laoshanxi/app-mesh/issues/97
