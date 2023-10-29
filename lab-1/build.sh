g++ -o lab-daemon -std=c++17 ./src/main.cpp ./src/daemon.cpp -Wall -Wextra

sudo touch /var/run/daemon.pid
sudo chmod 666 /var/run/daemon.pid
