Vagrant.configure("2") do |config|
  config.vm.guest = :freebsd
  config.vm.box = "freebsd/FreeBSD-14.0-STABLE"
  config.vm.synced_folder ".", "/vagrant", id: "vagrant-root", disabled: true  
  config.ssh.shell = "sh"
  config.vm.base_mac = "080027D14C66"

  config.vm.provision "shell", inline: <<-SHELL
    yes | pkg update
    yes | pkg upgrade
    yes | pkg install python38-3.8.18
    yes | pkg install php80-composer-2.6.6
    yes | pkg install portsnap
    portsnap fetch --interactive
    portsnap extract
    su vagrant -c "python3.8 -m ensurepip"
    su vagrant -c "python3.8 -m pip install jinja2"
  SHELL
  config.vm.provider "virtualbox" do |vb|
    vb.customize ["modifyvm", :id, "--memory", "1024"]
    vb.customize ["modifyvm", :id, "--cpus", "1"]
    vb.customize ["modifyvm", :id, "--hwvirtex", "on"]
    vb.customize ["modifyvm", :id, "--audio", "none"]
    vb.customize ["modifyvm", :id, "--nictype1", "virtio"]
    vb.customize ["modifyvm", :id, "--nictype2", "virtio"]
  end
end