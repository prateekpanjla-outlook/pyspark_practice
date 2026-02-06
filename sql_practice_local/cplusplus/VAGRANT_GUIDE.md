# SQL Practice C++ - Vagrant Build Environment

## Quick Start

```bash
cd cplusplus
vagrant up
vagrant ssh
```

## Common Vagrant Commands

### Basic Commands
```bash
vagrant up              # Start and provision the VM
vagrant ssh             # SSH into the VM
vagrant halt            # Stop the VM
vagrant reload          # Restart the VM
vagrant suspend         # Save and suspend (faster startup)
vagrant resume          # Resume from suspend
vagrant destroy         # Delete the VM
vagrant status          # Check VM status
```

### Provisioning
```bash
vagrant provision       # Re-run provisioning scripts
vagrant up --provision # Start and re-provision
vagrant reload --provision  # Restart and re-provision
```

### Snapshots
```bash
vagrant snapshot save [name]     # Save a snapshot
vagrant snapshot restore [name]  # Restore snapshot
vagrant snapshot list            # List snapshots
vagrant snapshot delete [name]   # Delete snapshot
```

### Debugging
```bash
vagrant ssh-config      # Show SSH config
vagrant global-status   # Status of all VMs
vagrant validate        # Validate Vagrantfile
```

---

## Building in the VM

### First Time Setup
```bash
cd cplusplus
vagrant up              # Creates and starts VM (takes ~5-10 min first time)
vagrant ssh             # SSH into VM

# Inside VM:
cd /home/vagrant/project/cplusplus
ls -la                   # Your Windows files are here!

# Download Linux DuckDB (first time only)
wget https://github.com/duckdb/duckdb/releases/download/v1.0.0/libduckdb-linux-amd64.zip
unzip libduckdb-linux-amd64.zip
sudo cp libduckdb-linux-amd64/*.so* /usr/local/lib/
sudo cp libduckdb-linux-amd64/*.h /usr/local/include/
sudo ldconfig

# Build
mkdir build && cd build
cmake ..
make -j8

# Run
./sql-practice-server --port 8080
```

### Subsequent Builds (VM Already Running)
```bash
vagrant ssh
cd /home/vagrant/project/cplusplus/build
make -j8
./sql-practice-server
```

### Access from Windows
```bash
# Server runs at http://localhost:8080
# (Port is forwarded by Vagrant)
curl http://localhost:8080/health
```

---

## Vagrantfile Features

### Current Configuration
- **Box**: `ubuntu/jammy64` (Ubuntu 22.04 LTS)
- **RAM**: 2 GB
- **CPUs**: 2 cores
- **Network**: Port 8080 forwarded
- **Shared Folder**: Windows project synced to `/home/vagrant/project`

### Customize Resources
Edit `Vagrantfile`:
```ruby
config.vm.provider "virtualbox" do |vb|
  vb.memory = "4096"  # Change to 4 GB
  vb.cpus = 4         # Change to 4 CPUs
end
```

Then reload:
```bash
vagrant reload
```

---

## Recommended Vagrant Boxes for C++ Development

| Box | Description | Size | Use Case |
|-----|-------------|------|----------|
| **ubuntu/jammy64** ⭐ | Ubuntu 22.04 LTS | ~500 MB | Production-ready (current choice) |
| **ubuntu/focal64** | Ubuntu 20.04 LTS | ~450 MB | Older but stable |
| **hashicorp/bionic64** | Ubuntu 18.04 | ~400 MB | Legacy systems |
| **debian/bookworm64** | Debian 12 | ~350 MB | Minimal, stable |
| **fedora/39-cloud-base** | Fedora 39 | ~600 MB | Latest tools |
| **archlinux/archlinux** | Arch Linux | ~600 MB | Rolling release |

### Switch Boxes
Edit `Vagrantfile`:
```ruby
config.vm.box = "debian/bookworm64"  # Or any other box
```

Then:
```bash
vagrant destroy -f
vagrant up
```

---

## Existing Vagrant Setups for C++ Development

### GitHub Repositories

1. **cpp-vagrant** (Generic C++ dev box)
   - https://github.com/vyyoo/cpp-vagrant
   - Pre-installed: GCC, Clang, CMake, Conan

2. **modern-cpp-vm**
   - https://github.com/rzagorodkov/cpp-vm
   - Includes: Boost, Qt, LLVM

3. **cpp-dev-box**
   - https://github.com/taha-mohiuddin/cpp-dev-box
   - Docker + C++ toolchain

### Use Existing Box
```bash
# Clone an existing setup
git clone https://github.com/vyyoo/cpp-vagrant.git
cd cpp-vagrant
vagrant up
vagrant ssh
```

---

## Workflow Example

### Day 1: Initial Setup
```bash
cd cplusplus
vagrant up                    # ~10 minutes first time
vagrant snapshot save "fresh install"
```

### Day 2: Development
```bash
vagrant resume                 # Instant on
vagrant ssh
cd /home/vagrant/project/cplusplus
# Edit files in Windows (they sync automatically)
make -j8
```

### Build Failed? Rollback
```bash
vagrant snapshot restore "fresh install"
vagrant ssh
cd /home/vagrant/project/cplusplus/build
cmake .. && make -j8
```

### Done for Day
```bash
vagrant suspend               # Saves state, instant resume next time
```

---

## Synced Folders

Your Windows files are available in the VM:
```
Windows:                                Linux (VM):
C:\Users\prateek\Desktop\spark\  →  /home/vagrant/project/
  pyspark_practice/
    sql_practice_local/
      cplusplus/              →    /home/vagrant/project/cplusplus/
```

Edit files in Windows → changes instantly available in VM!

---

## Troubleshooting

### "vagrant: command not found"
**Solution**: Add Vagrant to PATH or use full path:
```cmd
"C:\HashiCorp\Vagrant\bin\vagrant.exe" up
```

### "Failed to mount folders"
**Solution**: Install VirtualBox Guest Additions:
```bash
vagrant plugin install vagrant-vbguest
vagrant reload
```

### "Port 8080 already in use"
**Solution**: Change port in `Vagrantfile`:
```ruby
config.vm.network "forwarded_port", guest: 8080, host: 8888
```

### "VM is slow"
**Solution**: Increase resources in `Vagrantfile`:
```ruby
vb.memory = "4096"
vb.cpus = 4
```

### "Out of disk space"
**Solution**: Clean build:
```bash
vagrant ssh
rm -rf /home/vagrant/project/cplusplus/build/*
```

---

## Comparison: Vagrant vs Manual VM

| Aspect | Vagrant | Manual VirtualBox |
|--------|---------|-------------------|
| **Setup** | 1 command | Manual ISO install |
| **Reproducible** | ✅ Yes (Vagrantfile) | ❌ No |
| **Shared folders** | ✅ Automatic | ⚠️ Manual setup |
| **Snapshots** | ✅ Built-in | ✅ Built-in |
| **Team collaboration** | ✅ Commit Vagrantfile | ⚠️ Document manually |
| **Portable** | ✅ One file | ⚠️ Full VM export |
| **First boot** | ~10 min | ~30 min |

---

## Best Practices

1. **Commit Vagrantfile to Git**
   ```bash
   git add Vagrantfile vagrant-setup.sh
   git commit -m "Add Vagrant build environment"
   ```

2. **Take snapshots before major changes**
   ```bash
   vagrant snapshot save "before big refactor"
   ```

3. **Use suspend instead of halt**
   ```bash
   vagrant suspend    # Saves state, faster resume
   # vagrant halt     # Full shutdown
   ```

4. **Clean up when done**
   ```bash
   vagrant destroy -f  # Free up disk space
   ```

---

## Quick Reference Card

```bash
# First time
vagrant up && vagrant ssh

# Everyday use
vagrant resume && vagrant ssh

# Edit in Windows, build in VM
cd /home/vagrant/project/cplusplus/build
make -j8

# Problems?
vagrant snapshot restore baseline

# Done
vagrant suspend
```
