#!/bin/bash
# GitLab CE Complete Local Setup
# Run: vagrant ssh -c 'bash -s' < devops-setup.sh

set -e

echo "=========================================="
echo "GitLab CE Local DevOps Server Setup"
echo "=========================================="

# Update system
sudo apt-get update

# Install dependencies
sudo apt-get install -y curl openssh-server ca-certificates tzdata perl

# Download GitLab CE Omnibus (includes ALL dependencies)
# Version: Latest stable for Ubuntu 22.04
echo "Downloading GitLab CE Omnibus package..."
cd /tmp
wget https://packages.gitlab.com/gitlab/gitlab-ce/apt/pool/main/g/gitlab-ce/gitlab-ce_17.5.0-ce.0_amd64.deb \
     --timeout=300 || {
    echo "Download failed. You can manually download from:"
    echo "https://packages.gitlab.com/gitlab/gitlab-ce/"
    echo "And upload to: /tmp/gitlab-ce_17.5.0-ce.0_amd64.deb"
    exit 1
}

# Install GitLab
echo "Installing GitLab CE..."
sudo dpkg -i gitlab-ce_17.5.0-ce.0_amd64.deb

# Configure for local-only use
echo "Configuring GitLab for local development..."
sudo tee /etc/gitlab/gitlab.rb > /dev/null <<'EOF'
# GitLab configuration for local VM
external_url 'http://localhost:8080'

# Disable external services (local-only mode)
gitlab_rails['initial_root_password'] = 'DevOps@2024!'
gitlab_rails['gitlab_signup_enabled'] = false
gitlab_rails['gitlab_password_authentication_enabled_for_web'] = true

# Use local PostgreSQL (included in Omnibus)
postgresql['enable'] = true
postgresql['listen_address'] = '127.0.0.1'
postgresql['port'] = 5432

# Use local Redis (included in Omnibus)
redis['enable'] = true
redis['bind'] = '127.0.0.1'
redis['port'] = 6379

# Disable external email (development mode)
gitlab_rails['smtp_enable'] = false
gitlab_rails['gitlab_email_display_name'] = 'GitLab'
gitlab_rails['gitlab_email_reply_to'] = 'noreply@gitlab.local'

# Use local storage (no S3, no external object storage)
gitlab_rails['uploads_storage_path'] = "/var/opt/gitlab/uploads"
gitlab_rails['shared_path'] = "/var/opt/gitlab/shared"

# Disable Telemetry
gitlab_rails['usage_ping_enabled'] = false

# Configure registry (local Docker registry)
registry['enable'] = true
registry_external_url 'http://localhost:5050'
registry_nginx['ssl_certificate'] = "/etc/gitlab/ssl/registry.crt"
registry_nginx['ssl_certificate_key'] = "/etc/gitlab/ssl/registry.key"

# Disable outgoing webhooks for security
gitlab_rails['webhook_timeout'] = 10

# Enable LFS (Git Large File Storage) locally
gitlab_rails['lfs_enabled'] = true
gitlab_rails['lfs_storage_path'] = "/var/opt/gitlab/gitlab-rails/uploads/lfs-objects"

# Minimize resource usage for 2GB RAM VM
sidekiq['concurrency'] = 2
unicorn['worker_processes'] = 2
nginx['worker_processes'] = 1

# Disable unnecessary features for local DevOps
gitlab_rails['hourly_rollups_enabled'] = false
gitlab_rails['repository_downloads_path'] = "tmp/repositories"
EOF

# Generate self-signed certificate for registry
echo "Generating SSL certificates..."
sudo mkdir -p /etc/gitlab/ssl
openssl req -newkey rsa:2048 -nodes -keyout /etc/gitlab/ssl/registry.key \
        -x509 -days 365 -out /etc/gitlab/ssl/registry.crt \
        -subj "/C=US/ST=State/L=City/O=Organization/CN=localhost"

# Reconfigure GitLab
echo "Reconfiguring GitLab (this will take 5-10 minutes)..."
sudo gitlab-ctl reconfigure

# Add user to gitlab group
sudo usermod -aG gitlab vagrant

# Create alias for convenience
echo "Creating convenience aliases..."
cat >> /home/vagrant/.bashrc <<'EOF'

# GitLab aliases
alias gitlab-logs='sudo gitlab-ctl tail'
alias gitlab-status='sudo gitlab-ctl status'
alias gitlab-restart='sudo gitlab-ctl restart'
alias gitlab-reconfigure='sudo gitlab-ctl reconfigure'
alias gitlab-backup='sudo gitlab-backup create'
EOF

echo ""
echo "=========================================="
echo "✅ GitLab CE Setup Complete!"
echo "=========================================="
echo ""
echo "Access GitLab at: http://localhost:8080"
echo "Username: root"
echo "Password: DevOps@2024!"
echo ""
echo "Container Registry: http://localhost:5050"
echo ""
echo "Useful commands:"
echo "  vagrant ssh -c 'sudo gitlab-ctl tail'     # View logs"
echo "  vagrant ssh -c 'sudo gitlab-ctl status'    # Check status"
echo "  vagrant ssh -c 'sudo gitlab-ctl restart'   # Restart"
echo ""
echo "Initial setup:"
echo "1. Open http://localhost:8080 in your browser"
echo "2. Login with root / DevOps@2024!"
echo "3. Create your personal access token:"
echo "   User Settings → Access Tokens → Token name: devops"
echo "4. Create your first project!"
echo ""
echo "For CI/CD: GitLab Runner is included. Configure in:"
echo "   Admin → Settings → CI/CD → Runners"
