-- Siqiniq-001 Database Initialization Script

-- Enable performance schema
SET GLOBAL performance_schema = ON;

-- Create game-specific tables
CREATE TABLE IF NOT EXISTS players (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    player_id VARCHAR(255) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    health INT DEFAULT 100,
    max_health INT DEFAULT 100,
    x FLOAT DEFAULT 0,
    y FLOAT DEFAULT 100,
    z FLOAT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_player_id (player_id),
    INDEX idx_name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS player_inventory (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    player_id VARCHAR(255) NOT NULL,
    item_id VARCHAR(255) NOT NULL,
    quantity INT DEFAULT 1,
    slot_index INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (player_id) REFERENCES players(player_id) ON DELETE CASCADE,
    INDEX idx_player_id (player_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS world_state (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    world_id VARCHAR(255) UNIQUE NOT NULL,
    state_data JSON,
    last_saved TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_world_id (world_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS game_sessions (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    session_id VARCHAR(255) UNIQUE NOT NULL,
    player_id VARCHAR(255) NOT NULL,
    connected_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    disconnected_at TIMESTAMP NULL,
    ip_address VARCHAR(45),
    INDEX idx_session_id (session_id),
    INDEX idx_player_id (player_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Insert default world state
INSERT INTO world_state (world_id, state_data) VALUES
('sunny_plains_001', '{"name": "Sunny Plains", "initialized": true}')
ON DUPLICATE KEY UPDATE state_data = state_data;

-- Create monitoring user
CREATE USER IF NOT EXISTS 'monitor'@'%' IDENTIFIED BY 'monitor_password';
GRANT PROCESS, REPLICATION CLIENT ON *.* TO 'monitor'@'%';
FLUSH PRIVILEGES;

SELECT 'Database initialized successfully' AS Status;
