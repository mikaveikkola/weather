#include "ConnectionPool.h"

#include <stdexcept>
#include <memory>

namespace weather {

// ─── ConnectionPool ────────────────────────────────────────────────────────

ConnectionPool::ConnectionPool(const std::string& connStr, int size)
    : connStr_(connStr)
{
    for (int i = 0; i < size; ++i)
        pool_.push_back(std::make_unique<pqxx::connection>(connStr_));
}

ConnectionPool::~ConnectionPool() = default;

ConnectionPool::DbConn ConnectionPool::acquire() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !pool_.empty(); });
    auto conn = std::move(pool_.back());
    pool_.pop_back();

    // Reconnect if the connection has gone away
    if (!conn->is_open()) {
        conn = std::make_unique<pqxx::connection>(connStr_);
    }
    return DbConn(*this, std::move(conn));
}

void ConnectionPool::release(std::unique_ptr<pqxx::connection> conn) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pool_.push_back(std::move(conn));
    }
    cv_.notify_one();
}

// ─── DbConn ────────────────────────────────────────────────────────────────

ConnectionPool::DbConn::DbConn(ConnectionPool& pool,
                                std::unique_ptr<pqxx::connection> conn)
    : pool_(&pool), conn_(std::move(conn))
{}

ConnectionPool::DbConn::~DbConn() {
    if (pool_ && conn_)
        pool_->release(std::move(conn_));
}

// ─── Singleton ─────────────────────────────────────────────────────────────

static std::unique_ptr<ConnectionPool> g_pool;

void initPool(const std::string& connStr, int size) {
    g_pool = std::make_unique<ConnectionPool>(connStr, size);
}

ConnectionPool& getPool() {
    if (!g_pool) throw std::runtime_error("Connection pool not initialised");
    return *g_pool;
}

} // namespace weather
