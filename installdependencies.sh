#!/bin/bash
# These you would run yourself...
#sudo git clone https://github.com/umdenes100/Vision-System-Python.git
#sudo cd Vision-System-Python
sudo apt-get -y install v4l-utils
sudo apt install python3
sudo apt install python3-pip
sudo apt install python3-venv
sudo python3 -m venv .venv
source .venv/bin/activate
sudo .vev/bin/pip3 install -r requirements.txt

echo "Installation Complete. Make sure to set the static ip of this machine to 192.168.1.2 (See README.md)"