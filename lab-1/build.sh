cmake -Bbuild

cd ./build && make

sudo touch /var/run/daemon.pid
sudo chmod 666 /var/run/daemon.pid
