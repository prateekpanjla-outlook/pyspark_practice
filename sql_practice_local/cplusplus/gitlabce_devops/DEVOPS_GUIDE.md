# GitLab CE - Complete Local DevOps Platform

## Overview
Completely self-contained GitLab CE instance running in your Vagrant VM. No external dependencies after initial setup.

## What's Included

| Component | Version | Purpose |
|-----------|---------|---------|
| **GitLab CE** | 17.5.0 | Git hosting, CI/CD, Registry |
| **PostgreSQL** | 14.x (bundled) | Database |
| **Redis** | 7.x (bundled) | Caching |
| **Nginx** | (bundled) | Web server |
| **Container Registry** | (bundled) | Docker images |

## Requirements

- **RAM**: 4GB minimum (increase in Vagrantfile)
- **Disk**: 20GB free
- **Network**: Port 8080, 5050 forwarded

## Setup Steps

### 1. Update Vagrantfile Resources
Edit `Vagrantfile` to increase RAM:
```ruby
config.vm.provider "virtualbox" do |vb|
  vb.memory = "4096"  # 4GB RAM
  vb.cpus = 2
end

# Add GitLab ports
config.vm.network "forwarded_port", guest: 8080, host: 8080
config.vm.network "forwarded_port", guest: 5050, host: 5050
```

### 2. Provision the VM
```bash
vagrant halt
vagrant up
vagrant ssh
```

### 3. Run Setup Script
```bash
cd /vagrant
bash devops-setup.sh
```

Or manually:
```bash
# Download GitLab
wget https://packages.gitlab.com/gitlab/gitlab-ce/apt/pool/main/g/gitlab-ce/gitlab-ce_17.5.0-ce.0_amd64.deb
sudo dpkg -i gitlab-ce_17.5.0-ce.0_amd64.deb

# Configure
sudo cp /vagrant/gitlab.rb /etc/gitlab/gitlab.rb
sudo gitlab-ctl reconfigure
```

## Access Your DevOps Platform

| Service | URL | Credentials |
|---------|-----|-------------|
| **GitLab Web** | http://localhost:8080 | root / DevOps@2024! |
| **Container Registry** | http://localhost:5050 | Same as GitLab |
| **GitLab API** | http://localhost:8080/api/v4 | Use personal access token |

## First-Time Setup

1. **Login**: Open http://localhost:8080, use `root` / `DevOps@2024!`

2. **Create Personal Access Token**:
   - Click your avatar → Preferences → Access Tokens
   - Add new token: `devops-full`
   - Scopes: `api`, `read_repository`, `write_repository`
   - Save the token securely!

3. **Create Your First Project**:
   ```bash
   # From your project directory
   git init
   git add .
   git commit -m "Initial commit"

   # Push to GitLab
   git remote add origin http://localhost:8080/root/my-project.git
   git push -u origin main
   ```

## CI/CD Pipeline Example

Create `.gitlab-ci.yml` in your project:
```yaml
stages:
  - build
  - test
  - deploy

build:
  stage: build
  script:
    - mkdir build && cd build
    - cmake ..
    - make -j$(nproc)
  artifacts:
    paths:
      - build/
    expire_in: 1 hour

test:
  stage: test
  script:
    - cd build
    - ctest --output-on-failure

deploy:
  stage: deploy
  script:
    - echo "Deploy to staging..."
  only:
    - main
```

## GitLab Runner Setup (Optional)

For CI/CD jobs to run, register a runner:

```bash
# In the VM
sudo gitlab-runner register \
  --url http://localhost:8080 \
  --registration-token YOUR_TOKEN \
  --executor shell \
  --description "Local Vagrant Runner" \
  --run-untagged="true"
```

## Common Operations

### View Logs
```bash
vagrant ssh -c 'sudo gitlab-ctl tail'
```

### Check Status
```bash
vagrant ssh -c 'sudo gitlab-ctl status'
```

### Restart Services
```bash
vagrant ssh -c 'sudo gitlab-ctl restart'
```

### Backup
```bash
vagrant ssh -c 'sudo gitlab-backup create'
```

### Restore
```bash
vagrant ssh -c 'sudo gitlab-backup restore BACKUP_ID'
```

## Docker Registry Usage

```bash
# Login to registry
docker login localhost:5050

# Tag and push
docker tag myapp:latest localhost:5050/root/myapp:latest
docker push localhost:5050/root/myapp:latest

# Pull from registry
docker pull localhost:5050/root/myapp:latest
```

## Project Structure for DevOps

```
your-project/
├── .gitlab-ci.yml          # CI/CD pipeline
├── Dockerfile               # Container build
├── docker-compose.yml       # Local testing
├── src/                     # Source code
└── tests/                   # Tests
```

## Troubleshooting

### GitLab won't start
```bash
vagrant ssh -c 'sudo gitlab-ctl tail'
# Check for PostgreSQL errors
vagrant ssh -c 'sudo gitlab-ctl status postgresql'
```

### Out of memory
- Increase Vagrantfile RAM to 6GB
- Reduce `unicorn['worker_processes']` in gitlab.rb

### Port conflicts
```bash
# Check what's using port 8080
vagrant ssh -c 'sudo lsof -i :8080'
```

## Self-Contained Verification

No external dependencies needed:
- ✅ No external SMTP (works without email)
- ✅ No external S3 (uses local filesystem)
- ✅ No external object storage
- ✅ No external authentication (local users only)
- ✅ Bundled PostgreSQL and Redis
- ✅ Self-signed SSL certificates included

## Resources

- **Logs**: `/var/log/gitlab/`
- **Config**: `/etc/gitlab/gitlab.rb`
- **Data**: `/var/opt/gitlab/`
- **Backups**: `/var/opt/gitlab/backups/`
