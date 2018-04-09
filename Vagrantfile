# -*- mode: ruby -*-
# vi: set ft=ruby :

# All Vagrant configuration is done below. The "2" in Vagrant.configure
# configures the configuration version (we support older styles for
# backwards compatibility). Please don't change it unless you know what
# you're doing.
Vagrant.configure("2") do |config|
  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://atlas.hashicorp.com/search.
  config.vm.box = "ubuntu/xenial64"

  # Create a forwarded port mapping which allows access to a specific port
  # within the machine from a port on the host machine. In the example below,
  # accessing "localhost:8080" will access port 80 on the guest machine.
  # NOTE: This will enable public access to the opened port
  # config.vm.network "forwarded_port", guest: 80, host: 8080

  # Create a forwarded port mapping which allows access to a specific port
  # within the machine from a port on the host machine and only allow access
  # via 127.0.0.1 to disable public access
  # config.vm.network "forwarded_port", guest: 80, host: 8080, host_ip: "127.0.0.1"

  # Create a private network, which allows host-only access to the machine
  # using a specific IP.
  # config.vm.network "private_network", ip: "192.168.33.10"

  # Create a public network, which generally matched to bridged network.
  # Bridged networks make the machine appear as another physical device on
  # your network.
  # config.vm.network "public_network"

  # Share an additional folder to the guest VM. The first argument is
  # the path on the host to the actual folder. The second argument is
  # the path on the guest to mount the folder. And the optional third
  # argument is a set of non-required options.
  config.vm.synced_folder ".", "/ff"

  # Provider-specific configuration so you can fine-tune various
  # backing providers for Vagrant. These expose provider-specific options.
  # Example for VirtualBox:
  #
  config.vm.provider "virtualbox" do |vb|
    # Customize the amount of memory on the VM:
    vb.memory = 1024 * 8
    vb.name = 'ff_vm'
    vb.cpus = 4
  end

  # Enable provisioning with a shell script. Additional provisioners such as
  # Puppet, Chef, Ansible, Salt, and Docker are also available. Please see the
  # documentation for more information about their specific syntax and use.
  config.vm.provision "shell", inline: <<-SHELL
    apt-get update
    apt-get -y upgrade
    apt-get -y dist-upgrade
    add-apt-repository ppa:jonathonf/gcc-7.3
    apt-get update
    apt-get install -y autoconf automake build-essential python-dev libssl-dev libtool pkg-config cmake
    apt-get -y install gcc-7 g++-7 git gdb valgrind
    apt-get install -y openjdk-8-jdk ant
    apt-get install -y linux-tools-common linux-tools-generic linux-tools-`uname -r`
  SHELL

  config.vm.provision "shell", privileged: false, inline: <<-SHELL
    set -e

    if [ ! -f $HOME/buck ]; then
      cd $HOME
      git clone https://github.com/facebook/buck.git
      cd buck
      ant
      echo 'export PATH="$PATH:$HOME/buck/bin"' >> $HOME/.bashrc
    fi

    if [ ! -f $HOME/.gflags_install ]; then
        cd $HOME
        git clone https://github.com/gflags/gflags
        cd gflags
        mkdir build && cd build
        CC=`which gcc-7` CXX=`which g++-7` ccmake ..
        CC=`which gcc-7` CXX=`which g++-7` make
        sudo make install
        touch $HOME/.gflags_install
    fi

    if [ ! -f $HOME/ff ]; then
        mkdir $HOME/ff
        ln -s /ff/ff $HOME/ff/ff
        ln -s /ff/mode $HOME/ff/mode
        ln -s /ff/.buckversion $HOME/ff/.buckversion

        echo '[cxx]
cxxpp = /usr/bin/g++-7
cxx = /usr/bin/g++-7
cpp = /usr/bin/gcc-7
cc = /usr/bin/gcc-7
ld = /usr/bin/g++-7
aspp = /usr/bin/gcc-7
cxxflags = -Wall -std=c++17 -O3' >> $HOME/ff/.buckconfig
    fi
  SHELL
end

