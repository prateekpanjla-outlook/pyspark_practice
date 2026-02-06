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
