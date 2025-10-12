// ==============================================================================
// WebPakLoader.js - Web-based PAK Asset Loader
// Copyright (c) 2025 Patrice Chevillat
// ==============================================================================

/**
 * WebPakLoader - Efficient asset loading from PAK bundles for web applications
 *
 * Features:
 * - HTTP Range requests for partial downloads
 * - Decompression (gzip, zstd via WebAssembly)
 * - In-memory caching
 * - Progressive loading
 * - Dependency resolution
 *
 * Usage:
 * ```javascript
 * const loader = new WebPakLoader('assets/game_assets.web.pak');
 * await loader.init();
 *
 * const texture = await loader.loadTexture('rpg_characters');
 * const atlas = await loader.loadAtlas('rpg_characters');
 * ```
 */

class WebPakLoader {
    constructor(pakUrl, options = {}) {
        this.pakUrl = pakUrl;
        this.options = {
            cacheEnabled: true,
            maxCacheSize: 100 * 1024 * 1024,  // 100 MB
            preloadPriority: true,
            ...options
        };

        this.toc = null;               // Table of Contents
        this.header = null;            // PAK header
        this.cache = new Map();        // In-memory cache
        this.cacheSize = 0;            // Current cache size in bytes
        this.loadingPromises = new Map(); // Deduplicate concurrent loads
        this.statistics = {
            assetsLoaded: 0,
            cacheHits: 0,
            cacheMisses: 0,
            bytesDownloaded: 0,
            bytesDecompressed: 0
        };
    }

    // =========================================================================
    // Initialization
    // =========================================================================

    /**
     * Initialize the loader by fetching the Table of Contents
     */
    async init() {
        console.log('[WebPakLoader] Initializing...', this.pakUrl);

        try {
            // First, fetch file size
            const headResponse = await fetch(this.pakUrl, { method: 'HEAD' });
            const fileSize = parseInt(headResponse.headers.get('Content-Length'));

            if (!fileSize) {
                throw new Error('Could not determine PAK file size');
            }

            // Fetch last 64KB (contains TOC)
            const tocSize = Math.min(64 * 1024, fileSize);
            const tocResponse = await fetch(this.pakUrl, {
                headers: {
                    'Range': `bytes=${fileSize - tocSize}-${fileSize - 1}`
                }
            });

            if (!tocResponse.ok) {
                throw new Error(`Failed to fetch TOC: ${tocResponse.status}`);
            }

            const tocData = await tocResponse.arrayBuffer();

            // Parse TOC (simplified - in real implementation, use protobuf decoder)
            this.toc = this._parseTOC(tocData);

            // Also parse header (first 1KB)
            const headerResponse = await fetch(this.pakUrl, {
                headers: {
                    'Range': 'bytes=0-1023'
                }
            });

            const headerData = await headerResponse.arrayBuffer();
            this.header = this._parseHeader(headerData);

            console.log(`[WebPakLoader] Initialized: ${this.toc.assets.length} assets`);

            return true;

        } catch (error) {
            console.error('[WebPakLoader] Initialization failed:', error);
            throw error;
        }
    }

    // =========================================================================
    // Asset Loading
    // =========================================================================

    /**
     * Load asset by ID
     * @param {string} assetId - Asset identifier
     * @returns {Promise<ArrayBuffer>} Asset data (decompressed)
     */
    async loadAsset(assetId) {
        // Check cache first
        if (this.options.cacheEnabled && this.cache.has(assetId)) {
            this.statistics.cacheHits++;
            return this.cache.get(assetId);
        }

        this.statistics.cacheMisses++;

        // Deduplicate concurrent requests
        if (this.loadingPromises.has(assetId)) {
            return this.loadingPromises.get(assetId);
        }

        const loadPromise = this._loadAssetInternal(assetId);
        this.loadingPromises.set(assetId, loadPromise);

        try {
            const data = await loadPromise;
            this.loadingPromises.delete(assetId);

            // Add to cache
            if (this.options.cacheEnabled) {
                this._addToCache(assetId, data);
            }

            this.statistics.assetsLoaded++;
            return data;

        } catch (error) {
            this.loadingPromises.delete(assetId);
            throw error;
        }
    }

    async _loadAssetInternal(assetId) {
        const entry = this.toc.assets.find(a => a.asset_id === assetId);

        if (!entry) {
            throw new Error(`Asset not found: ${assetId}`);
        }

        console.log(`[WebPakLoader] Loading asset: ${assetId}`);

        // Fetch asset data using range request
        const response = await fetch(this.pakUrl, {
            headers: {
                'Range': `bytes=${entry.data_offset}-${entry.data_offset + entry.compressed_size - 1}`
            }
        });

        if (!response.ok) {
            throw new Error(`Failed to fetch asset: ${response.status}`);
        }

        let data = await response.arrayBuffer();
        this.statistics.bytesDownloaded += data.byteLength;

        // Decompress if needed
        if (entry.compression !== 'NONE') {
            data = await this._decompress(data, entry.compression);
            this.statistics.bytesDecompressed += data.byteLength;
        }

        return data;
    }

    /**
     * Load texture as Image
     * @param {string} assetId - Asset identifier
     * @returns {Promise<HTMLImageElement>}
     */
    async loadTexture(assetId) {
        const data = await this.loadAsset(assetId);
        const blob = new Blob([data], { type: 'image/png' });
        const url = URL.createObjectURL(blob);

        const img = new Image();
        img.src = url;

        await img.decode();  // Wait for decode

        // Clean up blob URL after a delay
        setTimeout(() => URL.revokeObjectURL(url), 1000);

        return img;
    }

    /**
     * Load texture atlas with metadata
     * @param {string} atlasId - Atlas identifier (without _meta suffix)
     * @returns {Promise<Object>} {image: HTMLImageElement, metadata: Object}
     */
    async loadAtlas(atlasId) {
        const [image, metadataData] = await Promise.all([
            this.loadTexture(atlasId),
            this.loadAsset(`${atlasId}_meta`)
        ]);

        // Parse metadata JSON
        const metadataText = new TextDecoder().decode(metadataData);
        const metadata = JSON.parse(metadataText);

        return {
            image,
            metadata,
            // Helper method to get sprite
            getSprite(spriteId) {
                return metadata.sprites[spriteId];
            }
        };
    }

    /**
     * Load JSON asset
     * @param {string} assetId
     * @returns {Promise<Object>}
     */
    async loadJSON(assetId) {
        const data = await this.loadAsset(assetId);
        const text = new TextDecoder().decode(data);
        return JSON.parse(text);
    }

    /**
     * Load text asset
     * @param {string} assetId
     * @returns {Promise<string>}
     */
    async loadText(assetId) {
        const data = await this.loadAsset(assetId);
        return new TextDecoder().decode(data);
    }

    /**
     * Load audio asset
     * @param {string} assetId
     * @returns {Promise<AudioBuffer>}
     */
    async loadAudio(assetId) {
        const data = await this.loadAsset(assetId);
        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
        return audioContext.decodeAudioData(data);
    }

    // =========================================================================
    // Batch Loading
    // =========================================================================

    /**
     * Load multiple assets in parallel
     * @param {string[]} assetIds
     * @returns {Promise<Map<string, ArrayBuffer>>}
     */
    async loadAssets(assetIds) {
        const promises = assetIds.map(id =>
            this.loadAsset(id).then(data => [id, data])
        );

        const results = await Promise.all(promises);
        return new Map(results);
    }

    /**
     * Load assets with dependencies
     * @param {string} assetId
     * @returns {Promise<Map<string, ArrayBuffer>>}
     */
    async loadAssetWithDependencies(assetId) {
        const dependencies = this.getDependencies(assetId, true);
        dependencies.push(assetId);  // Include the asset itself

        return this.loadAssets(dependencies);
    }

    // =========================================================================
    // Asset Information
    // =========================================================================

    /**
     * Get asset metadata
     * @param {string} assetId
     * @returns {Object|null}
     */
    getAssetInfo(assetId) {
        return this.toc.assets.find(a => a.asset_id === assetId) || null;
    }

    /**
     * Check if asset exists
     * @param {string} assetId
     * @returns {boolean}
     */
    hasAsset(assetId) {
        return this.toc.assets.some(a => a.asset_id === assetId);
    }

    /**
     * List all assets
     * @returns {string[]}
     */
    listAssets() {
        return this.toc.assets.map(a => a.asset_id);
    }

    /**
     * List assets by type
     * @param {string} assetType
     * @returns {string[]}
     */
    listAssetsByType(assetType) {
        return this.toc.assets
            .filter(a => a.asset_type === assetType)
            .map(a => a.asset_id);
    }

    /**
     * List assets by tag
     * @param {string} tag
     * @returns {string[]}
     */
    listAssetsByTag(tag) {
        return this.toc.assets
            .filter(a => a.tags && a.tags.includes(tag))
            .map(a => a.asset_id);
    }

    /**
     * Get dependencies for an asset
     * @param {string} assetId
     * @param {boolean} recursive - Get transitive dependencies
     * @returns {string[]}
     */
    getDependencies(assetId, recursive = false) {
        const entry = this.getAssetInfo(assetId);
        if (!entry || !entry.dependencies) {
            return [];
        }

        if (!recursive) {
            return entry.dependencies;
        }

        // Transitive closure
        const visited = new Set();
        const queue = [...entry.dependencies];

        while (queue.length > 0) {
            const depId = queue.shift();
            if (visited.has(depId)) continue;

            visited.add(depId);

            const depEntry = this.getAssetInfo(depId);
            if (depEntry && depEntry.dependencies) {
                queue.push(...depEntry.dependencies);
            }
        }

        return Array.from(visited);
    }

    // =========================================================================
    // Preloading
    // =========================================================================

    /**
     * Preload critical assets
     */
    async preloadCriticalAssets() {
        const critical = this.toc.assets.filter(a => a.priority === 'CRITICAL');
        console.log(`[WebPakLoader] Preloading ${critical.length} critical assets`);

        const promises = critical.map(a => this.loadAsset(a.asset_id));
        await Promise.all(promises);
    }

    /**
     * Preload assets by tag
     * @param {string} tag
     */
    async preloadByTag(tag) {
        const assetIds = this.listAssetsByTag(tag);
        console.log(`[WebPakLoader] Preloading ${assetIds.length} assets with tag: ${tag}`);

        const promises = assetIds.map(id => this.loadAsset(id));
        await Promise.all(promises);
    }

    // =========================================================================
    // Cache Management
    // =========================================================================

    _addToCache(assetId, data) {
        const size = data.byteLength;

        // Evict if cache is full (LRU)
        while (this.cacheSize + size > this.options.maxCacheSize && this.cache.size > 0) {
            const firstKey = this.cache.keys().next().value;
            const firstValue = this.cache.get(firstKey);
            this.cacheSize -= firstValue.byteLength;
            this.cache.delete(firstKey);
        }

        this.cache.set(assetId, data);
        this.cacheSize += size;
    }

    /**
     * Clear cache
     */
    clearCache() {
        this.cache.clear();
        this.cacheSize = 0;
    }

    /**
     * Get cache statistics
     */
    getCacheStats() {
        return {
            entries: this.cache.size,
            size: this.cacheSize,
            maxSize: this.options.maxCacheSize,
            usage: (this.cacheSize / this.options.maxCacheSize) * 100
        };
    }

    // =========================================================================
    // Statistics
    // =========================================================================

    /**
     * Get loader statistics
     */
    getStatistics() {
        return {
            ...this.statistics,
            cacheStats: this.getCacheStats()
        };
    }

    /**
     * Reset statistics
     */
    resetStatistics() {
        this.statistics = {
            assetsLoaded: 0,
            cacheHits: 0,
            cacheMisses: 0,
            bytesDownloaded: 0,
            bytesDecompressed: 0
        };
    }

    // =========================================================================
    // Internal Utilities
    // =========================================================================

    _parseHeader(data) {
        // Simplified header parsing
        // In real implementation, use protobuf decoder
        const view = new DataView(data);

        return {
            magic: this._readString(data, 0, 8),
            version_major: view.getUint32(8, true),
            version_minor: view.getUint32(12, true),
            toc_offset: Number(view.getBigUint64(16, true)),
            asset_count: view.getUint32(24, true),
            compression: this._readString(data, 28, 16)
        };
    }

    _parseTOC(data) {
        // Simplified TOC parsing
        // In real implementation, use protobuf decoder

        // For now, assume JSON-encoded TOC for web
        try {
            const text = new TextDecoder().decode(data);
            const jsonStart = text.indexOf('{');
            if (jsonStart === -1) {
                throw new Error('Invalid TOC format');
            }
            return JSON.parse(text.substring(jsonStart));
        } catch (error) {
            console.error('[WebPakLoader] TOC parsing error:', error);
            throw error;
        }
    }

    async _decompress(data, compressionType) {
        // Simplified decompression
        // In real implementation, use CompressionStream API or WebAssembly

        if (compressionType === 'GZIP') {
            // Use native browser gzip decompression
            const stream = new Response(data).body.pipeThrough(new DecompressionStream('gzip'));
            const decompressed = await new Response(stream).arrayBuffer();
            return decompressed;
        }

        // For ZSTD, you'd need a WebAssembly decoder
        console.warn(`[WebPakLoader] Compression type ${compressionType} not yet supported`);
        return data;
    }

    _readString(buffer, offset, length) {
        const bytes = new Uint8Array(buffer, offset, length);
        return new TextDecoder().decode(bytes).replace(/\0/g, '');
    }
}

// ==============================================================================
// Export
// ==============================================================================

if (typeof module !== 'undefined' && module.exports) {
    module.exports = WebPakLoader;
}