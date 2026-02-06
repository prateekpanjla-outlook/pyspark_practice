#!/bin/bash
# Additional setup script for Vagrant VM
# This runs every time the VM starts

set -e

echo "Running additional setup..."

# Only install on first boot
if [ ! -f /home/vagrant/.vagrant_provisioned ]; then
    echo "First boot - setting up additional tools..."

    # Install useful development tools
    apt-get install -y vim htop tree gdb valgrind

    # Install Ninja (faster builds)
    apt-get install -y ninja-build

    # Install ccache (faster rebuilds)
    apt-get install -y ccache

    # Configure git if needed
    if [ ! -f /home/vagrant/.gitconfig ]; then
        git config --global user.name "Build User"
        git config --global user.email "builduser@localhost"
    fi

    # Mark as provisioned
    touch /home/vagrant/.vagrant_provisioned
    echo "Additional setup complete!"
else
    echo "VM already provisioned, skipping setup."
fi
