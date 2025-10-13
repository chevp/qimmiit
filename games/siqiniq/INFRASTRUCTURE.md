# Siqiniq-001 Infrastructure Guide

Complete infrastructure setup for Siqiniq-001 game server with MariaDB and Docker.

## 📦 Components

### Core Services
- **Siqiniq Game Server** - Quarkus-based game server (JVM or Native)
- **MariaDB 11.4** - Primary database
- **Redis 7** - Caching and session management

### Optional Services
- **phpMyAdmin** - Database administration UI
- **Redis Commander** - Redis administration UI
- **Prometheus** - Metrics collection
- **Grafana** - Metrics visualization
- **Nginx** - Reverse proxy and load balancer

## 🚀 Quick Start

### Docker Compose (Recommended)

**Basic deployment:**
```bash
cd docker
docker-compose up -d
```

**With admin tools:**
```bash
docker-compose --profile admin up -d
```

**With monitoring:**
```bash
docker-compose --profile monitoring up -d
```

**Full stack:**
```bash
docker-compose --profile admin --profile monitoring up -d
```

### Using Scripts

**Windows:**
```bash
scripts\docker-deploy.bat --build --all
```

**Options:**
- `--build` - Build application before deploying
- `--monitoring` - Include Prometheus + Grafana
- `--admin` - Include phpMyAdmin + Redis Commander
- `--all` - Include all optional services

## 🗄️ Database Configuration

### MariaDB

**Connection Details:**
- Host: `localhost` (or `mariadb` in Docker network)
- Port: `3306`
- Database: `siqiniq`
- User: `siqiniq`
- Password: `siqiniq`
- Root Password: `root_password`

**Features:**
- UTF-8MB4 character set
- InnoDB engine
- 200 max connections
- 256MB buffer pool
- Automatic initialization script
- Health checks

**Accessing:**
```bash
# Via Docker
docker exec -it siqiniq-mariadb mariadb -u siqiniq -p

# Via phpMyAdmin
http://localhost:8081
```

**Backup:**
```bash
docker exec siqiniq-mariadb mariadb-dump -u siqiniq -p siqiniq > backup.sql
```

**Restore:**
```bash
docker exec -i siqiniq-mariadb mariadb -u siqiniq -p siqiniq < backup.sql
```

### Redis

**Connection Details:**
- Host: `localhost` (or `redis` in Docker network)
- Port: `6379`
- No password (default)

**Features:**
- Append-only file persistence
- 256MB max memory
- LRU eviction policy
- Health checks

**Accessing:**
```bash
# Via Docker
docker exec -it siqiniq-redis redis-cli

# Via Redis Commander
http://localhost:8082
```

**Monitoring:**
```bash
docker exec siqiniq-redis redis-cli INFO
```

## 🌐 Exposed Ports

| Service | Port | Description |
|---------|------|-------------|
| Game Server | 8080 | Main HTTP/WebSocket |
| MariaDB | 3306 | Database |
| Redis | 6379 | Cache |
| phpMyAdmin | 8081 | DB Admin UI |
| Redis Commander | 8082 | Redis UI |
| Prometheus | 9090 | Metrics |
| Grafana | 3000 | Dashboards |
| Nginx | 80, 443 | Reverse Proxy |

## 📊 Monitoring

### Prometheus

**Metrics endpoint:**
```
http://localhost:8080/q/metrics
```

**Prometheus UI:**
```
http://localhost:9090
```

**Key metrics:**
- `application_ready_time_seconds` - Startup time
- `jvm_memory_used_bytes` - Memory usage
- `http_server_requests_seconds_count` - Request count
- `websocket_connections_active` - Active connections

### Grafana

**Access:**
```
http://localhost:3000
Username: admin
Password: admin
```

**Pre-configured datasources:**
- Prometheus (automatically configured)

**Creating dashboards:**
1. Add Prometheus datasource
2. Import Quarkus dashboard (ID: 14370)
3. Create custom panels for game metrics

### Health Checks

**Liveness:**
```bash
curl http://localhost:8080/q/health/live
```

**Readiness:**
```bash
curl http://localhost:8080/q/health/ready
```

**Full health:**
```bash
curl http://localhost:8080/q/health
```

## 🔐 Security

### Production Checklist

- [ ] Change all default passwords
- [ ] Enable HTTPS (configure Nginx SSL)
- [ ] Restrict database access
- [ ] Configure firewall rules
- [ ] Enable Redis authentication
- [ ] Set up backup strategy
- [ ] Configure log rotation
- [ ] Enable rate limiting (Nginx)
- [ ] Set resource limits (Docker/K8s)

### Environment Variables

**Sensitive data:**
```bash
# Database
MARIADB_ROOT_PASSWORD=<secure-password>
MARIADB_PASSWORD=<secure-password>

# Application
QUARKUS_DATASOURCE_PASSWORD=<secure-password>
```

**Production:**
```bash
QUARKUS_PROFILE=prod
QUARKUS_LOG_LEVEL=WARN
```

## ☸️ Kubernetes Deployment

### Prerequisites
- Kubernetes cluster
- kubectl configured
- Docker image built and pushed to registry

### Deploy

**Apply manifests:**
```bash
kubectl apply -f docker/k8s/
```

**Or use script:**
```bash
scripts\k8s-deploy.bat
```

### Verify

**Check pods:**
```bash
kubectl get pods -n siqiniq-001
```

**Check services:**
```bash
kubectl get svc -n siqiniq-001
```

**View logs:**
```bash
kubectl logs -f deployment/siqiniq-server -n siqiniq-001
```

### Access

**Port forwarding:**
```bash
kubectl port-forward svc/siqiniq-server 8080:8080 -n siqiniq-001
```

**Load Balancer:**
```bash
kubectl get svc siqiniq-server-lb -n siqiniq-001
```

### Scaling

**Manual:**
```bash
kubectl scale deployment siqiniq-server --replicas=5 -n siqiniq-001
```

**Auto-scaling:**
HorizontalPodAutoscaler is pre-configured:
- Min: 2 replicas
- Max: 10 replicas
- CPU threshold: 70%
- Memory threshold: 80%

## 📁 Volume Management

### Docker Volumes

**List volumes:**
```bash
docker volume ls | findstr siqiniq
```

**Backup volume:**
```bash
docker run --rm -v siqiniq_mariadb-data:/data -v %CD%:/backup alpine tar czf /backup/mariadb-backup.tar.gz /data
```

**Restore volume:**
```bash
docker run --rm -v siqiniq_mariadb-data:/data -v %CD%:/backup alpine tar xzf /backup/mariadb-backup.tar.gz -C /
```

### Kubernetes PVC

**Check PVCs:**
```bash
kubectl get pvc -n siqiniq-001
```

**Resize PVC:**
```bash
kubectl patch pvc mariadb-pvc -n siqiniq-001 -p '{"spec":{"resources":{"requests":{"storage":"20Gi"}}}}'
```

## 🔧 Troubleshooting

### Server won't start

**Check logs:**
```bash
# Docker
docker logs siqiniq-server

# Kubernetes
kubectl logs deployment/siqiniq-server -n siqiniq-001
```

**Common issues:**
- Database not ready: Wait for MariaDB healthcheck
- Port conflict: Check if port 8080 is in use
- Memory: Increase container limits

### Database connection errors

**Verify database is running:**
```bash
docker ps | findstr mariadb
```

**Test connection:**
```bash
docker exec siqiniq-mariadb mariadb -u siqiniq -p -e "SELECT 1"
```

**Check network:**
```bash
docker network inspect siqiniq_siqiniq-network
```

### High memory usage

**Check JVM settings:**
```bash
JAVA_OPTS: -Xms512m -Xmx1024m
```

**Monitor:**
```bash
curl http://localhost:8080/q/metrics | findstr jvm_memory
```

### WebSocket issues

**Check Nginx config:**
- Ensure upgrade headers are set
- Verify timeout settings

**Test directly:**
```javascript
const ws = new WebSocket('ws://localhost:8080/game/ws');
```

## 📚 Additional Resources

- [Quarkus Container Images](https://quarkus.io/guides/container-image)
- [MariaDB Docker Hub](https://hub.docker.com/_/mariadb)
- [Redis Docker Hub](https://hub.docker.com/_/redis)
- [Kubernetes Documentation](https://kubernetes.io/docs/)
- [Docker Compose Documentation](https://docs.docker.com/compose/)

## 🎯 Performance Tuning

### MariaDB

**Increase buffer pool:**
```yaml
command: --innodb-buffer-pool-size=512M
```

**Optimize queries:**
```sql
EXPLAIN SELECT * FROM players WHERE player_id = 'xxx';
```

### Redis

**Increase max memory:**
```yaml
command: --maxmemory 512mb
```

**Monitor hit rate:**
```bash
redis-cli INFO stats | findstr keyspace
```

### Application

**JVM tuning:**
```bash
JAVA_OPTS: >
  -Xms1024m
  -Xmx2048m
  -XX:+UseG1GC
  -XX:MaxGCPauseMillis=100
  -XX:+ParallelRefProcEnabled
```

**Native image** (fastest):
```bash
./mvnw clean package -Pnative
```

## 📞 Support

For issues or questions:
1. Check logs
2. Review this documentation
3. Check Quarkus guides
4. Open issue on GitHub
