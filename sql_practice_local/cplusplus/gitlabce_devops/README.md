# GitLab CE DevOps Server

Complete local DevOps platform for your C++ and other projects.

## Quick Start

### 1. Start the VM
```bash
cd gitlabce_devops
vagrant up
```

This will:
- Download Ubuntu 22.04 VM
- Configure with 4 CPUs, 10GB RAM, 50GB disk
- Install Docker and Docker Compose

### 2. Setup GitLab
```bash
vagrant ssh
cd /vagrant
bash devops-setup.sh
```

This will:
- Download GitLab CE (one-time download, ~1.5GB)
- Install with PostgreSQL and Redis
- Configure for local-only use
- Generate SSL certificates

**Wait 5-10 minutes for initial configuration.**

### 3. Access GitLab
Open: http://localhost:8080
- Username: `root`
- Password: `DevOps@2024!`

## Features

| Feature | Description |
|---------|-------------|
| **Git Hosting** | Private Git repositories |
| **CI/CD** | Built-in pipelines, runners, artifacts |
| **Container Registry** | Private Docker registry at localhost:5050 |
| **Wiki** | Documentation per project |
| **Issue Tracker** | Bug tracking and project management |
| **Merge Requests** | Code review and collaboration |
| **LFS** | Large file storage support |

## VM Configuration

- **CPUs**: 4 cores
- **RAM**: 10GB
- **Disk**: 50GB
- **OS**: Ubuntu 22.04 LTS
- **Ports**: 8080 (GitLab), 5050 (Registry), 2222 (SSH)

## Typical Workflow

### Create a New Project

```bash
# On your Windows machine in your project folder
git init
git add .
git commit -m "Initial commit"

# Add remote (from GitLab web UI, get project URL)
git remote add origin http://localhost:8080/root/my-project.git
git push -u origin main
```

### Setup CI/CD

Create `.gitlab-ci.yml` in your project:

```yaml
build:
  stage: build
  script:
    - echo "Building..."
  tags:
    - docker

test:
  stage: test
  script:
    - echo "Testing..."
  tags:
    - docker
```

### Use Container Registry

```bash
# Login
docker login localhost:5050

# Push image
docker tag myapp:latest localhost:5050/root/myproject:latest
docker push localhost:5050/root/myproject:latest

# Pull image
docker pull localhost:5050/root/myproject:latest
```

## Management Commands

```bash
# View logs
vagrant ssh -c 'sudo gitlab-ctl tail'

# Check status
vagrant ssh -c 'sudo gitlab-ctl status'

# Restart
vagrant ssh -c 'sudo gitlab-ctl restart'

# Backup
vagrant ssh -c 'sudo gitlab-backup create'

# Stop VM
vagrant halt

# Resume VM (fast!)
vagrant up

# Destroy VM (free disk space)
vagrant destroy -f
```

## Sharing Projects

Your `cplusplus` folder is synced to `/home/vagrant/projects/cplusplus` in the VM.

```bash
vagrant ssh
cd /home/vagrant/projects/cplusplus
git remote add origin http://localhost:8080/root/cpp-server.git
git push -u origin main
```

## Troubleshooting

### VM won't start
```bash
vagrant destroy -f
vagrant up
```

### Can't access http://localhost:8080
```bash
# Check if GitLab is running
vagrant ssh -c 'sudo gitlab-ctl status'

# Check logs
vagrant ssh -c 'sudo gitlab-ctl tail'
```

### Out of memory
- Vagrantfile already configured with 10GB RAM
- If still issues, increase to 16GB

### Port conflicts
- Ensure ports 8080 and 5050 are not in use on Windows
- Check with: `netstat -ano | findstr :8080`

## Resources

- [GitLab CE Documentation](https://docs.gitlab.com/ee/)
- [GitLab CI/CD](https://docs.gitlab.com/ee/ci/)
- [Container Registry](https://docs.gitlab.com/ee/user/packages/container_registry/)
- [GitLab Runner](https://docs.gitlab.com/runner/)
