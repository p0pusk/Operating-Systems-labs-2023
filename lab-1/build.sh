sudo touch /var/run/daemon.pid
cmake -Bbuild

cd ./build && make
