package io.qimmiit.qilak.tools;

import jakarta.enterprise.context.ApplicationScoped;
import org.jboss.logging.Logger;

import java.sql.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * AkutikStorage - Java binding for Akutik storage tools
 *
 * This is a Java implementation inspired by the C++ Akutik library.
 * Provides the same tool-based, no-business-logic approach.
 *
 * Similar to akutik::Storage in C++, but for Java/Quarkus
 */
@ApplicationScoped
public class AkutikStorage {

    private static final Logger LOG = Logger.getLogger(AkutikStorage.class);

    private Connection connection;
    private String databasePath;

    /**
     * Initialize storage with database path
     */
    public boolean initialize(String databasePath) {
        this.databasePath = databasePath;
        LOG.infof("Initializing Akutik storage: %s", databasePath);

        try {
            connection = DriverManager.getConnection("jdbc:sqlite:" + databasePath);

            // Create storage table
            try (Statement stmt = connection.createStatement()) {
                stmt.execute("""
                    CREATE TABLE IF NOT EXISTS storage (
                        key TEXT PRIMARY KEY,
                        value BLOB NOT NULL
                    )
                """);

                stmt.execute("CREATE INDEX IF NOT EXISTS idx_key ON storage(key)");
            }

            LOG.info("Akutik storage initialized successfully");
            return true;

        } catch (SQLException e) {
            LOG.errorf("Failed to initialize storage: %s", e.getMessage());
            return false;
        }
    }

    /**
     * Store binary data
     */
    public boolean put(String key, byte[] data) {
        String sql = "INSERT OR REPLACE INTO storage (key, value) VALUES (?, ?)";

        try (PreparedStatement pstmt = connection.prepareStatement(sql)) {
            pstmt.setString(1, key);
            pstmt.setBytes(2, data);
            pstmt.executeUpdate();
            return true;

        } catch (SQLException e) {
            LOG.errorf("Failed to put data for key %s: %s", key, e.getMessage());
            return false;
        }
    }

    /**
     * Store string data
     */
    public boolean putString(String key, String value) {
        return put(key, value.getBytes());
    }

    /**
     * Retrieve binary data
     */
    public Optional<byte[]> get(String key) {
        String sql = "SELECT value FROM storage WHERE key = ?";

        try (PreparedStatement pstmt = connection.prepareStatement(sql)) {
            pstmt.setString(1, key);

            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return Optional.of(rs.getBytes("value"));
                }
            }

        } catch (SQLException e) {
            LOG.errorf("Failed to get data for key %s: %s", key, e.getMessage());
        }

        return Optional.empty();
    }

    /**
     * Retrieve string data
     */
    public Optional<String> getString(String key) {
        return get(key).map(String::new);
    }

    /**
     * Check if key exists
     */
    public boolean exists(String key) {
        String sql = "SELECT 1 FROM storage WHERE key = ? LIMIT 1";

        try (PreparedStatement pstmt = connection.prepareStatement(sql)) {
            pstmt.setString(1, key);

            try (ResultSet rs = pstmt.executeQuery()) {
                return rs.next();
            }

        } catch (SQLException e) {
            LOG.errorf("Failed to check existence for key %s: %s", key, e.getMessage());
            return false;
        }
    }

    /**
     * Remove entry
     */
    public boolean remove(String key) {
        String sql = "DELETE FROM storage WHERE key = ?";

        try (PreparedStatement pstmt = connection.prepareStatement(sql)) {
            pstmt.setString(1, key);
            int affected = pstmt.executeUpdate();
            return affected > 0;

        } catch (SQLException e) {
            LOG.errorf("Failed to remove key %s: %s", key, e.getMessage());
            return false;
        }
    }

    /**
     * Get all keys matching pattern
     */
    public List<String> keys(String pattern) {
        List<String> results = new ArrayList<>();

        // Convert simple * pattern to SQL LIKE pattern
        String sqlPattern = pattern.replace("*", "%");
        String sql = "SELECT key FROM storage WHERE key LIKE ?";

        try (PreparedStatement pstmt = connection.prepareStatement(sql)) {
            pstmt.setString(1, sqlPattern);

            try (ResultSet rs = pstmt.executeQuery()) {
                while (rs.next()) {
                    results.add(rs.getString("key"));
                }
            }

        } catch (SQLException e) {
            LOG.errorf("Failed to query keys with pattern %s: %s", pattern, e.getMessage());
        }

        return results;
    }

    /**
     * Begin transaction
     */
    public boolean beginTransaction() {
        try {
            connection.setAutoCommit(false);
            return true;
        } catch (SQLException e) {
            LOG.errorf("Failed to begin transaction: %s", e.getMessage());
            return false;
        }
    }

    /**
     * Commit transaction
     */
    public boolean commit() {
        try {
            connection.commit();
            connection.setAutoCommit(true);
            return true;
        } catch (SQLException e) {
            LOG.errorf("Failed to commit transaction: %s", e.getMessage());
            return false;
        }
    }

    /**
     * Rollback transaction
     */
    public boolean rollback() {
        try {
            connection.rollback();
            connection.setAutoCommit(true);
            return true;
        } catch (SQLException e) {
            LOG.errorf("Failed to rollback transaction: %s", e.getMessage());
            return false;
        }
    }

    /**
     * Close storage
     */
    public void close() {
        if (connection != null) {
            try {
                connection.close();
                LOG.info("Akutik storage closed");
            } catch (SQLException e) {
                LOG.errorf("Error closing storage: %s", e.getMessage());
            }
        }
    }
}
